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
#include <stdio.h>
#include "er.h"



#define LONGITUD 2000
#define FILTER_1_DATA_ALIGN       (0x55u)


/* Defines for DMA_1 */
#define DMA_1_BYTES_PER_BURST 2
#define DMA_1_REQUEST_PER_BURST 1
#define DMA_1_SRC_BASE (CYDEV_PERIPH_BASE)
#define DMA_1_DST_BASE (CYDEV_SRAM_BASE)

/* Defines for DMA_2 */
#define DMA_2_BYTES_PER_BURST 2
#define DMA_2_REQUEST_PER_BURST 1
#define DMA_2_SRC_BASE (CYDEV_PERIPH_BASE)
#define DMA_2_DST_BASE (CYDEV_PERIPH_BASE)

/* Variable declarations for DMA_1 */
/* Move these variable declarations to the top of the function */
uint8 DMA_1_Chan;
uint8 DMA_1_TD[1];

/* Variable declarations for DMA_2 */
/* Move these variable declarations to the top of the function */
uint8 DMA_2_Chan;
uint8 DMA_2_TD[1];


/*Declaraciones de las variables utilizadas */

int16 datos[LONGITUD];
float y[LONGITUD];
uint8 flag = '0';
uint j;


/*Prototipo de las funciones utilizadas*/
CY_ISR_PROTO(Fin_Adq);
CY_ISR_PROTO(RxIsr);

void my_Start(void);
void DMA_Config(void);
void Check_UART_Trigger(void);

uint8 bandera = 0;
int main(void)
{
    my_Start();
    DMA_Config();
    
     //Punteros a la rutinas de interrupción  
    #if(INTERRUPT_CODE_ENABLED == ENABLED)
    isr_Fin_Adq_StartEx(Fin_Adq);
    isr_rx_StartEx(RxIsr);
    #endif /* INTERRUPT_CODE_ENABLED == ENABLED */
    
    
    CyDelay(50);  
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Inicialización de registros */
    Reg_Hab_Adq_Write(1u);
    Reg_Received_Write(0u);

    isr_Fin_Adq_Disable();
    isr_rx_Disable();    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    
    for(;;)
    {


        switch (flag)
        {
        case '0':
            //printf("%c\r\n",flag); 
            //Se espera el flanco que conmuta el flip flop
            Reg_Hab_Adq_Write(0u);
            Reg_Received_Write(0u);
            isr_Fin_Adq_Enable();
            isr_rx_Enable();    
            //ADC_DelSig_1_Start();
            flag = '1';
            break;
        
        case '1':
            //Se espera la finalizacion de la adquisicion
            break;
            
        case '2':
            //Finalizo la adquisicion y se transmiten los datos
            Reg_Hab_Adq_Write(1u);
            Reg_Received_Write(0u);   /* cerrar ciclo del trigger UART */
            isr_Fin_Adq_Disable();
            isr_rx_Disable();    
            //Se envia la señal digitalizada por el puerto serie               
            for(j=0; j<(sizeof(y)/sizeof(y[0])); j++)
            { 
                y[j] = ADC_SAR_1_CountsTo_Volts(datos[j]); 
                //y[j] = ADC_DelSig_1_CountsTo_Volts(datos[j]);
                Chart_1_Plot(y[j]);
            }
            //envBytesChecksum((uint8_t*)datos, LONGITUD*2,UART_1_PutChar);
            
            //Se reinicia la maquina de estados. 
            flag = '0';
            
            break;           
        }         
        
    }
}   


