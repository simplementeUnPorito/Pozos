/*******************************************************************************
* File Name: Sal_DAC.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Sal_DAC_H) /* Pins Sal_DAC_H */
#define CY_PINS_Sal_DAC_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Sal_DAC_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Sal_DAC__PORT == 15 && ((Sal_DAC__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Sal_DAC_Write(uint8 value);
void    Sal_DAC_SetDriveMode(uint8 mode);
uint8   Sal_DAC_ReadDataReg(void);
uint8   Sal_DAC_Read(void);
void    Sal_DAC_SetInterruptMode(uint16 position, uint16 mode);
uint8   Sal_DAC_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Sal_DAC_SetDriveMode() function.
     *  @{
     */
        #define Sal_DAC_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Sal_DAC_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Sal_DAC_DM_RES_UP          PIN_DM_RES_UP
        #define Sal_DAC_DM_RES_DWN         PIN_DM_RES_DWN
        #define Sal_DAC_DM_OD_LO           PIN_DM_OD_LO
        #define Sal_DAC_DM_OD_HI           PIN_DM_OD_HI
        #define Sal_DAC_DM_STRONG          PIN_DM_STRONG
        #define Sal_DAC_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Sal_DAC_MASK               Sal_DAC__MASK
#define Sal_DAC_SHIFT              Sal_DAC__SHIFT
#define Sal_DAC_WIDTH              1u

/* Interrupt constants */
#if defined(Sal_DAC__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Sal_DAC_SetInterruptMode() function.
     *  @{
     */
        #define Sal_DAC_INTR_NONE      (uint16)(0x0000u)
        #define Sal_DAC_INTR_RISING    (uint16)(0x0001u)
        #define Sal_DAC_INTR_FALLING   (uint16)(0x0002u)
        #define Sal_DAC_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Sal_DAC_INTR_MASK      (0x01u) 
#endif /* (Sal_DAC__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Sal_DAC_PS                     (* (reg8 *) Sal_DAC__PS)
/* Data Register */
#define Sal_DAC_DR                     (* (reg8 *) Sal_DAC__DR)
/* Port Number */
#define Sal_DAC_PRT_NUM                (* (reg8 *) Sal_DAC__PRT) 
/* Connect to Analog Globals */                                                  
#define Sal_DAC_AG                     (* (reg8 *) Sal_DAC__AG)                       
/* Analog MUX bux enable */
#define Sal_DAC_AMUX                   (* (reg8 *) Sal_DAC__AMUX) 
/* Bidirectional Enable */                                                        
#define Sal_DAC_BIE                    (* (reg8 *) Sal_DAC__BIE)
/* Bit-mask for Aliased Register Access */
#define Sal_DAC_BIT_MASK               (* (reg8 *) Sal_DAC__BIT_MASK)
/* Bypass Enable */
#define Sal_DAC_BYP                    (* (reg8 *) Sal_DAC__BYP)
/* Port wide control signals */                                                   
#define Sal_DAC_CTL                    (* (reg8 *) Sal_DAC__CTL)
/* Drive Modes */
#define Sal_DAC_DM0                    (* (reg8 *) Sal_DAC__DM0) 
#define Sal_DAC_DM1                    (* (reg8 *) Sal_DAC__DM1)
#define Sal_DAC_DM2                    (* (reg8 *) Sal_DAC__DM2) 
/* Input Buffer Disable Override */
#define Sal_DAC_INP_DIS                (* (reg8 *) Sal_DAC__INP_DIS)
/* LCD Common or Segment Drive */
#define Sal_DAC_LCD_COM_SEG            (* (reg8 *) Sal_DAC__LCD_COM_SEG)
/* Enable Segment LCD */
#define Sal_DAC_LCD_EN                 (* (reg8 *) Sal_DAC__LCD_EN)
/* Slew Rate Control */
#define Sal_DAC_SLW                    (* (reg8 *) Sal_DAC__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Sal_DAC_PRTDSI__CAPS_SEL       (* (reg8 *) Sal_DAC__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Sal_DAC_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Sal_DAC__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Sal_DAC_PRTDSI__OE_SEL0        (* (reg8 *) Sal_DAC__PRTDSI__OE_SEL0) 
#define Sal_DAC_PRTDSI__OE_SEL1        (* (reg8 *) Sal_DAC__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Sal_DAC_PRTDSI__OUT_SEL0       (* (reg8 *) Sal_DAC__PRTDSI__OUT_SEL0) 
#define Sal_DAC_PRTDSI__OUT_SEL1       (* (reg8 *) Sal_DAC__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Sal_DAC_PRTDSI__SYNC_OUT       (* (reg8 *) Sal_DAC__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Sal_DAC__SIO_CFG)
    #define Sal_DAC_SIO_HYST_EN        (* (reg8 *) Sal_DAC__SIO_HYST_EN)
    #define Sal_DAC_SIO_REG_HIFREQ     (* (reg8 *) Sal_DAC__SIO_REG_HIFREQ)
    #define Sal_DAC_SIO_CFG            (* (reg8 *) Sal_DAC__SIO_CFG)
    #define Sal_DAC_SIO_DIFF           (* (reg8 *) Sal_DAC__SIO_DIFF)
#endif /* (Sal_DAC__SIO_CFG) */

/* Interrupt Registers */
#if defined(Sal_DAC__INTSTAT)
    #define Sal_DAC_INTSTAT            (* (reg8 *) Sal_DAC__INTSTAT)
    #define Sal_DAC_SNAP               (* (reg8 *) Sal_DAC__SNAP)
    
	#define Sal_DAC_0_INTTYPE_REG 		(* (reg8 *) Sal_DAC__0__INTTYPE)
#endif /* (Sal_DAC__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Sal_DAC_H */


/* [] END OF FILE */
