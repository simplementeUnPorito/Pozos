/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

#include <stdio.h>

#include "config_general.h"
#include "er.h"
#include "emision_adq.h"
#include "clock_config.h"
#include "envolvente.h"
#include "derivada.h"
#include "crc.h"
#include "eeprom_v.h"
#include "filtfilt.h"
#include "correlacion.h"
#include "pico_max.h"
#include "normalizar.h"

#define TAM_BUF_TX  100
char bufTX[TAM_BUF_TX];

// implementación del timeout, se usa un contador que cuenta 
// las veces en el ciclo de espera, cuando este valor supera
// al de T_ESPERA se sale por timeout.
// Debe estar definido FRECUENCIA_CLOCK_CONFIG con la frecuencia
// de operación de microprocesador.
#define T_ESPERA	((FRECUENCIA_CLOCK_CONFIG / 60) * 2)	// aprox. dos segundo

// Tamaño de los buffers de adquisición de datos. YA ESTÁ DEFINIDO EN ENVOLVENTE.H
//#define TAM_BUF 2048

// Vector donde se guarda la envolvente
static float32_t envolvente[N_PUNTOS_PROCESADOS];

// Vector donde se guardan la envolvente filtrada
//float32_t envolventeFiltrada[N_PUNTOS_PROCESADOS];

// Vector donde se guarda la derivada
static float32_t derivada[N_PUNTOS_PROCESADOS];

// Vector donde se guarda la derivada normalizada
//float32_t derNorm[N_PUNTOS_PROCESADOS];

// Variables donde se guardan los datos leídos y a ser escritos en la eeprom
static struct modeloEnvolvente envRec, envAct;

// Vector donde se guarda el resultado de la correlación
static float32_t resCor[N_PUNTOS_PROCESADOS];

// bandera que indica si hay datos válidos en la EEPROM
static uint8_t banderaErrorEEPROM = 0;

// Buffer temporal para los comando M, O y R

static uint8_t bufTemp[TAM_BUF_TEMP];  // buffer temporal para almacenar los datos recibidos

/*
Esperamos por un tiempo dos caracteres por el puerto serial. Estos son los datos
que indican cuantas interrupciones del DMA del ADC tenemos que esperar.
El primer byte puede ser cualquiera entre 0 y 255. El segundo carácter solo puede
ser '0' o '5'.
Parámetro:
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
          suponemos la función GetChar de la biblioteca del PSoC que retorna 0 si no
          hay datos recibidos.
Retorno:
    Puntero al vector con los dos bytes.
    NULL si ocurrió un timeout o el dato del segundo byte no es correcto.
    
*/
uint8_t *rec2bytes (uint8_t (*gc)(void)) {
    static uint8_t datos[3];
    uint8_t v;
    uint32_t cont = 0;
    
    for (cont = 0; !(v = gc()) && cont < T_ESPERA; cont++);   // esperamos un byte por un tiempo
    if (cont >= T_ESPERA) return NULL;	// timeout, termina inmediatamente
    datos[0] = v;
//    if (v >= '1' && v <= '9')
//        datos[0] = v;
//    else
//        datos[0] = '0'; // indica error

    for (cont = 0; !(v = gc()) && cont < T_ESPERA; cont++);   // esperamos un byte por un tiempo
    if (cont >= T_ESPERA) return NULL;	// timeout, termina inmediatamente
    if ( v == '0' || v == '5')
        datos[1] = v;
    else
        return NULL; // indica error
    datos[2] = '\0';
    
    return datos;
}

/*
Comando que digitaliza la señal y envía los datos al que los pide sin procesarlos.
Para completar el comando se necesita saber cuantas mediciones realizar.
Se esperan dos caracteres numéricos: 10, 15, 20, 25, 30, 35, ...
El primer número representa la cantidad de adquisiciones que se realizarán. Se adquieren
N_PUNTOS_PROCESADOS * 2 muestras en cada adquisición realizada (interrupción del DMA del ADC). 
De estas muestras para esta aplicación solamente se consideran N_PUNTOS_PROCESADOS muestras. 
Si el segundo número es 0 entonces se envían las N_PUNTOS_PROCESADOS muestras del primer buffer
más una muestra del segundo. Si el segundo número recibido es 5 se envían las últimas 
N_PUNTOS_PROCESADOS /2 muestras del primer buffer y las primeras N_PUNTOS_PROCESADOS /2 muestras 
del segundo buffer.
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
Retorno:
    Ninguno
*/
void comandoL (void (*pc)(uint8_t), uint8_t (*gc)(void)) {
    uint16_t *buf;
    uint8_t *com;
    uint32_t cont;

    com = rec2bytes(gc);
    if (!com) {
        pc('e');
        return;
    }
    
    if (com[0] == 0 || (com[1] != '0' && com[1] != '5')){
        pc('e');
        return;
    }
    
    iniciaEmisionYAdquisicion(com[0]);
    
    if (esperaFinAdquisicion()) {
        pc('e');
        return;
    }
    
    // Construimos el buffer y enviamos los datos a la PC
    // Los buffers de adquisición se definen en el archivo emision_adq.c
    // Aquí se recibe el puntero nada más.
    if (com[1] == '5')
        // la mitad del buffer anterior y el actual
        buf = datosAdquiridos(1);
    else 
        // el buffer actual
        buf = datosAdquiridos(0);
        
    // enviamos a la PC
    pc('r');   // aviso que estoy listo
    for (cont = 0; !gc() && cont < T_ESPERA; cont++);  // esperamos respuesta
    if (cont == T_ESPERA) {
        pc('e');
        return;
    }
    // enviamos si recibimos respuesta
    envBytesChecksum ((uint8_t *)buf, N_PUNTOS_PROCESADOS * sizeof(uint16_t), pc);
}

