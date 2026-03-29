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

// Coeficientes del filtro paso bajos para filtrar la envolvente
// diseñado en python con numpy.
#define N_TAPS_PASO_BAJOS   135
static const float32_t filtroPB [N_TAPS_PASO_BAJOS] = {
  2.39251767e-05, -4.87858907e-05, -1.24434634e-04, -2.03082225e-04,
 -2.84346630e-04, -3.67173633e-04, -4.49649331e-04, -5.28875205e-04,
 -6.00923735e-04, -6.60887957e-04, -7.03032664e-04, -7.21048351e-04,
 -7.08401935e-04, -6.58771043e-04, -5.66541824e-04, -4.27344089e-04,
 -2.38592612e-04,  5.12496289e-19,  2.85975156e-04,  6.13780745e-04,
  9.74557653e-04,  1.35608924e-03,  1.74293611e-03,  2.11677028e-03,
  2.45691319e-03,  2.74107136e-03,  2.94625255e-03,  3.04983412e-03,
  3.03074531e-03,  2.87071587e-03,  2.55553632e-03,  2.07627022e-03,
  1.43035632e-03,  6.22538949e-04, -3.34431377e-04, -1.41937879e-03,
 -2.60260658e-03, -3.84615590e-03, -5.10443499e-03, -6.32522015e-03,
 -7.45101296e-03, -8.42072197e-03, -9.17162114e-03, -9.64152262e-03,
 -9.77108895e-03, -9.50620022e-03, -8.80028465e-03, -7.61651864e-03,
 -5.92980303e-03, -3.72842747e-03, -1.01534358e-03,  2.19101960e-03,
  5.85644890e-03,  9.93118151e-03,  1.43506927e-02,  1.90370280e-02,
  2.39006484e-02,  2.88427374e-02,  3.37578997e-02,  3.85371624e-02,
  4.30711788e-02,  4.72535233e-02,  5.09839615e-02,  5.41715770e-02,
  5.67376429e-02,  5.86181323e-02,  5.97657734e-02,  6.01515768e-02,
  5.97657734e-02,  5.86181323e-02,  5.67376429e-02,  5.41715770e-02,
  5.09839615e-02,  4.72535233e-02,  4.30711788e-02,  3.85371624e-02,
  3.37578997e-02,  2.88427374e-02,  2.39006484e-02,  1.90370280e-02,
  1.43506927e-02,  9.93118151e-03,  5.85644890e-03,  2.19101960e-03,
 -1.01534358e-03, -3.72842747e-03, -5.92980303e-03, -7.61651864e-03,
 -8.80028465e-03, -9.50620022e-03, -9.77108895e-03, -9.64152262e-03,
 -9.17162114e-03, -8.42072197e-03, -7.45101296e-03, -6.32522015e-03,
 -5.10443499e-03, -3.84615590e-03, -2.60260658e-03, -1.41937879e-03,
 -3.34431377e-04,  6.22538949e-04,  1.43035632e-03,  2.07627022e-03,
  2.55553632e-03,  2.87071587e-03,  3.03074531e-03,  3.04983412e-03,
  2.94625255e-03,  2.74107136e-03,  2.45691319e-03,  2.11677028e-03,
  1.74293611e-03,  1.35608924e-03,  9.74557653e-04,  6.13780745e-04,
  2.85975156e-04,  5.12496289e-19, -2.38592612e-04, -4.27344089e-04,
 -5.66541824e-04, -6.58771043e-04, -7.08401935e-04, -7.21048351e-04,
 -7.03032664e-04, -6.60887957e-04, -6.00923735e-04, -5.28875205e-04,
 -4.49649331e-04, -3.67173633e-04, -2.84346630e-04, -2.03082225e-04,
 -1.24434634e-04, -4.87858907e-05,  2.39251767e-05
};

// tamaño máximo del pad necesario
#define MAX_PADLEN  (3u * (N_TAPS_PASO_BAJOS - 1u))

