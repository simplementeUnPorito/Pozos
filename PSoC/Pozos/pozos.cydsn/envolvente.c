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
#include "arm_math.h"
#include "arm_common_tables.h"
#include "arm_const_structs.h"

#include "config_general.h"
#include "envolvente.h"

// Buffers para ejecutar el cálculo de la envolvente
#define HACER_FFT   0
#define HACER_IFFT  1

// variable local a este archivo que contiene el espectro en frecuencia de la señal
static float32_t espectro[2 * N_PUNTOS_PROCESADOS];

/*
Función que copia una señal real (vector de uint16_t) a un vector de float32_t con parte real
y parte imaginaria. El primer elemento del vector float es la parte real, el siguiente la parte
imaginaria y así sucesivamente.
Parámetros:
    dest -> puntero al vector complejo de destino
    ent  -> puntero al vector entero que contiene la señal
    cant -> cantidad de elementos que tiene el vector entero (ent). Corresponde a la cantidad de muestras adquiridas.
Retorno:
    Ninguno
*/
#ifdef CON_SIGNO
void poneParteReal (float32_t *dest, int16_t *ent, uint16_t cant) {
#else
void poneParteReal (float32_t *dest, uint16_t *ent, uint16_t cant) {
#endif
    while (cant--) {
        *dest++ = (float32_t)*ent++;   // parte real
        *dest++ = (float32_t)0;        // parte imaginaria
    }
}

/*
Función que corrige el espectro para obtener la señal analítica (transformada de Hilbert).
La operación que se realiza es: 
X[0] = 0
X[m] = 2*X[m] para 1 <= m <= N/2 -1
X[N/2] = X[N/2]
Todos los demás en 0.
El primer parámetro es un vector cuyo primer elemento es la parte real y el segundo la parte imaginaria; 
y así sucesivamente. El segundo parámetro es la cantidad de muestras, cada muestra corresponde a un 
valor complejo (dos elementos del vector) en el espectro.
Parámetros:
    dest -> puntero al vector complejo de destino
    cant -> cantidad de elementos que tiene el vector entero (ent)
Retorno:
    Ninguno
*/
void corregirEspectro (float32_t *dest, uint16_t cant) {
    *dest++ = (float32_t)0.0f;    // en 0 la componente contínua, es un valor complejo, esta es la parte real y la siguiente la imaginaria.
    *dest++ = (float32_t)0.0f;
    
    // hasta la mitad se multiplica por dos
    arm_scale_f32(dest, (float32_t)2.0f, dest, cant -2);
    // la última parte se pone a cero, excepto el primer elemento de las frecuencias negativas
    // cuyo valor se mantiene.
    dest += cant;
    arm_scale_f32(dest, (float32_t)0.0f, dest, cant -2);
}

/*
Función que calcula la envolvente de una señal real utilizando FFT.
Parámetros:
    entrada -> puntero al vector de la señal real.
    envolente -> puntero al vector donde se pone la salida. Debe contener TAM_BUFFER elementos
Retorno:
    puntero al vector que contiene la envolvente (float32_t)
*/
#ifdef CON_SIGNO
void *calcularEnvolvente (int16_t *entrada, float32_t *envolvente) {
#else
void *calcularEnvolvente (uint16_t *entrada, float32_t *envolvente) {
#endif
    // ejecutamos el FFT
    poneParteReal(espectro, entrada, N_PUNTOS_PROCESADOS);
    // BitReverseFlag tiene que estar en 1 para que el espectro tenga el formato estándar
    // Primero la parte positiva y seguido la parte negativa
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, espectro, HACER_FFT, 1);
    // corregimos el espectro
    corregirEspectro(espectro, N_PUNTOS_PROCESADOS);
    // ejecutamos la iFFT. Mantenemos el BitReverseFlag en uno
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, espectro, HACER_IFFT, 1);
    // calculamos el valor absoluto, o sea la envolvente
    arm_cmplx_mag_f32(espectro, envolvente, N_PUNTOS_PROCESADOS);
    
    return envolvente;
}

/* [] END OF FILE */
