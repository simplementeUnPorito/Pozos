/* ============================================================================
 *   SeroalPlot v0.0 custom component
 *
 * Description:
 *   Implements interface for serial communication with SerialPlot charting software
 *   Uses UART communication to send data types:
         int8, int16, int32, uint8, uint16, uin32, float
 *   in:
 *      -single binary format 
 *      -custom frame (+header, +checksum)
 *      -ASCII
 *
 *
 * Credits:
 *   Serial PLOT v0.10.0
 *   by Yavuz Ozderya
 *   https://bitbucket.org/hyOzd/serialplot   
 *
 * ============================================================================
 * PROVIDED AS-IS, NO WARRANTY OF ANY KIND, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * FREE TO SHARE, USE AND MODIFY UNDER TERMS: CREATIVE COMMONS - SHARE ALIKE
 * ============================================================================
*/


#ifndef Chart_1_H
#define Chart_1_H
 
    
#include <project.h>
#include <cytypes.h>
           
#define true  1
#define false 0

/***************************************
*        read-only parameters
***************************************/  

#define Chart_1_DataFormat      1           // Output data format. simple binary / ASCII / custom frame. 
#define Chart_1_DataType        7           // Output data type: int8(uint8), int16(uint16), int32(uint32) and float.  
#define Chart_1_NumChan         1           // Number of output channels. Valid range [1 to 8].  
#define Chart_1_Checksum        false           // Enable data frame checksum.  
//#define Chart_1_UART            UART_1           // instance name of UART used for communication   
//#define Chart_1_UART_mode       1           // UART mode: Tx, Rx, Tx+Rx   
//#define Chart_1_range           float32           // int8, int16, int32, float32(single)      
//#define Chart_1_FrameHeader     0xAA, 0xBB           // Frame header: 0xAA, 0xBB       
//#define Chart_1_FrameHeaderSize 2       // Frame header size       
//#define Chart_1_ArgsList        float32 V1                  // arguments list (debug..)
//#define Chart_1_ValsList        V1                  // values list (debug..)
//#define Chart_1_FormatStr       "%g\r\n"             // ASCII format string (debug..)
  


    
/***************************************
*        global variables
***************************************/  

    


/***************************************
*        read-only variables
***************************************/  
   
 
    
/***************************************
*        Function Prototypes
***************************************/

void  Chart_1_Plot(float32 V1); // universal overload procedure 
 



    
#endif /* Chart_1_H */


/* [] END OF FILE */