void my_Start()
{
//Se deben instanciar los componentes
//Los analógicos
    TIA_1_Start();
    TIA_2_Start();
    ADC_SAR_1_Start();
    //ADC_DelSig_1_Start();
    Opamp_1_Start();
    Opamp_2_Start();
    
//     Filter_1_Start();
//    //Para que el filtro pueda manejar 
//    //una resolución de 9-16 bits, el registro de coherencia debe ser mid, Dalign debe estar habilitado, 
//    Filter_1_SetDalign(Filter_1_STAGEA_DALIGN, Filter_1_ENABLED);
//    Filter_1_SetDalign(Filter_1_HOLDA_DALIGN, Filter_1_ENABLED);
//    Filter_1_SetDalign(Filter_1_STAGEB_DALIGN, Filter_1_ENABLED);
//    Filter_1_SetDalign(Filter_1_HOLDB_DALIGN, Filter_1_ENABLED);
//    Filter_1_DALIGN_REG =  Filter_1_DALIGN_REG | FILTER_1_DATA_ALIGN;
//     
//    Filter_1_SetCoherency(Filter_1_CHANNEL_A,Filter_1_KEY_MID );
//    Filter_1_SetCoherency(Filter_1_CHANNEL_B,Filter_1_KEY_MID );
//    Filter_1_SetCoherency(Filter_1_STAGEA_COHER,Filter_1_KEY_MID );
//    Filter_1_SetCoherency(Filter_1_STAGEB_COHER,Filter_1_KEY_MID ); 
//    Filter_1_SetCoherency(Filter_1_HOLDA_COHER,Filter_1_KEY_MID ); 
//    Filter_1_SetCoherency(Filter_1_HOLDB_COHER,Filter_1_KEY_MID );     
    
 //Los digitales
    Clock_ADC_Start();
    Clock_Trig_Start();
    UART_1_Start();
}



//Rutinas de interrupción
CY_ISR(Fin_Adq){   
    //Se inicia el convertidor
    flag = '2';
    //ADC_DelSig_1_Stop();
    isr_Fin_Adq_ClearPending();  
}


CY_ISR(RxIsr)
{
    uint8 dato_rx;
    isr_rx_ClearPending();
    while(UART_1_GetRxBufferSize() > 0u)
    {
        dato_rx = UART_1_GetChar();

        if(dato_rx == 'T')
        {
            Reg_Received_Write(1u);
        }
    }
}



void DMA_Config(){

///* DMA Configuration for DMA_1*/
DMA_1_Chan = DMA_1_DmaInitialize(DMA_1_BYTES_PER_BURST, DMA_1_REQUEST_PER_BURST, 
    HI16(DMA_1_SRC_BASE), HI16(DMA_1_DST_BASE));
DMA_1_TD[0] = CyDmaTdAllocate();
CyDmaTdSetConfiguration(DMA_1_TD[0], 2*LONGITUD, DMA_1_TD[0], DMA_1__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
CyDmaTdSetAddress(DMA_1_TD[0], LO16((uint32)ADC_SAR_1_SAR_WRK0_PTR), LO16((uint32)datos));
//CyDmaTdSetAddress(DMA_1_TD[0], LO16((uint32)ADC_DelSig_1_DEC_SAMP_PTR), LO16((uint32)datos));
CyDmaChSetInitialTd(DMA_1_Chan, DMA_1_TD[0]);
CyDmaChEnable(DMA_1_Chan, 1);


/* DMA Configuration for DMA_1 */
//DMA_1_Chan = DMA_1_DmaInitialize(DMA_1_BYTES_PER_BURST, DMA_1_REQUEST_PER_BURST, 
//    HI16(DMA_1_SRC_BASE), HI16(DMA_1_DST_BASE));
//DMA_1_TD[0] = CyDmaTdAllocate();
////CyDmaTdSetConfiguration(DMA_1_TD[0], 2*LONGITUD, DMA_1_TD[0], DMA_1__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
//CyDmaTdSetConfiguration(DMA_1_TD[0], 2*LONGITUD, DMA_1_TD[0], DMA_1__TD_TERMOUT_EN | CY_DMA_TD_INC_DST_ADR);
//CyDmaTdSetAddress(DMA_1_TD[0], LO16((uint32)Filter_1_HOLDA_PTR), LO16((uint32)datos));
//CyDmaChSetInitialTd(DMA_1_Chan, DMA_1_TD[0]);
//CyDmaChEnable(DMA_1_Chan, 1);

/* DMA Configuration for DMA_2*/
//DMA_2_Chan = DMA_2_DmaInitialize(DMA_2_BYTES_PER_BURST, DMA_2_REQUEST_PER_BURST, HI16(DMA_2_SRC_BASE), HI16(DMA_2_DST_BASE));
//DMA_2_TD[0] = CyDmaTdAllocate();
//CyDmaTdSetConfiguration(DMA_2_TD[0], 2, DMA_2_TD[0], 0);
//CyDmaTdSetAddress(DMA_2_TD[0], LO16((uint32)ADC_SAR_1_SAR_WRK0_PTR), LO16((uint32)Filter_1_STAGEA_PTR));
//CyDmaChSetInitialTd(DMA_2_Chan, DMA_2_TD[0]);
//CyDmaChEnable(DMA_2_Chan, 1);
}




/* [] END OF FILE */