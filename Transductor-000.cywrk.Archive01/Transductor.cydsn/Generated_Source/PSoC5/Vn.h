/*******************************************************************************
* File Name: Vn.h  
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

#if !defined(CY_PINS_Vn_H) /* Pins Vn_H */
#define CY_PINS_Vn_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Vn_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Vn__PORT == 15 && ((Vn__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Vn_Write(uint8 value);
void    Vn_SetDriveMode(uint8 mode);
uint8   Vn_ReadDataReg(void);
uint8   Vn_Read(void);
void    Vn_SetInterruptMode(uint16 position, uint16 mode);
uint8   Vn_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Vn_SetDriveMode() function.
     *  @{
     */
        #define Vn_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Vn_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Vn_DM_RES_UP          PIN_DM_RES_UP
        #define Vn_DM_RES_DWN         PIN_DM_RES_DWN
        #define Vn_DM_OD_LO           PIN_DM_OD_LO
        #define Vn_DM_OD_HI           PIN_DM_OD_HI
        #define Vn_DM_STRONG          PIN_DM_STRONG
        #define Vn_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Vn_MASK               Vn__MASK
#define Vn_SHIFT              Vn__SHIFT
#define Vn_WIDTH              1u

/* Interrupt constants */
#if defined(Vn__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Vn_SetInterruptMode() function.
     *  @{
     */
        #define Vn_INTR_NONE      (uint16)(0x0000u)
        #define Vn_INTR_RISING    (uint16)(0x0001u)
        #define Vn_INTR_FALLING   (uint16)(0x0002u)
        #define Vn_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Vn_INTR_MASK      (0x01u) 
#endif /* (Vn__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Vn_PS                     (* (reg8 *) Vn__PS)
/* Data Register */
#define Vn_DR                     (* (reg8 *) Vn__DR)
/* Port Number */
#define Vn_PRT_NUM                (* (reg8 *) Vn__PRT) 
/* Connect to Analog Globals */                                                  
#define Vn_AG                     (* (reg8 *) Vn__AG)                       
/* Analog MUX bux enable */
#define Vn_AMUX                   (* (reg8 *) Vn__AMUX) 
/* Bidirectional Enable */                                                        
#define Vn_BIE                    (* (reg8 *) Vn__BIE)
/* Bit-mask for Aliased Register Access */
#define Vn_BIT_MASK               (* (reg8 *) Vn__BIT_MASK)
/* Bypass Enable */
#define Vn_BYP                    (* (reg8 *) Vn__BYP)
/* Port wide control signals */                                                   
#define Vn_CTL                    (* (reg8 *) Vn__CTL)
/* Drive Modes */
#define Vn_DM0                    (* (reg8 *) Vn__DM0) 
#define Vn_DM1                    (* (reg8 *) Vn__DM1)
#define Vn_DM2                    (* (reg8 *) Vn__DM2) 
/* Input Buffer Disable Override */
#define Vn_INP_DIS                (* (reg8 *) Vn__INP_DIS)
/* LCD Common or Segment Drive */
#define Vn_LCD_COM_SEG            (* (reg8 *) Vn__LCD_COM_SEG)
/* Enable Segment LCD */
#define Vn_LCD_EN                 (* (reg8 *) Vn__LCD_EN)
/* Slew Rate Control */
#define Vn_SLW                    (* (reg8 *) Vn__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Vn_PRTDSI__CAPS_SEL       (* (reg8 *) Vn__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Vn_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Vn__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Vn_PRTDSI__OE_SEL0        (* (reg8 *) Vn__PRTDSI__OE_SEL0) 
#define Vn_PRTDSI__OE_SEL1        (* (reg8 *) Vn__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Vn_PRTDSI__OUT_SEL0       (* (reg8 *) Vn__PRTDSI__OUT_SEL0) 
#define Vn_PRTDSI__OUT_SEL1       (* (reg8 *) Vn__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Vn_PRTDSI__SYNC_OUT       (* (reg8 *) Vn__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Vn__SIO_CFG)
    #define Vn_SIO_HYST_EN        (* (reg8 *) Vn__SIO_HYST_EN)
    #define Vn_SIO_REG_HIFREQ     (* (reg8 *) Vn__SIO_REG_HIFREQ)
    #define Vn_SIO_CFG            (* (reg8 *) Vn__SIO_CFG)
    #define Vn_SIO_DIFF           (* (reg8 *) Vn__SIO_DIFF)
#endif /* (Vn__SIO_CFG) */

/* Interrupt Registers */
#if defined(Vn__INTSTAT)
    #define Vn_INTSTAT            (* (reg8 *) Vn__INTSTAT)
    #define Vn_SNAP               (* (reg8 *) Vn__SNAP)
    
	#define Vn_0_INTTYPE_REG 		(* (reg8 *) Vn__0__INTTYPE)
#endif /* (Vn__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Vn_H */


/* [] END OF FILE */