/*
Comando que digitaliza la señal, obtiene la envolvente de Hilbert y envía los datos.
La adquisición se realiza según el se explica en el comandoL.
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
Retorno:
    Ninguno
*/
void comandoE (void (*pc)(uint8_t), uint8_t (*gc)(void)) {
    uint16_t *buf;
    uint8_t *com;
    uint32_t cont;

    com = rec2bytes(gc);
    if (!com) {
        pc('e');
        return;
    }
    
    if (com[0] == 0 || (com[1] != '0' && com[1] != '5')){
        pc('e');
        return;
    }
    
    iniciaEmisionYAdquisicion(com[0]);

    if (esperaFinAdquisicion()) {
        pc('e');
        return;
    }
    
    // Construimos el buffer y enviamos los datos a la PC
    if (com[1] == '5')
        // la mitad del buffer anterior y el actual
        buf = datosAdquiridos(1);
    else 
        // el buffer actual
        buf = datosAdquiridos(0);
    
    // Calculamos la envolvente
    calcularEnvolvente(buf, envolvente);

    // filtramos la envolvente
    if (filtfilt_fir_cmsis(envolvente, N_PUNTOS_PROCESADOS, envolvente) < 0) {
        pc('e');
        return;
    }

    // enviamos a la PC
    pc('r');   // aviso que estoy listo
    for (cont = 0; !gc() && cont < T_ESPERA; cont++);  // esperamos respuesta
    if (cont == T_ESPERA) 
        return;
    
    // enviamos si recibimos respuesta
    envBytesChecksum ((uint8_t *)envolvente, N_PUNTOS_PROCESADOS * sizeof(float32_t), pc);
}

/*
Comando que digitaliza la señal, obtiene la envolvente de Hilbert, calcula la derivada
y envía los datos.
La adquisición se realiza según el se explica en el comandoL.
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
Retorno:
    Ninguno
*/
void comandoD (void (*pc)(uint8_t), uint8_t (*gc)(void)) {
    uint16_t *buf;
    uint8_t *com;
    uint32_t cont;

    com = rec2bytes(gc);
    if (!com) {
        pc('e');
        return;
    }
    
    if (com[0] == 0 || (com[1] != '0' && com[1] != '5')){
        pc('e');
        return;
    }
    
    iniciaEmisionYAdquisicion(com[0]);
    
    if (esperaFinAdquisicion()) {
        pc('e');
        return;
    }
    
    // Construimos el buffer y enviamos los datos a la PC
    if (com[1] == '5')
        // la mitad del buffer anterior y el actual
        buf = datosAdquiridos(1);
    else 
        // el buffer actual
        buf = datosAdquiridos(0);
    
    // Calculamos la envolvente
    calcularEnvolvente(buf, envolvente);
    
    // filtramos la envolvente
    if (filtfilt_fir_cmsis(envolvente, N_PUNTOS_PROCESADOS, envolvente) < 0) {
        pc('e');
        return;
    }
    
    // Calculamos la derivada
    if (derivada_rfft_cmsis(envolvente, derivada, N_PUNTOS_PROCESADOS, FRECUENCIA_MUESTREO, 1, 0) < 0){
        pc('e');   // aviso de error
        return;
    }
    
    // Normalizamos la salida
    normalize_peak_f32(derivada, envolvente, N_PUNTOS_PROCESADOS);
    
    // enviamos a la PC
    pc('r');   // aviso que estoy listo
    for (cont = 0; !gc() && cont < T_ESPERA; cont++);  // esperamos respuesta
    if (cont == T_ESPERA) 
        return;
    
    // enviamos si recibimos respuesta
    envBytesChecksum ((uint8_t *)envolvente, N_PUNTOS_PROCESADOS * sizeof(float32_t), pc);
}

