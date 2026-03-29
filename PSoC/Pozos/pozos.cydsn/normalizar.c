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
#include "config_general.h"

/*
Función que normaliza un vector utilizando la función CMSIS.
Parámetros:
    in -> puntero al vector float a normalizar
    out -> puntero al vector float de salida
    N -> cantidad de puntos de la señal
Retorno:
    Ninguno.
*/
void normalize_peak_f32(const float32_t *in, float32_t *out, uint32_t N) {
    float32_t maxVal;
    uint32_t index;
    float32_t eps = 1e-12f;

    // 1) obtener vector de valores absolutos (puedes usar buffer temporal)
    arm_abs_f32(in, out, N);

    // 2) obtener el máximo absoluto
    arm_max_f32(out, N, &maxVal, &index);

    // 3) evitar division por cero
    if (maxVal <= eps) {
        // Señal casi nula: copia 0 o deja como está
        for (uint32_t i = 0; i < N; i++) out[i] = 0.0f;
        return;
    }

    // 4) escalar: out = in * (1/maxVal)
    float32_t scale = 1.0f / maxVal;
    arm_scale_f32(in, scale, out, N);
}
/* [] END OF FILE */
