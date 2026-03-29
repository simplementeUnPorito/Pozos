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

#ifndef EMISION_ADQ
#define EMISION_ADQ
/*
Función que prepara lo necesario para realizar la emisión.
Parámetros:
    Ninguno
Retorno:
    Ninguno
*/
void inicializaEmisionAdq ();

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
int esperaFinAdquisicion ();

/*
Función que hace la emisión e inicia la adquisición. Recibe como parámetro las veces que el buffer
del convertidor AD va a ser llenado (o las veces que se va a producir la interrupción
del fin del DMA correspondiente).
Parámetros:
    vecesDMA -> cuantos buffers hay que esperar
Retorno:
    Ninguno
*/
void iniciaEmisionYAdquisicion (uint16_t vecesDMA);

/*
Función que retorna la dirección del buffer de datos donde se encuentran
las 2048 muestras adquiridas.
Parámetros:
    medio -> si vale 1 se copian las últimas 1023 muestras del primer buffer y 
             las primeras 1025 muestras del segundo buffer. Si vale 0 solo se copia
             el primer valor del segundo buffer al primero.
Retorno:
    Dirección de adcBuffer
*/
uint16_t *datosAdquiridos (uint8_t medio);
#endif

/* [] END OF FILE */