// Buffers estáticos para el filtfilt
//float32_t *x_ext = (float32_t *)malloc(sizeof(float32_t) * L);
static float32_t x_ext[N_PUNTOS_PROCESADOS + 2 * MAX_PADLEN];
//float32_t *conv_out = (float32_t *)malloc(sizeof(float32_t) * convLen);
static float32_t conv_out[(N_PUNTOS_PROCESADOS + 2 * MAX_PADLEN) + N_TAPS_PASO_BAJOS - 1u];
//float32_t *tmp = (float32_t *)malloc(sizeof(float32_t) * L); // para resultados intermedios
static float32_t tmp[N_PUNTOS_PROCESADOS + 2 * MAX_PADLEN];

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
                       float32_t *y) {
    if (N < 1) return -1;
    
    //if (M > N_TAPS_PASO_BAJOS) return -3;
    if (N > N_PUNTOS_PROCESADOS) return -4;
    
    uint32_t padlen = 3 * (N_TAPS_PASO_BAJOS - 1u);
    if (N <= padlen) return -1;       // igual que MATLAB: señal demasiado corta

    uint32_t L = N + 2 * padlen;      // longitud de la señal extendida
    //uint32_t convLen = L + M - 1u;    // longitud del resultado de la convolución

    // 1) Construir la señal extendida por reflexión (igual que MATLAB):
    // left: 2*x[0] - x[1..padlen] (revertido)
    // middle: x[0..N-1]
    // right: 2*x[N-1] - x[N-2 .. N-1-padlen] (revertido)

    // Left pad:
    for (uint32_t i = 0; i < padlen; ++i) {
        // correspondencia: left[padlen-1 - i] = 2*x[0] - x[1 + i]
        uint32_t idx_src = 1u + i; // x[1 + i]
        uint32_t idx_dst = padlen - 1u - i;
        x_ext[idx_dst] = 2.0f * x[0] - x[idx_src];
    }

    // Middle (original signal)
    memcpy(&x_ext[padlen], x, sizeof(float32_t) * N);

    // Right pad:
    for (uint32_t i = 0; i < padlen; ++i) {
        // right[i] = 2*x[N-1] - x[N-2 - i]
        uint32_t idx_src = (N >= 2 + i) ? (N - 2u - i) : 0u;
        x_ext[padlen + N + i] = 2.0f * x[N - 1u] - x[idx_src];
    }

    // 2) Filtrado hacia adelante: conv(x_ext, B)
    arm_conv_f32(x_ext, L, filtroPB, N_TAPS_PASO_BAJOS, conv_out);

    // Extraer la parte válida (del conv_out): indices [M-1 .. M-1 + L - 1]
    for (uint32_t i = 0; i < L; ++i) {
        tmp[i] = conv_out[i + (N_TAPS_PASO_BAJOS - 1u)];
    }

    // 3) Invertir tmp en x_ext para reutilizar buffers (ahora tmp es y_fwd)
    for (uint32_t i = 0; i < L; ++i) {
        x_ext[i] = tmp[L - 1u - i];
    }

    // 4) Filtrado de la señal invertida
    arm_conv_f32(x_ext, L, filtroPB, N_TAPS_PASO_BAJOS, conv_out);

    // 5) Extraer parte válida de conv_out y revertir otra vez:
    for (uint32_t i = 0; i < L; ++i) {
        tmp[i] = conv_out[i + (N_TAPS_PASO_BAJOS - 1u)]; // tmp = filtered(reversed(filtered(pad(x))))
    }

    // revertir tmp y tomar el centro correspondiente a la señal original
    // salida y[i] = tmp[padlen + i] (pero primero revertimos)
    for (uint32_t i = 0; i < N; ++i) {
        // after reverse: index in tmp reversed = L-1 - (padlen + i) = L - padlen - 1 - i
        uint32_t idx_rev = L - padlen - 1u - i;
        y[i] = tmp[idx_rev];
    }

    return 0;
}
                    
/* [] END OF FILE */