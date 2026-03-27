/*******************************************************************************
* File Name: V_sensor.h  
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

#if !defined(CY_PINS_V_sensor_H) /* Pins V_sensor_H */
#define CY_PINS_V_sensor_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "V_sensor_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 V_sensor__PORT == 15 && ((V_sensor__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    V_sensor_Write(uint8 value);
void    V_sensor_SetDriveMode(uint8 mode);
uint8   V_sensor_ReadDataReg(void);
uint8   V_sensor_Read(void);
void    V_sensor_SetInterruptMode(uint16 position, uint16 mode);
uint8   V_sensor_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the V_sensor_SetDriveMode() function.
     *  @{
     */
        #define V_sensor_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define V_sensor_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define V_sensor_DM_RES_UP          PIN_DM_RES_UP
        #define V_sensor_DM_RES_DWN         PIN_DM_RES_DWN
        #define V_sensor_DM_OD_LO           PIN_DM_OD_LO
        #define V_sensor_DM_OD_HI           PIN_DM_OD_HI
        #define V_sensor_DM_STRONG          PIN_DM_STRONG
        #define V_sensor_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define V_sensor_MASK               V_sensor__MASK
#define V_sensor_SHIFT              V_sensor__SHIFT
#define V_sensor_WIDTH              1u

/* Interrupt constants */
#if defined(V_sensor__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in V_sensor_SetInterruptMode() function.
     *  @{
     */
        #define V_sensor_INTR_NONE      (uint16)(0x0000u)
        #define V_sensor_INTR_RISING    (uint16)(0x0001u)
        #define V_sensor_INTR_FALLING   (uint16)(0x0002u)
        #define V_sensor_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define V_sensor_INTR_MASK      (0x01u) 
#endif /* (V_sensor__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define V_sensor_PS                     (* (reg8 *) V_sensor__PS)
/* Data Register */
#define V_sensor_DR                     (* (reg8 *) V_sensor__DR)
/* Port Number */
#define V_sensor_PRT_NUM                (* (reg8 *) V_sensor__PRT) 
/* Connect to Analog Globals */                                                  
#define V_sensor_AG                     (* (reg8 *) V_sensor__AG)                       
/* Analog MUX bux enable */
#define V_sensor_AMUX                   (* (reg8 *) V_sensor__AMUX) 
/* Bidirectional Enable */                                                        
#define V_sensor_BIE                    (* (reg8 *) V_sensor__BIE)
/* Bit-mask for Aliased Register Access */
#define V_sensor_BIT_MASK               (* (reg8 *) V_sensor__BIT_MASK)
/* Bypass Enable */
#define V_sensor_BYP                    (* (reg8 *) V_sensor__BYP)
/* Port wide control signals */                                                   
#define V_sensor_CTL                    (* (reg8 *) V_sensor__CTL)
/* Drive Modes */
#define V_sensor_DM0                    (* (reg8 *) V_sensor__DM0) 
#define V_sensor_DM1                    (* (reg8 *) V_sensor__DM1)
#define V_sensor_DM2                    (* (reg8 *) V_sensor__DM2) 
/* Input Buffer Disable Override */
#define V_sensor_INP_DIS                (* (reg8 *) V_sensor__INP_DIS)
/* LCD Common or Segment Drive */
#define V_sensor_LCD_COM_SEG            (* (reg8 *) V_sensor__LCD_COM_SEG)
/* Enable Segment LCD */
#define V_sensor_LCD_EN                 (* (reg8 *) V_sensor__LCD_EN)
/* Slew Rate Control */
#define V_sensor_SLW                    (* (reg8 *) V_sensor__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define V_sensor_PRTDSI__CAPS_SEL       (* (reg8 *) V_sensor__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define V_sensor_PRTDSI__DBL_SYNC_IN    (* (reg8 *) V_sensor__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define V_sensor_PRTDSI__OE_SEL0        (* (reg8 *) V_sensor__PRTDSI__OE_SEL0) 
#define V_sensor_PRTDSI__OE_SEL1        (* (reg8 *) V_sensor__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define V_sensor_PRTDSI__OUT_SEL0       (* (reg8 *) V_sensor__PRTDSI__OUT_SEL0) 
#define V_sensor_PRTDSI__OUT_SEL1       (* (reg8 *) V_sensor__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define V_sensor_PRTDSI__SYNC_OUT       (* (reg8 *) V_sensor__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(V_sensor__SIO_CFG)
    #define V_sensor_SIO_HYST_EN        (* (reg8 *) V_sensor__SIO_HYST_EN)
    #define V_sensor_SIO_REG_HIFREQ     (* (reg8 *) V_sensor__SIO_REG_HIFREQ)
    #define V_sensor_SIO_CFG            (* (reg8 *) V_sensor__SIO_CFG)
    #define V_sensor_SIO_DIFF           (* (reg8 *) V_sensor__SIO_DIFF)
#endif /* (V_sensor__SIO_CFG) */

/* Interrupt Registers */
#if defined(V_sensor__INTSTAT)
    #define V_sensor_INTSTAT            (* (reg8 *) V_sensor__INTSTAT)
    #define V_sensor_SNAP               (* (reg8 *) V_sensor__SNAP)
    
	#define V_sensor_0_INTTYPE_REG 		(* (reg8 *) V_sensor__0__INTTYPE)
#endif /* (V_sensor__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_V_sensor_H */


/* [] END OF FILE */