/*
Comando que digitaliza la señal, obtiene la envolvente de Hilbert, calcula la derivada,
hace la correlación con la plantilla en la EEPROM y envía los datos.
La adquisición se realiza según el se explica en el comandoL.
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
Retorno:
    Ninguno
*/
void comandoX (void (*pc)(uint8_t), uint8_t (*gc)(void)) {
    uint16_t *buf;
    uint8_t *com;
    uint32_t cont;

    com = rec2bytes(gc);
    if (!com) {
        pc('e');
        return;
    }
    
    if (com[0] == 0 || (com[1] != '0' && com[1] != '5')){
        pc('e');
        return;
    }
    
    iniciaEmisionYAdquisicion(com[0]);
    if (esperaFinAdquisicion()) {
        pc('e');
        return;
    }
    
    // Construimos el buffer y enviamos los datos a la PC
    if (com[1] == '5')
        // la mitad del buffer anterior y el actual
        buf = datosAdquiridos(1);
    else 
        // el buffer actual
        buf = datosAdquiridos(0);
    
    // Calculamos la envolvente
    calcularEnvolvente(buf, envolvente);
    
    // filtramos la envolvente
    if (filtfilt_fir_cmsis(envolvente, N_PUNTOS_PROCESADOS, envolvente) < 0) {
        pc('e');
        return;
    }
    
    // Calculamos la derivada
    if (derivada_rfft_cmsis(envolvente, derivada, N_PUNTOS_PROCESADOS, FRECUENCIA_MUESTREO, 1, 0) < 0){
        pc('e');   // aviso de error
        return;
    }
    
    // Normalizamos la salida
    normalize_peak_f32(derivada, envolvente, N_PUNTOS_PROCESADOS);

    // hacemos la correlación
    calcularCorrelacion (envolvente, N_PUNTOS_PROCESADOS, envAct.m, envAct.utilizado, resCor);

    // enviamos a la PC
    pc('r');   // aviso que estoy listo
    for (cont = 0; !gc() && cont < T_ESPERA; cont++);  // esperamos respuesta
    if (cont == T_ESPERA) 
        return;
    
    // enviamos si recibimos respuesta
    envBytesChecksum ((uint8_t *)resCor, (N_PUNTOS_PROCESADOS - envAct.utilizado + 1) * sizeof(float32_t), pc);
}

/*
Función que redondea x a 3 decimales (multiplica por 1000 y aplica redondeo (función round)
Separa la parte entera y la parte fraccionaria de 3 dígitos.
Maneja el signo correctamente; si el valor redondeado es 0, imprime "0.000" (sin signo negativo).
Solo utiliza especificadores de formato %d (se usa "%03d" para rellenar ceros en la fracción).
Parámetro:
    x -> número a imprimir
Valor de retorno:
    Ninguno.
 */
void print_with_3decimals(float32_t x) {
    long long scaled;
    int integer_part;
    int frac_part;
    int negative = 0;

    // Redondeo: mitad hacia arriba (away from zero)
    scaled = round(x * 1000);
//    if (x >= 0.0) {
//        scaled = (long long)(x * 1000.0 + 0.5);
//    } else {
//        scaled = (long long)(x * 1000.0 - 0.5);
//    }

    // Si el valor redondeado es negativo, guardamos el signo y trabajamos con positivo
    if (scaled < 0) {
        negative = 1;
        scaled = -scaled;
    }

    integer_part = (int)(scaled / 1000);
    frac_part = (int)(scaled % 1000); // tendrá 0..999

    if (negative && (integer_part != 0 || frac_part != 0))
        printf("-%d.%03d", integer_part, frac_part);
    else
        printf("%d.%03d", integer_part, frac_part);
}

