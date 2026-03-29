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
#ifndef _DERIVADA_H
#define _DERIVADA_H
    
#include "project.h"

/* 
Función que calcula la derivada utilizando FFT.
Parámetros:
    x_in -> señal de entradad
    dx_out -> derivada calculada
    N -> cantidad de puntos de la señal de entrada
    fs -> frecuencia de muestreo
    scale_per_sample -> bandera para realizar el escalado a la salida
    do_detrend -> Bandera para realizar el detrend
Retorno:
     0 -> no hay error
    -1 -> error en la cantidad de puntos
*/
int derivada_rfft_cmsis(const float *x_in, float *dx_out,
                        uint32_t N, float fs, int scale_per_sample, int do_detrend);

#endif
/* [] END OF FILE */
