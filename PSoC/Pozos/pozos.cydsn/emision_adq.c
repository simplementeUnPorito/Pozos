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

#include "senal_a_emitir.h"
#include "config_general.h"

//////////////////////////////////////////////////////////
// Bandera y rutina de servicio de la interrupción del timeout de adquisición
//////////////////////////////////////////////////////////
// Bandera que se activa cuando transcurren 5 segundos.
volatile uint8 timeout_flag = 0;

/*
Rutina de servicio a la interrupción del temporizador de 5 segundos.
Parámetros: Ninguno
Retorno: Ninguno
*/
CY_ISR(Timeout_ISR){
    TimeoutAdquisicion_ReadStatusRegister();  // Limpia interrupción
    TimeoutAdquisicion_Stop();                // detener si es one-shot
    timeout_flag = 1;
}

//////////////////////////////////////////////////////////
// buffer y variables para el ADC
//////////////////////////////////////////////////////////
#define CANT_MAX_OPERACIONES_DMA    (2 * N_PUNTOS_PROCESADOS)          // Cantidad de operaciones (copia de byte) para un TD del DMA
#define TAM_ADC_BUF (CANT_MAX_OPERACIONES_DMA/2)    // Tamaño máximo de un buffer de un TD del DMA cuando cada valor es de 16 bits
// El valor máximo de la cantidad de bytes que el DMA puede copiar por cada descriptor es 4094
// pero en esta aplicación se utilizan 2048 muestras. 
// Para hacer eso se crean los buffer con 2048 posiciones, de tal forma que 
// se pueda construir en ellos la señal que vamos a procesar posteriormente.
static uint16_t adcBuffer[TAM_ADC_BUF];
static uint16_t adcBuffer1[TAM_ADC_BUF];

// variable que identifica el buffer utilizado en la última interrupción.
// Si vale 1 es el adqBuffer y si vale 2 es el adqBuffer1.
static volatile uint8_t identificadorBufferUsado = 0;
// Bandera que se activa cuando se termina la adquisición del convertidor AD.
static volatile uint8_t bandFinADC = 0;
// Aquí el programa debe poner la cantidad de veces que el DMA complete debe producirse
static volatile uint16_t vecesDMAcomplete = 0;
// Esta variable se utiliza en la rutina de servicio de la interrupción del DMA solamente
// Cuenta la cantidad de veces que activa esta rutina de interrupción
static volatile uint16_t contadorDMAcomplete = 0;

///////////////////////////////////
// Generado por el DMA Wizard.
///////////////////////////////////
/* Defines for DMA_ADC */
#define DMA_ADC_BYTES_PER_BURST 2
#define DMA_ADC_REQUEST_PER_BURST 1
#define DMA_ADC_SRC_BASE (CYDEV_PERIPH_BASE)
#define DMA_ADC_DST_BASE (CYDEV_SRAM_BASE)

/* Variable declarations for DMA_ADC */
/* Move these variable declarations to the top of the function */
static uint8 DMA_ADC_Chan;
static uint8 DMA_ADC_TD[2];

/* Defines for DMA_DAC */
#define DMA_DAC_BYTES_PER_BURST 1
#define DMA_DAC_REQUEST_PER_BURST 1
#define DMA_DAC_SRC_BASE (senhalAEmitir)
#define DMA_DAC_DST_BASE (CYDEV_PERIPH_BASE)

/* Variable declarations for DMA_DAC */
/* Move these variable declarations to the top of the function */
static uint8 DMA_DAC_Chan;
static uint8 DMA_DAC_TD[1];
///////////////////////////////////
// Fin Generado por el DMA Wizard.
///////////////////////////////////

