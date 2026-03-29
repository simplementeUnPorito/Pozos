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

#ifndef CORRELACION_H_
#define CORRELACION_H_

/*
Función para calcular la correlación entre dos señales de diferentes tamaños.
Esta función "busca" una señal dentro de otra, por lo que se supone que una señal
será más pequeña (la que se busca) y otra más grande (la buscada).
La búsqueda se hace corriendo la señal buscada hasta que el último elemento de esta
señal coincida con el último elemento de la señal donde se busca. Por lo que la salida
tiene el tamaño size1 - size2 +1.
Parámetros:
    signal1 -> puntero a la primera señal, donde se busca, más grande
    size1 -> cantidad de elementos de signal1
    signal2 -> puntero a la segunda señal, la buscada, más pequeña
    size2 -> cantidad de elementos de signal2
    resultado -> size1 - size2 +1
Retorno:
    -1 si size2 >= size1
     0 si los parámetros son correctos
*/
int calcularCorrelacion(const float *signal1, uint16_t size1, const float *signal2, uint16_t size2, float *resultado);
    
#endif

/* [] END OF FILE */
