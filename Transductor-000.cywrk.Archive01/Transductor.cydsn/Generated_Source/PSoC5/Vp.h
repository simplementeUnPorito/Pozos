/*******************************************************************************
* File Name: Vp.h  
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

#if !defined(CY_PINS_Vp_H) /* Pins Vp_H */
#define CY_PINS_Vp_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Vp_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Vp__PORT == 15 && ((Vp__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Vp_Write(uint8 value);
void    Vp_SetDriveMode(uint8 mode);
uint8   Vp_ReadDataReg(void);
uint8   Vp_Read(void);
void    Vp_SetInterruptMode(uint16 position, uint16 mode);
uint8   Vp_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Vp_SetDriveMode() function.
     *  @{
     */
        #define Vp_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Vp_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Vp_DM_RES_UP          PIN_DM_RES_UP
        #define Vp_DM_RES_DWN         PIN_DM_RES_DWN
        #define Vp_DM_OD_LO           PIN_DM_OD_LO
        #define Vp_DM_OD_HI           PIN_DM_OD_HI
        #define Vp_DM_STRONG          PIN_DM_STRONG
        #define Vp_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Vp_MASK               Vp__MASK
#define Vp_SHIFT              Vp__SHIFT
#define Vp_WIDTH              1u

/* Interrupt constants */
#if defined(Vp__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Vp_SetInterruptMode() function.
     *  @{
     */
        #define Vp_INTR_NONE      (uint16)(0x0000u)
        #define Vp_INTR_RISING    (uint16)(0x0001u)
        #define Vp_INTR_FALLING   (uint16)(0x0002u)
        #define Vp_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Vp_INTR_MASK      (0x01u) 
#endif /* (Vp__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Vp_PS                     (* (reg8 *) Vp__PS)
/* Data Register */
#define Vp_DR                     (* (reg8 *) Vp__DR)
/* Port Number */
#define Vp_PRT_NUM                (* (reg8 *) Vp__PRT) 
/* Connect to Analog Globals */                                                  
#define Vp_AG                     (* (reg8 *) Vp__AG)                       
/* Analog MUX bux enable */
#define Vp_AMUX                   (* (reg8 *) Vp__AMUX) 
/* Bidirectional Enable */                                                        
#define Vp_BIE                    (* (reg8 *) Vp__BIE)
/* Bit-mask for Aliased Register Access */
#define Vp_BIT_MASK               (* (reg8 *) Vp__BIT_MASK)
/* Bypass Enable */
#define Vp_BYP                    (* (reg8 *) Vp__BYP)
/* Port wide control signals */                                                   
#define Vp_CTL                    (* (reg8 *) Vp__CTL)
/* Drive Modes */
#define Vp_DM0                    (* (reg8 *) Vp__DM0) 
#define Vp_DM1                    (* (reg8 *) Vp__DM1)
#define Vp_DM2                    (* (reg8 *) Vp__DM2) 
/* Input Buffer Disable Override */
#define Vp_INP_DIS                (* (reg8 *) Vp__INP_DIS)
/* LCD Common or Segment Drive */
#define Vp_LCD_COM_SEG            (* (reg8 *) Vp__LCD_COM_SEG)
/* Enable Segment LCD */
#define Vp_LCD_EN                 (* (reg8 *) Vp__LCD_EN)
/* Slew Rate Control */
#define Vp_SLW                    (* (reg8 *) Vp__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Vp_PRTDSI__CAPS_SEL       (* (reg8 *) Vp__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Vp_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Vp__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Vp_PRTDSI__OE_SEL0        (* (reg8 *) Vp__PRTDSI__OE_SEL0) 
#define Vp_PRTDSI__OE_SEL1        (* (reg8 *) Vp__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Vp_PRTDSI__OUT_SEL0       (* (reg8 *) Vp__PRTDSI__OUT_SEL0) 
#define Vp_PRTDSI__OUT_SEL1       (* (reg8 *) Vp__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Vp_PRTDSI__SYNC_OUT       (* (reg8 *) Vp__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Vp__SIO_CFG)
    #define Vp_SIO_HYST_EN        (* (reg8 *) Vp__SIO_HYST_EN)
    #define Vp_SIO_REG_HIFREQ     (* (reg8 *) Vp__SIO_REG_HIFREQ)
    #define Vp_SIO_CFG            (* (reg8 *) Vp__SIO_CFG)
    #define Vp_SIO_DIFF           (* (reg8 *) Vp__SIO_DIFF)
#endif /* (Vp__SIO_CFG) */

/* Interrupt Registers */
#if defined(Vp__INTSTAT)
    #define Vp_INTSTAT            (* (reg8 *) Vp__INTSTAT)
    #define Vp_SNAP               (* (reg8 *) Vp__SNAP)
    
	#define Vp_0_INTTYPE_REG 		(* (reg8 *) Vp__0__INTTYPE)
#endif /* (Vp__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Vp_H */


/* [] END OF FILE */