/*
Comando que digitaliza la señal, obtiene la envolvente de Hilbert, calcula la derivada
y hace la correlación con el modelo para obtener la posición del calibrador o del pelo del agua.
La adquisición se realiza según el se explica en el comandoL.
Los datos para buscar la señal se obtienen de la EEPROM.
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
    nbuf -> cuantos buffers hay que esperar
    mitad -> si hay que considerar la mitad de un buffer
Retorno:
    Ninguno
*/
void comandoCalc (void (*pc)(uint8_t), uint8_t (*gc)(void), uint8_t nBuf, uint8_t mitad) {
    uint16_t *buf;
    uint32_t cont;
    float32_t max, *posMax, tof;

    if (nBuf == 0 || (mitad != '0' && mitad != '5')){
        pc('e');
        return;
    }
    
    iniciaEmisionYAdquisicion(nBuf);
    if (esperaFinAdquisicion()) {
        pc('e');
        return;
    }
    
    // Construimos el buffer y enviamos los datos a la PC
    if (mitad == '5')
        // la mitad del buffer anterior y el actual
        buf = datosAdquiridos(1);
    else 
        // el buffer actual
        buf = datosAdquiridos(0);
    
    // Calculamos la envolvente
    calcularEnvolvente(buf, envolvente);
    
    // filtramos la envolvente
    if (filtfilt_fir_cmsis(envolvente, N_PUNTOS_PROCESADOS, envolvente) < 0) {
        pc('e');
        return;
    }
    
    // Calculamos la derivada
    if (derivada_rfft_cmsis(envolvente, derivada, N_PUNTOS_PROCESADOS, FRECUENCIA_MUESTREO, 0, 0) < 0){
        pc('e');   // aviso de error
        return;
    }
    
    // Normalizamos la salida
    normalize_peak_f32(derivada, envolvente, N_PUNTOS_PROCESADOS);

    // hacemos la correlación
    calcularCorrelacion (envolvente, N_PUNTOS_PROCESADOS, envAct.m, envAct.utilizado, resCor);
    
    // buscamos el máximo
    if (nBuf == 1) {
        // primer buf, hay que saltar la inducción de la emisión
        // TODO: ahora el salto es un valor constante, debería ser configurable
        // TODO: ahora el valor del umbral de detección es 0, debería ser variable
        maximo(resCor, 300, (sizeof(resCor)/sizeof(float32_t)) -1, 0, &max, &posMax);
        if (posMax == NULL) {
            // Error
            pc('e');   // aviso de error
            return;
        }
    } else {
        maximo(resCor, 0, (sizeof(resCor)/sizeof(float32_t)) -1, 0, &max, &posMax);
        if (posMax == NULL) {
            // Error
            pc('e');   // aviso de error
            return;
        }
    }
    
    // calculamos el tiempo de vuelo en segundo
    tof = ((((nBuf -1) * N_PUNTOS_PROCESADOS) + (posMax - resCor)) * 1/FRECUENCIA_MUESTREO) / 2;
    tof = tof * VELOCIDAD_DEL_SONIDO;
    
    // enviamos a la PC
    pc('r');   // aviso que estoy listo
    for (cont = 0; !gc() && cont < T_ESPERA; cont++);  // esperamos respuesta
    if (cont == T_ESPERA) 
        return;
    
    // enviamos el valor si recibimos respuesta
    envBytesChecksum ((uint8_t *)&tof, sizeof(float32_t), pc);
}

/*
Comando que recibe de la PC la información sobre la ubicación del pelo del agua
y del calibrador, así como la cantidad de muestras a saltar en el primer buffer 
para evitar la inducción de la emisión.
Se esperan los datos relativos a la ubicación del pelo de agua, el calibrador y las muestras a saltar
Solo ser reciben 4 bytes utilizando el protocolo de comunicación ya establecido. 
El primer byte es la ubicación calibrador. El siguiente byte contiene un valor que puede 
ser solo 0 o 5 para indicar si se considera medio buffer o no. El siguiente byte es la 
ubicación del pelo del agua y finalmente el último byte contiene solo 0 o 5 para indicar 
que se considera medio buffer o no. 
Parámetro:
    hayDato -> puntero a la función que verifica si hay datos recibidos por el puerto serial
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    pd -> puntero a la estructura de datos donde se colocarán los datos recibidos.
Retorno:
     0 -> si se recibieron los datos correctamente.
    -1 si hay algún error.
*/
int comandoR (uint8_t (*hayDato)(), uint8_t (*gc)(void), void (*pc)(uint8_t), struct modeloEnvolvente *pd) {
    uint8_t ret;
    
    pc('r');   // aviso que estoy listo
    // recibimos todos los bytes de la estructura menos 2, que son los que indican 
    // el tamaño del modelo para la correlación, que no se modifica
    if ((ret = recBytesChecksum(bufTemp, TAM_BUF_TEMP -2, hayDato, gc)) != 0) {
        pc('e');
        return -1;
    }

    pd->utilizado = bufTemp[1] << 8 | bufTemp[0];
    pd->ubicacionCalibrador = bufTemp[2];
    pd->ubicacionCalibradorMedio = bufTemp[3];
    pd->nivelMinCalibrador = (bufTemp[7] << 24u) | (bufTemp[6] << 16u) | (bufTemp[5] << 8u) | bufTemp[4];
    pd->posicionCalibrador = (bufTemp[11] << 24u) | (bufTemp[10] << 16u) | (bufTemp[9] << 8u) | bufTemp[8];
    pd->ubicacionAgua = bufTemp[12];
    pd->ubicacionAguaMedio = bufTemp[13];
    pd->nivelMinAgua = (bufTemp[17] << 24u) | (bufTemp[16] << 16u) | (bufTemp[15] << 8u) | bufTemp[14];
    pd->saltar = bufTemp[19] << 8 | bufTemp[18];
    
    // Revisar que los valores recibidos tengan sentido
    if ((pd->ubicacionAgua == 0) ||
        (pd->ubicacionAguaMedio != '5'        && pd->ubicacionAguaMedio != '0') || 
        (pd->ubicacionCalibrador == 0) ||
        (pd->ubicacionCalibradorMedio != '0'  && pd->ubicacionCalibradorMedio != '5')) {
        pc('e');
        return -1;
    }

    return 0;
}

