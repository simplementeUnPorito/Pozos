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

#include <Chart_1.h> // must specify API prefix in Symbol->Properties->Doc.APIPrefix
#include <stdio.h>      // sformat (ASCII mode only)



//====================================
//        private definitions
//====================================
#define Chart_1_DataPacketSize   4  // size of sinle data packet (slice)  

#define _SimpleBinary  (0x00u)
#define _ASCII         (0x01u)
#define _CustomFrame   (0x02u)
    

//====================================
//        private variables
//====================================

  
//====================================
//        private functions
//====================================





#if ((Chart_1_DataFormat==_SimpleBinary) & (Chart_1_DataPacketSize==1) )
//==============================================================================
// Unique function to send data in Simple Binary format, int8 (uint8), N=1       
// this is simplest and fastest format, which does not need
// neither header nor checksum to transmit the data
//==============================================================================
void Chart_1_Plot(float32 V1)
{
    // Format:      Simple Binary
    // Channels:    1
    // Number Type: float32
    // Endianness:  Little Endian

    UART_1_PutChar(V1);    // unique 8-bit case
}
#endif





#if ((Chart_1_DataFormat==_SimpleBinary) & (Chart_1_DataPacketSize!=1) )
//==============================================================================
// Universal parametrized function to send data in Single Binary Frame format
// excluding unique combination of data packet size=1: int8 (uint8), N=1
//    
// This mode of operation does not guarantee coerrect data transmission
// all received data may be corrupted due to a loss of a single byte    
//    
// works for NumChan = 1 to 8
// Args - arguments list: "int8 V1, int8 V2,.." 
// Vals - values list   : "V1, V2,.."
//==============================================================================
void Chart_1_Plot(float32 V1) 
{   
    // Format:      Simple Binary
    // Channels:    1
    // Number Type: float32
    // Endianness:  Little Endian

    float32 val[1] = {V1};  
 
    UART_1_PutArray((uint8 *) &val, sizeof(val)); // send data as array of char
}
#endif




#if ((Chart_1_DataFormat==_CustomFrame) & (!Chart_1_Checksum))
//==============================================================================
// Universal parametrized function to send data in Custom Frame format
// works for NumChan = 1 to 8
// Args - arguments list: "int8 V1, int8 V2,.." 
// Vals - values list   : "V1, V2,.."
//==============================================================================
void Chart_1_Plot(float32 V1) 
{
    // todo: use customizer to extract number of bytes in the header?
    
    // Format:      Custom Frame
    // Frame Start: 0xAA, 0xBB
    // Channels:    1
    // Frame Size:  Fixed, Size=4
    // Number Type: float32
    // Endianness:  Little Endian
    // Checksum:    false

    struct {
        uint8    head[2];
        float32  val[1];
    } __attribute__ ((packed)) Frame  = { {0xAA, 0xBB}, {V1} };  

    
    UART_1_PutArray((uint8 *) &Frame, sizeof(Frame)); // send data Frame as array of char
}





#elif ((Chart_1_DataFormat==_CustomFrame) & (Chart_1_Checksum))
//==============================================================================
// Universal parametrized function to send data in Custom Frame format w/Checksum
// works for NumChan = 1 to 8
// Args - arguments list: "int8 V1, int8 V2,.." 
// Vals - values list   : "V1, V2,.."
//==============================================================================
void Chart_1_Plot(float32 V1) 
{ 
    // Format:      Custom Frame
    // Frame Start: 0xAA, 0xBB
    // Channels:    1
    // Frame Size:  Fixed, Size=4 
    // Number Type: float32
    // Endianness:  Little Endian
    // Checksum:    false

    struct {
        uint8    head[2];     
        float32  val[1];       
        uint8   CS; // checksum
    } __attribute__ ((packed)) Frame  = { {0xAA, 0xBB}, {V1}, 0};  

    
    // get sum of all bytes in val[] array
    uint8 * pVal = (uint8 *) &Frame.val[0];             // pointer to val[] start
    uint8 * pCS  = (uint8 *) &Frame.CS;                 // pointer to val[] stop
    while (pVal < pCS) { * pCS += * pVal++; }           // add val[] bytes to checksum
    
    
    UART_1_PutArray((uint8 *) &Frame, sizeof(Frame)); // send data Frame as array of char
}





#elif   (Chart_1_DataFormat==_ASCII)
//==============================================================================
// Universal parametrized function to send data in ASCII format, N = 1..8 
// (floats need newlib nano + heapsize 0x200)
//
// Format:      ASCII
// Channels:    1-8
// ChDelimiter: "," 
//    
// Args      - arguments list: "int8 V1, int8 V2" 
// FormatStr - format string : "%d,%d\r\n" 
// Vals      - values list   : "V1, V2"

//==============================================================================
void Chart_1_Plot(float32 V1) 
{  
    // Format:      ASCII
    // Channels:    1
    // Delimiter:   comma

    char abuff[23];    // output UART buffer // todo: static? 

    sprintf(abuff, "%g\r\n", V1);   
    UART_1_PutString(abuff);          
}
#endif



/* [] END OF FILE */



