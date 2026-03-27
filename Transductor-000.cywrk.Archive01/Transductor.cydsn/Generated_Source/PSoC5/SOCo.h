/*******************************************************************************
* File Name: SOCo.h  
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

#if !defined(CY_PINS_SOCo_H) /* Pins SOCo_H */
#define CY_PINS_SOCo_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "SOCo_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 SOCo__PORT == 15 && ((SOCo__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    SOCo_Write(uint8 value);
void    SOCo_SetDriveMode(uint8 mode);
uint8   SOCo_ReadDataReg(void);
uint8   SOCo_Read(void);
void    SOCo_SetInterruptMode(uint16 position, uint16 mode);
uint8   SOCo_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the SOCo_SetDriveMode() function.
     *  @{
     */
        #define SOCo_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define SOCo_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define SOCo_DM_RES_UP          PIN_DM_RES_UP
        #define SOCo_DM_RES_DWN         PIN_DM_RES_DWN
        #define SOCo_DM_OD_LO           PIN_DM_OD_LO
        #define SOCo_DM_OD_HI           PIN_DM_OD_HI
        #define SOCo_DM_STRONG          PIN_DM_STRONG
        #define SOCo_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define SOCo_MASK               SOCo__MASK
#define SOCo_SHIFT              SOCo__SHIFT
#define SOCo_WIDTH              1u

/* Interrupt constants */
#if defined(SOCo__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in SOCo_SetInterruptMode() function.
     *  @{
     */
        #define SOCo_INTR_NONE      (uint16)(0x0000u)
        #define SOCo_INTR_RISING    (uint16)(0x0001u)
        #define SOCo_INTR_FALLING   (uint16)(0x0002u)
        #define SOCo_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define SOCo_INTR_MASK      (0x01u) 
#endif /* (SOCo__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define SOCo_PS                     (* (reg8 *) SOCo__PS)
/* Data Register */
#define SOCo_DR                     (* (reg8 *) SOCo__DR)
/* Port Number */
#define SOCo_PRT_NUM                (* (reg8 *) SOCo__PRT) 
/* Connect to Analog Globals */                                                  
#define SOCo_AG                     (* (reg8 *) SOCo__AG)                       
/* Analog MUX bux enable */
#define SOCo_AMUX                   (* (reg8 *) SOCo__AMUX) 
/* Bidirectional Enable */                                                        
#define SOCo_BIE                    (* (reg8 *) SOCo__BIE)
/* Bit-mask for Aliased Register Access */
#define SOCo_BIT_MASK               (* (reg8 *) SOCo__BIT_MASK)
/* Bypass Enable */
#define SOCo_BYP                    (* (reg8 *) SOCo__BYP)
/* Port wide control signals */                                                   
#define SOCo_CTL                    (* (reg8 *) SOCo__CTL)
/* Drive Modes */
#define SOCo_DM0                    (* (reg8 *) SOCo__DM0) 
#define SOCo_DM1                    (* (reg8 *) SOCo__DM1)
#define SOCo_DM2                    (* (reg8 *) SOCo__DM2) 
/* Input Buffer Disable Override */
#define SOCo_INP_DIS                (* (reg8 *) SOCo__INP_DIS)
/* LCD Common or Segment Drive */
#define SOCo_LCD_COM_SEG            (* (reg8 *) SOCo__LCD_COM_SEG)
/* Enable Segment LCD */
#define SOCo_LCD_EN                 (* (reg8 *) SOCo__LCD_EN)
/* Slew Rate Control */
#define SOCo_SLW                    (* (reg8 *) SOCo__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define SOCo_PRTDSI__CAPS_SEL       (* (reg8 *) SOCo__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define SOCo_PRTDSI__DBL_SYNC_IN    (* (reg8 *) SOCo__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define SOCo_PRTDSI__OE_SEL0        (* (reg8 *) SOCo__PRTDSI__OE_SEL0) 
#define SOCo_PRTDSI__OE_SEL1        (* (reg8 *) SOCo__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define SOCo_PRTDSI__OUT_SEL0       (* (reg8 *) SOCo__PRTDSI__OUT_SEL0) 
#define SOCo_PRTDSI__OUT_SEL1       (* (reg8 *) SOCo__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define SOCo_PRTDSI__SYNC_OUT       (* (reg8 *) SOCo__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(SOCo__SIO_CFG)
    #define SOCo_SIO_HYST_EN        (* (reg8 *) SOCo__SIO_HYST_EN)
    #define SOCo_SIO_REG_HIFREQ     (* (reg8 *) SOCo__SIO_REG_HIFREQ)
    #define SOCo_SIO_CFG            (* (reg8 *) SOCo__SIO_CFG)
    #define SOCo_SIO_DIFF           (* (reg8 *) SOCo__SIO_DIFF)
#endif /* (SOCo__SIO_CFG) */

/* Interrupt Registers */
#if defined(SOCo__INTSTAT)
    #define SOCo_INTSTAT            (* (reg8 *) SOCo__INTSTAT)
    #define SOCo_SNAP               (* (reg8 *) SOCo__SNAP)
    
	#define SOCo_0_INTTYPE_REG 		(* (reg8 *) SOCo__0__INTTYPE)
#endif /* (SOCo__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_SOCo_H */


/* [] END OF FILE */
