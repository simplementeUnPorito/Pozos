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

//#define TAM_BUF (2048)
//#define CON_SIGNO

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
//#ifdef CON_SIGNO
//void poneParteReal (float32_t *dest, int16_t *ent, uint16_t cant);
//#else
//void poneParteReal (float32_t *dest, uint16_t *ent, uint16_t cant);
//#endif

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
//void corregirEspectro (float32_t *dest, uint16_t cant);

/*
Función que calcula la envolvente de una señal real utilizando FFT.
Parámetros:
    entrada -> puntero al vector de la señal real.
    envolente -> puntero al vector donde se pone la salida
Retorno:
    puntero al vector que contiene la envolvente (float32_t)
*/
#ifdef CON_SIGNO
void *calcularEnvolvente (int16_t *entrada, float32_t *envolvente);
#else
void *calcularEnvolvente (uint16_t *entrada, float32_t *envolvente);
#endif
/* [] END OF FILE */