/*
Función que inicializa el DMA para el ADC. 
Generado por el DMA Wizard.
Parámetros:
    Ninguno
Retonro:
    Ninguno
*/
void inicDMA_ADC () {
    /* DMA Configuration for DMA_ADC */
    DMA_ADC_Chan = DMA_ADC_DmaInitialize(DMA_ADC_BYTES_PER_BURST, DMA_ADC_REQUEST_PER_BURST, 
        HI16(DMA_ADC_SRC_BASE), HI16(DMA_ADC_DST_BASE));
    DMA_ADC_TD[0] = CyDmaTdAllocate();
    DMA_ADC_TD[1] = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(DMA_ADC_TD[0], CANT_MAX_OPERACIONES_DMA, DMA_ADC_TD[1], DMA_ADC__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
    CyDmaTdSetConfiguration(DMA_ADC_TD[1], CANT_MAX_OPERACIONES_DMA, DMA_ADC_TD[0], DMA_ADC__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
    CyDmaTdSetAddress(DMA_ADC_TD[0], LO16((uint32)ADC_SAR_SAR_WRK0_PTR), LO16((uint32)adcBuffer));
    CyDmaTdSetAddress(DMA_ADC_TD[1], LO16((uint32)ADC_SAR_SAR_WRK0_PTR), LO16((uint32)adcBuffer1));
    CyDmaChSetInitialTd(DMA_ADC_Chan, DMA_ADC_TD[0]);
    CyDmaChEnable(DMA_ADC_Chan, 1);
}

/*
Función que inicializa el DMA para el DAC. 
Generado por el DMA Wizard.
Parámetros:
    Ninguno
Retonro:
    Ninguno
*/
void inicDMA_DAC () {
    /* DMA Configuration for DMA_DAC */
    DMA_DAC_Chan = DMA_DAC_DmaInitialize(DMA_DAC_BYTES_PER_BURST, DMA_DAC_REQUEST_PER_BURST, 
        HI16(DMA_DAC_SRC_BASE), HI16(DMA_DAC_DST_BASE));
    DMA_DAC_TD[0] = CyDmaTdAllocate();
//                          DMA_DAC_TD[0], SINETABLE_SIZE, CY_DMA_DISABLE_TD, CY_DMA_TD_INC_SRC_ADR | CY_DMA_TD_AUTO_EXEC_NEXT
    CyDmaTdSetConfiguration(DMA_DAC_TD[0], SIGNAL_LENGTH, DMA_DAC_TD[0], CY_DMA_TD_INC_SRC_ADR);    //CY_DMA_DISABLE_TD
    CyDmaTdSetAddress(DMA_DAC_TD[0], LO16((uint32)senhalAEmitir), LO16((uint32)VDAC8_1_Data_PTR));
    CyDmaChSetInitialTd(DMA_DAC_Chan, DMA_DAC_TD[0]);
    CyDmaChEnable(DMA_DAC_Chan, 1);
}

/*
Rutina de servicio de la interrupción para el fin de adquisición por DMA. 
Cuando el convertidor ha realizado la cantidad de muestras solicitadas, el DMA genera
está interrupción.
Parámetros: ninguno
Retorno: ninguno
*/
CY_ISR(isr_DMA_ADC) {
    /* ISR Code here */
    if (++contadorDMAcomplete >= vecesDMAcomplete) {
        Inic_DAC_Write(0);  // detenemos la adquisición
        // TODO: verificar que el procesador es lo suficientemente rápido para parar la 
        // adquisición antes de que produzca una nueva adquisición. --> VERIFICADO <--
        contadorDMAcomplete = 0;
        
        // Avisamos que terminó la adquisición
        bandFinADC = 1;
    }
    
    // independientemente de si la adquisición termina o no, es necesario saber cual será 
    // el buffer a ser utilizado la próxima vez.
    if (!identificadorBufferUsado) {
        // esta es la primera que se produce la interrupción
        identificadorBufferUsado = 1;
    } else if (identificadorBufferUsado == 1) {
        // si estamos en el buffer 1, pasamos a 2
        identificadorBufferUsado = 2;
    } else {
        // si estamos en el buffer 2, pasamos a 1
        identificadorBufferUsado = 1;
    }   // TODO: aquí habría que ver otros posibles valores
}

/*
Función que hace la emisión e inicia la adquisición. Recibe como parámetro las veces que el buffer
del convertidor AD va a ser llenado (o las veces que se va a producir la interrupción
del fin del DMA correspondiente). Se utilizan dos buffers, cada vez que uno de ellos se llena
se pasa al otro y se produce una interrupción. Al parámetro actual se le suma 1 porque a cada adquisición
le corresponden 2 adquisiciones para completar las muestras necesarias, los 2048 valores.
Si veces DMA es 1 
///////////////////////////////////////////////////////////////////////////////////////
Esta función es un loop infinito. Habría que ver de cambiar eso y que tenga un timeout
///////////////////////////////////////////////////////////////////////////////////////
Parámetros:
    vecesDMA -> cuantos buffers hay que esperar
Retorno:
    Ninguno
*/
void iniciaEmisionYAdquisicion (uint16_t vecesDMA) {
    // Inicializamos la máquina de estados del contador de DMA complete
   
    vecesDMAcomplete = vecesDMA +1;
    contadorDMAcomplete = 0;
    bandFinADC = 0;
    
    // iniciamos la adquisición
    Inic_DAC_Write(1);
}

/*
Función que prepara lo necesario para realizar la emisión.
Parámetros:
    Ninguno
Retorno:
    Ninguno
*/
void inicializaEmisionAdq () {
    Opamp_eco_Start();
    Opamp_antia1_Start();
    Opamp_antia2_Start();
    VDAC8_1_Start();
    ADC_SAR_Start();
    inicDMA_DAC();
    inicDMA_ADC();
    isr_adc_complete_StartEx(isr_DMA_ADC);
    Inic_DAC_Write(0);
    identificadorBufferUsado = 0;
    VDAC8_1_SetValue(0x7F);
    isr_timeout_StartEx(Timeout_ISR);

}

/*
Función que espera que termine la adquisición de los datos solicitados.
Espera indefinidamente. Implementa un timeout de 5 segundos, que es el
tiempo máximo de muestro.
Parámetros:
    Ninguno
Retorno:
     0 si no ocurrió un timeout
    -1 si ocurrió un timeout
*/
int esperaFinAdquisicion () {
    timeout_flag = 0;
    TimeoutAdquisicion_Start();       // arranca el timer para 5 s
    
    while (!bandFinADC) {
        if (timeout_flag) {     // timeout alcanzado
            return -1;
            break;
        }        
    }
    bandFinADC = 0;
    Inic_DAC_Write(0);
    return 0;
}

/* 
Función que mezcla dos vectores y crea uno. Los vectores pueden estar solapados.
Toma la segunda mitad del vector a y lo pone en la primera parte del vector a; 
luego toma la primera mitad del vector b y completa el vector a. El vector de
salida tiene un elemento más que los dos vectores de entrada. Los tamaños están
definidos en las constantes TAM_ADC_BUF.
    a -> primer vector de entrada (uint16_t[TAM_ADC_BUF])
    b -> segundo vector de entrada (uint16_t[TAM_ADC_BUF])
    out -> vector de salida (uint16_t[TAM_ADC_BUF +1]) - puede ser == a o == b
Retorno: 
    Ninguno
 */
#define MEDIO_BUF ((TAM_ADC_BUF) / 2) /* 512 */
void mezclaBuffer(const uint16_t *a, const uint16_t *b, uint16_t *out)
{
    /* Copiar la segunda mitad de a (últimos MEDIO_BUF elementos) al inicio de out */
    const uint16_t *a_src = a + MEDIO_BUF; /* apunta a a[1023] */
    memmove(out, a_src, (size_t)MEDIO_BUF * sizeof(uint16_t));

    /* Copiar los primeros MEDIO_BUF elementos de b tras la parte copiada antes*/
    memmove(out + MEDIO_BUF, b, (size_t)MEDIO_BUF * sizeof(uint16_t));
}

/*
Función que retorna la dirección del buffer de datos donde se encuentran
las 2048 muestras adquiridas.
Parámetros:
    medio -> si vale 1 se copian las últimas 1023 muestras del primer buffer y 
             las primeras 1025 muestras del segundo buffer. Si vale 0 solo se copia
             el primer valor del segundo buffer al primero. Si vale 5 entonces
             se requiere el segundo buffer, y como falta un valor, se copia el
             el último valor adquirido a la última posición del buffer.
Retorno:
    Dirección de adcBuffer o adcBuffer1 según sea adecuado.
    NULL si el parámetro no es correcto.
*/
#define CANTIDAD_DATOS_COPIAR_BUF   1023
#define CANTIDAD_DATOS_COPIAR_BUF1  1025
uint16_t *datosAdquiridos (uint8_t medio) {
    if (!medio) {
        // se usa casi todo el buffer actual, solo se copia una muestra del otro
        if (identificadorBufferUsado == 2) {
            // si identificadorBufferUsado es 2, esto es el último buffer rellenado es adcBuffer1,
            // por lo tanto, es el buffer adcBuffer el que contiene toda la señal, 
            // menos una muestra
            //adcBuffer[TAM_ADC_BUF] = adcBuffer1[0]; // completamos la muestra que le falta
            return adcBuffer;
        } else {
            // si el valor identificadorBufferUsado == 1 quiere decir que el último
            // buffer rellenado es el adcBuffer, por lo tanto, la mayor parte de la 
            // señal está en adcBuffer1
            //adcBuffer1[TAM_ADC_BUF] = adcBuffer[0]; // completamos la muestra que le falta
            return adcBuffer1;
        }
    } else if (medio) { // el valor en medio es 1. Eso indica que se usa medio buffer actual y anterior
        // usa una parte del actual y otra parte del anterior
        if (identificadorBufferUsado == 2) {
            // si identificadorBufferUsado es 2, esto es el último buffer rellenado es adcBuffer1,
            // por lo tanto, es el buffer adcBuffer el que contiene casi la mitad de la señal 
            // (en la segunda mitad), se completa con adcBuffer1.
            // construimos el buffer: ponemos la mitad de adcBuffer al inicio y completamos
            // el resto con la mitad de adcBuffer1
            mezclaBuffer (adcBuffer, adcBuffer1, adcBuffer);
            return adcBuffer;
        } else {
            // si el valor identificadorBufferUsado == 1 quiere decir que el último
            // buffer rellenado es el adcBuffer, por lo tanto, adcBuffer1 contiene la mitad de  
            // la señal (en la segunda mitad) y la primera mitad de adcBuffer1 completa el buffer
            // construimos el buffer: ponemos la mitad de adcBuffer1 al inicio y completamos
            // el resto con la mitad de adcBuffer
            mezclaBuffer (adcBuffer1, adcBuffer, adcBuffer1);
            return adcBuffer1;
        }
    } // TODO, falta un else e identificar el error
    return NULL;    // el parámetro no es correcto
}

/* [] END OF FILE */
