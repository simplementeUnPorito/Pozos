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

/*
Función que funciona igual que la función filtfilt de matlab.
Parámetros:
  B -> coeficientes del FIR (longitud M)
  M -> número de taps
  x -> señal de entrada (longitud N)
  N -> longitud de x
  y -> puntero al vector de salida de longitud N
Retorna: 
     0 = OK.
    -1 = señal demasiado corta.
    -3 = señal demasiado larga.
    -4 = demasiados taps
*/
/*const float32_t *B, uint32_t M,*/
int filtfilt_fir_cmsis(const float32_t *x, uint32_t N,
                       float32_t *y);

/* [] END OF FILE */