/*
Comando que recibe de la PC la información sobre la ubicación del pelo del agua
y del calibrador, así como la señal que se utilizará para la detección.
El envío se realiza en dos partes:
1. Los datos relativos a la ubicación del pelo de agua y el calibrador
2. La señal modelo para la detección.
La primera parte implica la recepción de 6 bytes utilizando el protocolo de comunicación
ya establecido. Los primeros dos bytes corresponden a la cantidad de puntos de la señal 
modelo para la correlación. El siguiente byte es la ubicación calibrador. El siguiente byte
contiene un valor que puede ser solo 0 o 5 para indicar si se considera medio buffer o no.
El siguiente byte es la ubicación del pelo del agua y finalmente el último byte contiene
solo 0 o 5 para indicar que se considera medio buffer o no. 
Parámetro:
    hayDato -> puntero a la función que verifica si hay datos recibidos por el puerto serial
    gc -> puntero a la función que recibe datos por el puerto serial correspondiente
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    pd -> puntero a la estructura de datos donde se colocarán los datos recibidos.
Retorno:
     0 -> si se recibieron los datos correctamente.
    -1 si hay algún error.
*/
int comandoM (uint8_t (*hayDato)(), uint8_t (*gc)(void), void (*pc)(uint8_t), struct modeloEnvolvente *pd) {
    uint8_t ret;
    
//    pc('r');   // aviso que estoy listo
//    if ((ret = recBytesChecksum(bufTemp, TAM_BUF_TEMP, hayDato, gc)) != 0) {
//        pc('e');
//        return -1;
//    }
//    pd->utilizado = bufTemp[1] << 8 | bufTemp[0];
//    pd->ubicacionCalibrador = bufTemp[2];
//    pd->ubicacionCalibradorMedio = bufTemp[3];
//    pd->nivelMinCalibrador = (bufTemp[7] << 24u) | (bufTemp[6] << 16u) | (bufTemp[5] << 8u) | bufTemp[4];
//    pd->posicionCalibrador = (bufTemp[11] << 24u) | (bufTemp[10] << 16u) | (bufTemp[9] << 8u) | bufTemp[8];
//    pd->ubicacionAgua = bufTemp[12];
//    pd->ubicacionAguaMedio = bufTemp[13];
//    pd->nivelMinAgua = (bufTemp[17] << 24u) | (bufTemp[16] << 16u) | (bufTemp[15] << 8u) | bufTemp[14];
//    pd->saltar = bufTemp[19] << 8 | bufTemp[18];
//    
//    // Revisar que los valores recibidos tengan sentido
//    if (pd->utilizado > TAM_MODELO_ENVOLVENTE || 
//        (pd->ubicacionAgua == 0) ||
//        (pd->ubicacionAguaMedio != '5'        && pd->ubicacionAguaMedio != '0') || 
//        (pd->ubicacionCalibrador == 0) ||
//        (pd->ubicacionCalibradorMedio != '0'  && pd->ubicacionCalibradorMedio != '5')) {
//        pc('e');
//        return -1;
//    }
    if (comandoR(hayDato, gc, pc, pd) == -1)
        return -1;
    
    pc('r');   // aviso que estoy listo
    if ((ret = recBytesChecksum((uint8_t *)pd->m, pd->utilizado * sizeof(float32_t), hayDato, gc)) != 0) {
        pc('e');
        return -1;
    }
    return 0;
}

