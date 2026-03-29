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

#ifndef NORMALIZAR_H_
#define NORMALIZAR_H_

/*
Función que normaliza un vector utilizando la función CMSIS.
Parámetros:
    in -> puntero al vector float a normalizar
    out -> puntero al vector float de salida
    N -> cantidad de puntos de la señal
Retorno:
    Ninguno.
*/
void normalize_peak_f32(const float32_t *in, float32_t *out, uint32_t N);


#endif
/* [] END OF FILE */
