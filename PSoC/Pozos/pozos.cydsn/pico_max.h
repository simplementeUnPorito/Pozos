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

#ifndef PICO_MAX_H_
#define PICO_MAX_H_
    
/*
Función que busca el pico máximo en un vector de floats. Se establece un umbral por
debajo del cual no se debe buscar el máximo.
El máximo se define así: vector[max -1] < vector[max] >= vector[max +1]
Parámetros:
    entrada -> vector donde se busca el máximo. Es una variable global
    start_sample -> índice de inicio del vector donde se busca el máximo
    end_sample -> índice del final del vector donde se busca el máximo
    umbral -> el máximo debe ser mayor que este valor
    maxValor -> valor del máximo detectado
    posicion -> puntero del máximo detectado
Retorno:
    En maxValor y posicion deben quedar el valor del máximo detectado y el puntero al
    vector CORRELACION.
    NULL en la posición indica también que no se encontró el máximo.
     0 -> si encontró el máximo
    -1 -> si no se encontró un máximo
*/
int maximo(float *entrada, uint32_t start_sample, uint32_t end_sample, float umbral, float *maxValor, float **posicion);

#endif
/* [] END OF FILE */