/*
Comando que envía a la PC la información sobre la ubicación del pelo del agua
y del calibrador, así como la señal que se utilizará para la detección.
El envío se realiza en dos partes:
1. Los datos relativos a la ubicación del pelo de agua y el calibrador
2. La señal modelo para la detección.
La primera parte implica la recepción de 6 bytes utilizando el protocolo de comunicación
ya establecido. Los primeros dos bytes corresponden a la cantidad de puntos de la señal 
modelo para la correlación. El siguiente byte es la ubicación calibrador. El siguiente byte
contiene un valor que puede ser solo 0 o 5 para indicar si se considera medio buffer o no.
El siguiente byte es la ubicación del pelo del agua y finalmente el último byte contiene
solo 0 o 5 para indicar que se considera medio buffer o no. 
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
    pd -> puntero a la estructura de datos donde se colocarán los datos recibidos.
Retorno:
    Ninguno.
*/
int comandoO (void (*pc)(uint8_t), struct modeloEnvolvente *pd) {
    // Preparamos el buffer temporal
    bufTemp[0] = (pd->utilizado) & 0xFFu;
    bufTemp[1] = (pd->utilizado >> 8) & 0xFFu;
    bufTemp[2] = pd->ubicacionCalibrador;
    bufTemp[3] = pd->ubicacionCalibradorMedio;
    bufTemp[4] = ((uint32_t)pd->nivelMinCalibrador) & 0xFFu;
    bufTemp[5] = ((uint32_t)pd->nivelMinCalibrador >> 8u) & 0xFFu;
    bufTemp[6] = ((uint32_t)pd->nivelMinCalibrador >> 16u) & 0xFFu;
    bufTemp[7] = ((uint32_t)pd->nivelMinCalibrador >> 24u) & 0xFFu;
    bufTemp[8] = ((uint32_t)pd->posicionCalibrador) & 0xFFu;
    bufTemp[9] = ((uint32_t)pd->posicionCalibrador >> 8u) & 0xFFu;
    bufTemp[10] = ((uint32_t)pd->posicionCalibrador >> 16u) & 0xFFu;
    bufTemp[11] = ((uint32_t)pd->posicionCalibrador >> 24u) & 0xFFu;
    bufTemp[12] = pd->ubicacionAgua;
    bufTemp[13] = pd->ubicacionAguaMedio;
    bufTemp[14] = ((uint32_t)pd->nivelMinAgua) & 0xFFu;
    bufTemp[15] = ((uint32_t)pd->nivelMinAgua >> 8u) & 0xFFu;
    bufTemp[16] = ((uint32_t)pd->nivelMinAgua >> 16u) & 0xFFu;
    bufTemp[17] = ((uint32_t)pd->nivelMinAgua >> 24u) & 0xFFu;
    bufTemp[18] = pd->saltar & 0xFFu;
    bufTemp[19] = (pd->saltar >> 8) & 0xFFu;
    
    pc('r');   // aviso que estoy listo
    
    envBytesChecksum(bufTemp, TAM_BUF_TEMP, pc);
    
    pc('r');   // aviso que estoy listo

    envBytesChecksum((uint8_t *)pd->m, pd->utilizado * sizeof(float32_t), pc);
    
    return 0;
}

/*
Tarea que se realiza cuando se recibe el comando 'a'.
Comando que permite verificar que este programa está atento a recibir comandos.
responde con la letra 'a' y el bit de la bandera de error de la eeprom ('0' no hay error, '1' hay error)
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
Retorno:
    Ninguno
*/
void comandoA (void (*pc)(uint8_t)) {
    pc('a');
    pc(banderaErrorEEPROM + '0');
}

/*
Tarea que se realiza cuando se recibe el comando 'p'.
Comando que permite verificar el bit de la bandera de error de la eeprom ('0' no hay error, '1' hay error).
Envía por el puerto serial el valor de bandera en ASCII.
Parámetro:
    pc -> puntero a la función que envía datos por el puerto serial correspondiente
Retorno:
    Ninguno
*/
void comandoP (void (*pc)(uint8_t)) {
    pc(banderaErrorEEPROM + '0');
}

/*
Función que revisa si la EEPROM contiene datos válidos. Si es así los considera como 
el modelo para la correlación. Si no son válidos, se activa una bandera.
Parámetros:
    mod -> dirección de la memoria donde se guarda el modelo.
Retorno:
    0 -> si no ocurrió ningún error
    1 -> si no hay datos en la EEPROM
*/
int verificaEEPROM (struct modeloEnvolvente *mod) {
    uint16_t crcLeido, crcCal;
    
    leerEEPROM((uint8_t *)mod, sizeof(struct modeloEnvolvente), &crcLeido);
    crcCal = crc16((uint8_t *)mod, sizeof(struct modeloEnvolvente));
    if (crcCal == crcLeido) {
        return 0;
    } else {
        return 1;
    }
}

