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
uint8 flag = 'W';
uint j;


/*Prototipo de las funciones utilizadas*/
CY_ISR(Fin_Adq);
CY_ISR(RxIsr);

void my_Start(void);
void DMA_Config(void);
static void putFloat(float val);


int main(void)
{
    my_Start();
    DMA_Config();
    
     //Punteros a la rutinas de interrupción  
    #if(INTERRUPT_CODE_ENABLED == ENABLED)
    isr_Fin_Adq_StartEx(Fin_Adq);    
    //isr_rx_StartEx(RxIsr);
    #endif /* INTERRUPT_CODE_ENABLED == ENABLED */
    CyDelay(50);  
    CyGlobalIntEnable; /* Enable global interrupts. */
    //isr_rx_Enable();
    //Se deshabilita la adquisicion
    Reg_Hab_Adq_Write(1u);
    isr_Fin_Adq_Disable();    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    
    for(;;)
    {
        switch (flag)
        {
        case 'W':
            //Espera trigger: botón hardware o comando 'T' desde MATLAB por UART
            if (UART_1_GetRxBufferSize() > 0) {
                if (UART_1_GetByte() == 'T') {
                    flag = '0';
                }
            }
            break;

        case '0':
            //printf("%c\r\n",flag); 
            //Se espera el flanco que conmuta el flip flop
            Reg_Hab_Adq_Write(0u);
            isr_Fin_Adq_Enable();
            //ADC_DelSig_1_Start();
            flag = '1';
            break;
        
        case '1':
            //Se espera la finalizacion de la adquisicion                
            break;
            
        case '2':
            //Finalizo la adquisicion y se transmiten los datos
            Reg_Hab_Adq_Write(1u);
            isr_Fin_Adq_Disable();

            //Se envian los datos a MATLAB como floats ASCII (protocolo S / valores / E)
            UART_1_PutString("S\r\n");
            for(j=0; j<LONGITUD; j++)
            {
                y[j] = ADC_SAR_1_CountsTo_Volts(datos[j]);
                putFloat(y[j]);
            }
            UART_1_PutString("E\r\n");

            //Se reinicia la maquina de estados.
            flag = 'W';

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
    //Opamp_2_Start();
    
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
//	Filter_1_SetCoherency(Filter_1_STAGEA_COHER,Filter_1_KEY_MID );
//	Filter_1_SetCoherency(Filter_1_STAGEB_COHER,Filter_1_KEY_MID ); 
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




/* Envía un float por UART como decimal ASCII sin usar %f en sprintf.
 * Funciona aunque newlib-nano no incluya soporte de printf/float. */
static void putFloat(float val)
{
    char buf[20];
    int32 int_part;
    uint32 dec_part;

    if (val < 0.0f) {
        UART_1_PutChar('-');
        val = -val;
    }
    int_part = (int32)val;
    dec_part = (uint32)((val - (float)int_part) * 100000.0f + 0.5f);
    if (dec_part >= 100000u) { int_part++; dec_part = 0u; }
    sprintf(buf, "%ld.%05lu\r\n", (long)int_part, (unsigned long)dec_part);
    UART_1_PutString(buf);
}

/* [] END OF FILE */
