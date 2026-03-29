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

#ifndef CONFIG_GENERAL_H
#define CONFIG_GENERAL_H

#define VELOCIDAD_DEL_SONIDO    340 // en m/s
#define N_PUNTOS_PROCESADOS (1024u)
#define FRECUENCIA_MUESTREO (60000.0f)

// Variables que se utilizan para almacenar el modelo en la EEPROM
#define TAM_MODELO_ENVOLVENTE   (300u)
struct modeloEnvolvente {
    float32_t m[TAM_MODELO_ENVOLVENTE];
    uint16_t utilizado;             // Cantidad de posiciones que se utiliza realmente.
    uint8_t ubicacionCalibrador;    // Ubicación del calibrador en número de buffer de 3 m cada uno
    uint8_t ubicacionCalibradorMedio;   // solo puede valer 0 o 5 que indica un buffer entero o medio y medio
    float32_t nivelMinCalibrador;   // Nivel mínimo de detección en la salida de la correlación
    float32_t posicionCalibrador;   // Distancia a la que se encuentra el calibrador con respecto al emisor
    uint8_t ubicacionAgua;          // Misma codificación de ubicacionCalibrador.
    uint8_t ubicacionAguaMedio;
    float32_t nivelMinAgua;         // Nivel mínimo de detección en la salida de la correlación
    uint16_t saltar;                // El número de muestras que hay que saltar en el primer buffer por la emisión
};

// Este valor corresponde a la cantidad de bytes que suman los datos del struct modeloEnvolvente
// desde utilizado hasta saltar:
// utilizado -> 2
// ubicacionCalibrador -> 1 
// ubicacionCalibradorMedio -> 1
// nivelMinCalibrador -> 4
// posicionCalibrador -> 4
// ubicacionAgua -> 1
// ubicacionAguaMedio -> 1
// nivelMinAgua -> 4
// saltar -> 2
// TOTAL: 20 bytes
#define TAM_BUF_TEMP (20u)

#endif
/* [] END OF FILE */