/*
Función que calcula la velocidad del sonido utilizando los datos de la ubicación del 
calibrador, que está ubicado a una distancia conocida.
Parámetros:
    nbuf -> cuantos buffers hay que esperar
    mitad -> si hay que considerar la mitad de un buffer
    pos -> posición real del calibrador en metros
Retorno:
    La velocidad del sonido en m/s
    0.0 si ha ocurrido algún error.
*/
float calculaVelocidadSonido (uint8_t nBuf, uint8_t mitad, float32_t pos){
    uint16_t *buf;
    uint32_t cont;
    float32_t max, *posMax, tof;
    float32_t velocidadSonido;
    
    if (nBuf == 0 || (mitad != '0' && mitad != '5')){
        return 0.0f;
    }
    
    iniciaEmisionYAdquisicion(nBuf);
    if (esperaFinAdquisicion()) {
        return 0.0f;
    }
    
    // Construimos el buffer y enviamos los datos a la PC
    if (mitad == '5')
        // la mitad del buffer anterior y el actual
        buf = datosAdquiridos(1);
    else 
        // el buffer actual
        buf = datosAdquiridos(0);
    
    // Calculamos la envolvente
    calcularEnvolvente(buf, envolvente);
    
    // filtramos la envolvente
    if (filtfilt_fir_cmsis(envolvente, N_PUNTOS_PROCESADOS, envolvente) < 0) {
        return 0.0f;
    }
    
    // Calculamos la derivada
    if (derivada_rfft_cmsis(envolvente, derivada, N_PUNTOS_PROCESADOS, FRECUENCIA_MUESTREO, 0, 0) < 0){
        return 0.0f;
    }
    
    // Normalizamos la salida
    normalize_peak_f32(derivada, envolvente, N_PUNTOS_PROCESADOS);

    // hacemos la correlación
    calcularCorrelacion (envolvente, N_PUNTOS_PROCESADOS, envAct.m, envAct.utilizado, resCor);
    
    // buscamos el máximo
    if (nBuf == 1) {
        // primer buf, hay que saltar la inducción de la emisión
        // TODO: ahora el salto es un valor constante, debería ser configurable
        // TODO: ahora el valor del umbral de detección es 0, debería ser variable
        maximo(resCor, 300, (sizeof(resCor)/sizeof(float32_t)) -1, 0, &max, &posMax);
        if (posMax == NULL) {
            // Error
            return 0.0f;
        }
    } else {
        maximo(resCor, 0, (sizeof(resCor)/sizeof(float32_t)) -1, 0, &max, &posMax);
        if (posMax == NULL) {
            // Error
            return 0.0f;
        }
    }
    // calculamos el tiempo de vuelo
    tof = ((((nBuf -1) * N_PUNTOS_PROCESADOS) + (posMax - resCor)) * 1/FRECUENCIA_MUESTREO) / 2;
    velocidadSonido = pos / tof;

    return velocidadSonido;    
}

