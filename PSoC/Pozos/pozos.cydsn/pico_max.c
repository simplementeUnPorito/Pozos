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
#include <math.h>

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
int maximo(float *entrada, uint32_t start_sample, uint32_t end_sample, float umbral, float *maxValor, float **posicion){
    *maxValor = -1*INFINITY;		// máximo valor negativo
    *posicion = NULL;			// posición no válida
    uint8_t bandera_maximo = 0;

    // El primero y el último no pueden ser los máximos
    for(float *i = (entrada + start_sample +1); i < (entrada + end_sample); i++){
        if (*i > *(i -1) && *i >= *(i +1) && *i > umbral){
            // Encontramos un posible maximo
	        	if (*i > *maxValor) {
	        		// el actual es mayor que el anterior máximo
					*maxValor = *i;
					*posicion = i;
					bandera_maximo = 1;
	        	}
        }
    }

    //printf("Calibrador encontrado en la posicion %d con valor maximo %f\r\n", *posicion,*maxValor);
	if (bandera_maximo)
		return 0; // Si no encontramos el máximo. El valor de posición es -1 y
				  // el máximo es -INFINITO
	else
		return -1; // error, no hay máximo mayor que el umbral
}

/* [] END OF FILE */
