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
/* derivada_rfft_cmsis.c
   Uso de arm_rfft_fast_f32 para calcular la derivada por FFT sin malloc.
   Requiere CMSIS-DSP: arm_math.h
*/

#include "arm_math.h"
#include <math.h>

#include "config_general.h"
//#define MAX_N    2048    /* Ajusta a la potencia de 2 que necesites (por ejemplo 512, 1024) */
//#define PI       3.14159265358979323846f

/* Buffers estáticos */
//static float tmp_x[MAX_N];           /* señal real temporal (N) */
static float32_t rfft_buf[N_PUNTOS_PROCESADOS];        /* salida RFFT (N floats: interleaved complex) */
/* solo necesitamos omega para k = 0..N/2 */
static float32_t omega_buf[(N_PUNTOS_PROCESADOS/2) + 1];
/* Inicializar instancia RFFT */
static arm_rfft_fast_instance_f32 S;

/* preparar vector omega para k=0..N/2 */
static void prepare_omega_rfft(float32_t fs, uint32_t N)
{
    const float factor = 2.0f * PI * fs / (float)N;
    uint32_t half = N / 2;
    for (uint32_t k = 0; k <= half; k++) {
        //int32_t kk = (int32_t)k; /* k no negativo en esta representación */
        omega_buf[k] = factor * (float)k; /* rad/s para bin k */
    }
}

/* detrend (misma implementación simple que antes) */
static void detrend_inplace(float32_t *data, uint32_t N)
{
    float mean_x = 0.0f;
    float mean_n = (N - 1) / 2.0f;
    for (uint32_t i = 0; i < N; i++) mean_x += data[i];
    mean_x /= (float)N;
    float num = 0.0f, den = 0.0f;
    for (uint32_t i = 0; i < N; i++) {
        float dn = (float)i - mean_n;
        num += dn * (data[i] - mean_x);
        den += dn * dn;
    }
    float slope = (den == 0.0f) ? 0.0f : (num / den);
    float intercept = mean_x - slope * mean_n;
    for (uint32_t i = 0; i < N; i++) 
        data[i] -= (slope * (float)i + intercept);
}

/* 
Función que calcula la derivada utilizando FFT.
Parámetros:
    x_in             -> señal de entrada
    dx_out           -> derivada calculada
    N                -> cantidad de puntos de la señal de entrada
    fs               -> frecuencia de muestreo
    scale_per_sample -> bandera para realizar el escalado a la salida
    do_detrend       -> Bandera para realizar el detrend
Retorno:
     0 -> no hay error
    -1 -> error en la cantidad de puntos
*/
int derivada_rfft_cmsis(float32_t *x_in, float32_t *dx_out,
                        uint32_t N, float32_t fs, int scale_per_sample, int do_detrend)
{
    if (N == 0) return -1;
    if (N > N_PUNTOS_PROCESADOS) return -1;

    if (arm_rfft_fast_init_f32(&S, N) != ARM_MATH_SUCCESS) {
        return -1; /* inicialización falló (N no soportada) */
    }

    /* preparar omegas solo para 0..N/2 */
    prepare_omega_rfft(fs, N);

	if (do_detrend) 
		detrend_inplace(x_in, N);

	/* Forward RFFT: real[N] -> packed complex (N floats interleaved) */
	arm_rfft_fast_f32(&S, x_in, rfft_buf, 0); /* forward */

	/* Multiplicar bins k=0..N/2 por j*omega_k.
	   rfft_buf está en formato interleaved: re0,im0, re1,im1, ..., re(N/2),im(N/2), ... (solo primeros N/2+1 son únicos)
	*/
	uint32_t half = N / 2;
	for (uint32_t k = 0; k <= half; k++) {
		float a = rfft_buf[2 * k];     /* re */
		float b = rfft_buf[2 * k + 1]; /* im */
		float w = omega_buf[k];        /* rad/s */
		/* j*omega*(a + j b) = -omega*b + j*(omega*a) */
		rfft_buf[2 * k]     = -w * b;
		rfft_buf[2 * k + 1] =  w * a;
	}

	/* Inverse RFFT: packed complex (N floats) -> real[N] */
	arm_rfft_fast_f32(&S, rfft_buf, x_in, 1); /* inverse */

	/* CMSIS no escala automáticamente por 1/N en IFFT interno (igual que cfft), así que escalamos */
	float invN = 1.0f / (float)N;
	for (uint32_t n = 0; n < N; n++) {
		float val = x_in[n] * invN; /* real result */
		if (scale_per_sample) 
            val /= fs; /* opcional conversion per sample */
		dx_out[n] = val;
	}

    return 0;
}
/* [] END OF FILE */