int main(void)
{
    uint8_t cPC, cESP;
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    UART_PC_Start();
    UART_ESP_Start();
    EEPROM_Start();
    inicializaEmisionAdq();
    
    if (verificaEEPROM (&envAct))
        banderaErrorEEPROM = 1;
    else
        banderaErrorEEPROM = 0;
    
    for(;;)
    {
        /* Place your application code here. */
        // esperamos indefinidamente un comando
        for (cPC = 0, cESP = 0; (cPC = UART_PC_GetChar()) == 0 && (cESP = UART_ESP_GetChar()) == 0;);
        // el comando del ESP tiene prioridad sobre la PC
        // esta respuesta de ocupado será útil solamente cuando se recibe un mensaje 
        // simultáneamente. Si eso no es así entonces el primero que llega será 
        // atendido y el segundo deberá esperar a que termine el primer comando.
        if (cPC != 0 && cESP != 0) {    // si los dos tienen
            UART_PC_PutChar('O');   // indicamos que estamos ocupados.
            cPC = 0;                // borramos el comando de la PC.
        }
        
        // atendemos a los comandos del ESP
        /////////////////////////////
        // TODO: tal vez algunos comandos no tienen sentido para el ESP
        /////////////////////////////
        switch(cESP) {
            case 0:
                // no hacemos nada, 0 significa que no se recibió ningún carácter
                break;
            case 'a': case 'A':
                // comando de atención, para ver si está activo el sistema
                // envía también la bandera de error de la eeprom
                comandoA (UART_ESP_PutChar);
                break;
            case 'l': case 'L':
                // adquisición sin ningún procesamiento
                comandoL (UART_ESP_PutChar, UART_ESP_GetChar);
                break;
            case 'e': case 'E':
                // se realiza la adquisición y calcula la envolvente
                comandoE (UART_ESP_PutChar, UART_ESP_GetChar);
                break;
            case 'd': case 'D':
                // se realiza la adquisición y calcula la envolvente
                comandoD (UART_ESP_PutChar, UART_ESP_GetChar);
                break;
            case 'p': case 'P':
                // envía el estado de la EEPROM (estado de la bandera)
                comandoP (UART_ESP_PutChar);
                break;
            case 'm': case 'M':
                // recibe la estructura de datos que contienen el modelo de envolvente que se 
                // utilizará para la correlación y los datos sobre la ubicación del calibrador 
                // y el pelo del agua.
                comandoM (UART_ESP_GetRxBufferSize, UART_ESP_GetChar, UART_ESP_PutChar, &envRec);
                break;
            default:
                // nunca debería venir por aquí
                break;
        }
        
        switch(cPC) {
            case 0:
                // no hacemos nada, 0 significa que no se recibió ningún carácter
                break;
            case 'a': case 'A':
                // comando de atención, para ver si está activo el sistema
                // envía también la bandera de error de la eeprom
                comandoA (UART_PC_PutChar);
                break;
            case 'l': case 'L':
                // adquisición sin ningún procesamiento
                comandoL (UART_PC_PutChar, UART_PC_GetChar);
                break;
            case 'e': case 'E':
                // realiza la adquisición y calcula la envolvente
                comandoE (UART_PC_PutChar, UART_PC_GetChar);
                break;
            case 'd': case 'D':
                // realiza la adquisición, calcula la envolvente y realiza la derivada
                comandoD (UART_PC_PutChar, UART_PC_GetChar);
                break;
            case 'x': case 'X':
                // realiza la adquisición, calcula la envolvente, calcula la derivada y calcula la correlación
                comandoX (UART_PC_PutChar, UART_PC_GetChar);
                break;
            case 'p': case 'P':
                // envía el estado de la EEPROM (estado de la bandera)
                comandoP (UART_PC_PutChar);
                break;
            case 'm': case 'M':
                // recibe la estructura de datos que contienen el modelo de envolvente que se 
                // utilizará para la correlación y los datos sobre la ubicación del calibrador 
                // y el pelo del agua.
                if (comandoM (UART_PC_GetRxBufferSize, UART_PC_GetChar, UART_PC_PutChar, &envRec) == 0) {
                    // no ocurrió ningún error
                    // programamos la eeprom
                    if (escribirEEPROM((uint8_t *)&envRec, sizeof(struct modeloEnvolvente), crc16((uint8_t *)&envRec, sizeof(struct modeloEnvolvente))) == CYRET_SUCCESS) {
                        UART_PC_PutChar('c');
                        envAct = envRec;    // hacemos de la estructura recibida la actual
                        banderaErrorEEPROM = 0; // indicamos que no hay error en la EEPROM
                    } else {
                        banderaErrorEEPROM = 1; // indicamos que hay error en la EEPROM
                        UART_PC_PutChar('e');
                    }
                }
                break;
            case 'o': case 'O':
                // envía la estructura de datos que contienen el modelo de envolvente que se 
                // utiliza para la correlación y los datos sobre la ubicación del calibrador 
                // y el pelo del agua.
                if (verificaEEPROM(&envAct)) {
                    UART_PC_PutChar('e');
                    banderaErrorEEPROM = 1; // indicamos que hay error en la EEPROM
                } else 
                    comandoO (UART_PC_PutChar, &envAct);
                break;
            case 'c': case 'C':
                // intentamos determinar la posición exacta del calibrador según los datos actuales de 
                // la eeprom y los datos adquiridos.
                if (banderaErrorEEPROM){
                    UART_PC_PutChar('e');
                    break;
                } else {
                    comandoCalc (UART_PC_PutChar, UART_PC_GetChar, envAct.ubicacionCalibrador, 
                                 envAct.ubicacionCalibradorMedio);
                }
                break;
            case 'q': case 'Q':
                // intentamos determinar la posición exacta del pelo del agua según los datos actuales de 
                // la eeprom y los datos adquiridos.
                if (banderaErrorEEPROM){
                    UART_PC_PutChar('e');
                    break;
                } else {
                    comandoCalc (UART_PC_PutChar, UART_PC_GetChar, envAct.ubicacionAgua, 
                                 envAct.ubicacionAguaMedio);
                }
                break;
            case 'r': case 'R':
                // modificar solamente los datos de pelo de agua, calibrado y muestra a saltar en el primer buffer
                // en los datos almacenados en la EEPROM. Manteniendo sin modificación el modelo.
                if (verificaEEPROM(&envAct)) {
                    UART_PC_PutChar('e');
                    banderaErrorEEPROM = 1; // indicamos que hay error en la EEPROM
                } else {
                    if (comandoR (UART_PC_GetRxBufferSize, UART_PC_GetChar, UART_PC_PutChar, &envRec) == 0) {
                        // no ocurrió ningún error
                        // copiamos el resto de los datos de la estructura actual
                        envRec.utilizado = envAct.utilizado;
                        memcpy(envRec.m, envAct.m, TAM_MODELO_ENVOLVENTE);
                        // programamos la eeprom
                        if (escribirEEPROM((uint8_t *)&envRec, sizeof(struct modeloEnvolvente), crc16((uint8_t *)&envRec, sizeof(struct modeloEnvolvente))) == CYRET_SUCCESS) {
                            UART_PC_PutChar('c');
                            envAct = envRec;    // hacemos de la estructura recibida la actual
                            banderaErrorEEPROM = 0; // indicamos que no hay error en la EEPROM
                        } else {
                            banderaErrorEEPROM = 1; // indicamos que hay error en la EEPROM
                            UART_PC_PutChar('e');
                        }
                    }
                }
                break;
            default:
                // nunca debería venir por aquí
                break;
        }
    }
}

/* [] END OF FILE */
