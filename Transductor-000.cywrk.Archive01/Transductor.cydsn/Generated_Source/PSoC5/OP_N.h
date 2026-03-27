/*******************************************************************************
* File Name: OP_N.h  
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

#if !defined(CY_PINS_OP_N_H) /* Pins OP_N_H */
#define CY_PINS_OP_N_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "OP_N_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 OP_N__PORT == 15 && ((OP_N__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    OP_N_Write(uint8 value);
void    OP_N_SetDriveMode(uint8 mode);
uint8   OP_N_ReadDataReg(void);
uint8   OP_N_Read(void);
void    OP_N_SetInterruptMode(uint16 position, uint16 mode);
uint8   OP_N_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the OP_N_SetDriveMode() function.
     *  @{
     */
        #define OP_N_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define OP_N_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define OP_N_DM_RES_UP          PIN_DM_RES_UP
        #define OP_N_DM_RES_DWN         PIN_DM_RES_DWN
        #define OP_N_DM_OD_LO           PIN_DM_OD_LO
        #define OP_N_DM_OD_HI           PIN_DM_OD_HI
        #define OP_N_DM_STRONG          PIN_DM_STRONG
        #define OP_N_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define OP_N_MASK               OP_N__MASK
#define OP_N_SHIFT              OP_N__SHIFT
#define OP_N_WIDTH              1u

/* Interrupt constants */
#if defined(OP_N__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in OP_N_SetInterruptMode() function.
     *  @{
     */
        #define OP_N_INTR_NONE      (uint16)(0x0000u)
        #define OP_N_INTR_RISING    (uint16)(0x0001u)
        #define OP_N_INTR_FALLING   (uint16)(0x0002u)
        #define OP_N_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define OP_N_INTR_MASK      (0x01u) 
#endif /* (OP_N__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define OP_N_PS                     (* (reg8 *) OP_N__PS)
/* Data Register */
#define OP_N_DR                     (* (reg8 *) OP_N__DR)
/* Port Number */
#define OP_N_PRT_NUM                (* (reg8 *) OP_N__PRT) 
/* Connect to Analog Globals */                                                  
#define OP_N_AG                     (* (reg8 *) OP_N__AG)                       
/* Analog MUX bux enable */
#define OP_N_AMUX                   (* (reg8 *) OP_N__AMUX) 
/* Bidirectional Enable */                                                        
#define OP_N_BIE                    (* (reg8 *) OP_N__BIE)
/* Bit-mask for Aliased Register Access */
#define OP_N_BIT_MASK               (* (reg8 *) OP_N__BIT_MASK)
/* Bypass Enable */
#define OP_N_BYP                    (* (reg8 *) OP_N__BYP)
/* Port wide control signals */                                                   
#define OP_N_CTL                    (* (reg8 *) OP_N__CTL)
/* Drive Modes */
#define OP_N_DM0                    (* (reg8 *) OP_N__DM0) 
#define OP_N_DM1                    (* (reg8 *) OP_N__DM1)
#define OP_N_DM2                    (* (reg8 *) OP_N__DM2) 
/* Input Buffer Disable Override */
#define OP_N_INP_DIS                (* (reg8 *) OP_N__INP_DIS)
/* LCD Common or Segment Drive */
#define OP_N_LCD_COM_SEG            (* (reg8 *) OP_N__LCD_COM_SEG)
/* Enable Segment LCD */
#define OP_N_LCD_EN                 (* (reg8 *) OP_N__LCD_EN)
/* Slew Rate Control */
#define OP_N_SLW                    (* (reg8 *) OP_N__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define OP_N_PRTDSI__CAPS_SEL       (* (reg8 *) OP_N__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define OP_N_PRTDSI__DBL_SYNC_IN    (* (reg8 *) OP_N__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define OP_N_PRTDSI__OE_SEL0        (* (reg8 *) OP_N__PRTDSI__OE_SEL0) 
#define OP_N_PRTDSI__OE_SEL1        (* (reg8 *) OP_N__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define OP_N_PRTDSI__OUT_SEL0       (* (reg8 *) OP_N__PRTDSI__OUT_SEL0) 
#define OP_N_PRTDSI__OUT_SEL1       (* (reg8 *) OP_N__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define OP_N_PRTDSI__SYNC_OUT       (* (reg8 *) OP_N__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(OP_N__SIO_CFG)
    #define OP_N_SIO_HYST_EN        (* (reg8 *) OP_N__SIO_HYST_EN)
    #define OP_N_SIO_REG_HIFREQ     (* (reg8 *) OP_N__SIO_REG_HIFREQ)
    #define OP_N_SIO_CFG            (* (reg8 *) OP_N__SIO_CFG)
    #define OP_N_SIO_DIFF           (* (reg8 *) OP_N__SIO_DIFF)
#endif /* (OP_N__SIO_CFG) */

/* Interrupt Registers */
#if defined(OP_N__INTSTAT)
    #define OP_N_INTSTAT            (* (reg8 *) OP_N__INTSTAT)
    #define OP_N_SNAP               (* (reg8 *) OP_N__SNAP)
    
	#define OP_N_0_INTTYPE_REG 		(* (reg8 *) OP_N__0__INTTYPE)
#endif /* (OP_N__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_OP_N_H */


/* [] END OF FILE */
