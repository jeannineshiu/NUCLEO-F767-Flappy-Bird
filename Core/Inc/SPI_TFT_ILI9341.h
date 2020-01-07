/**
  *************************************************************************************
  * File Name          : SPI_TFT_ILI9341.h
  * Description        : 觸控LCD(型號TJCTM24028)，顯示面板部分(型號ILI9341) driver 標頭檔
  *************************************************************************************
  * NCU MIAT 實驗室製作
  *************************************************************************************
  * 使用者需自定義5個callback
  * 1個SPI_TFT_ILI9341_SPITransmit_Callback
  * 3個SPI_TFT_ILI9341_DigitalOut_Callback(分別控制CS RESET D/C 3腳)
  * 1個SPI_TFT_ILI9341_WaitMilliseconds_Callback
  * Callbacks 透過 SPI_TFT_ILI9341() 來指定
  * 
  * ++ SPI_TFT_ILI9341_SPITransmit_Callback ++
  *   型別: void callback_name(uint8_t*, uint16_t)
  *   功能: 在blocking模式下，做數個bytes的SPI transmit
  *         SPI設定為CPOL=1、CPHA=1、DataSize=8bits、MSB first、傳輸速率測試過27MBits/s或30MBits/s可運作
  *   參數: 第1個參數，欲連續傳輸的陣列指標
  * 
  * ++ SPI_TFT_ILI9341_DigitalOut_Callback ++
  *   型別: void callback_name(uint8_t)
  *   功能: 控制GPIO digital output
  *   參數: 1 則GPIO拉 high ，0 則拉 low
  * 
  * ++ SPI_TFT_ILI9341_WaitMilliseconds_Callback ++
  *   型別: void callback_name(uint16_t)
  *   功能: block 數個 milliseconds
  *   參數: 欲等待的 milliseconds
  * 
  * 
  * EXAMPLE:
  * #include "SPI_TFT_ILI9341.h"
  * #include "fonts.h"
  * 
  * SPI_TFT_ILI9341_DriverStruct TFT;
  * 
  * void TFT_spi_write_callback(uint8_t *data, uint16_t nbytes) { HAL_SPI_Transmit(&hspi6, data, nbytes, 100); }
  * void TFT_digitalOut_dc_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_DC_GPIO_Port, SPI_TFT_ILI9341_DC_Pin, high_low); }
  * void TFT_digitalOut_cs_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_CS_GPIO_Port, SPI_TFT_ILI9341_CS_Pin, high_low); }
  * void TFT_digitalOut_reset_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_RESET_GPIO_Port, SPI_TFT_ILI9341_RESET_Pin, high_low); }
  * void TFT_waitms_callback(uint16_t ms) { HAL_Delay(ms); }
  * 
  * int main(void) {
  * 
  *   //Initialize, including SPI, GPIO...
  *   // ...
  *   //end initialization
  *   
  *   SPI_TFT_ILI9341(&TFT, &TFT_spi_write_callback, &TFT_digitalOut_cs_callback, &TFT_digitalOut_reset_callback, &TFT_digitalOut_dc_callback, &TFT_waitms_callback);
  *   SPI_TFT_ILI9341_set_background(&TFT, Black);    // set background to black
  *   SPI_TFT_ILI9341_set_foreground(&TFT, White);    // set chars to white
  *   SPI_TFT_ILI9341_set_orientation(&TFT, 3);
  *   SPI_TFT_ILI9341_set_font(&TFT, (unsigned char*) Arial12x12);
  * 
  *   SPI_TFT_ILI9341_cls(&TFT);
  *   SPI_TFT_ILI9341_locate(&TFT, 0, 20);
  * 
  *   SPI_TFT_ILI9341_printf(&TFT, "Hello world!\n");
  * 
  *   while(1) ;
  * }
  * 
  */

#ifndef _SPI_TFT_ILI9341_H
#define _SPI_TFT_ILI9341_H

#include "stdint.h"
#include "stdarg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RGB(r,g,b)  (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue

/* some RGB color definitions                                                 */
#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */

typedef void (*SPI_TFT_ILI9341_SPITransmit_Callback) (uint8_t*, uint16_t);
typedef void (*SPI_TFT_ILI9341_DigitalOut_Callback) (uint8_t);
typedef void (*SPI_TFT_ILI9341_WaitMilliseconds_Callback) (uint16_t);
typedef struct SPI_TFT_ILI9341_DriverStruct {
  
  SPI_TFT_ILI9341_SPITransmit_Callback _spi;
  SPI_TFT_ILI9341_DigitalOut_Callback _cs; 
  SPI_TFT_ILI9341_DigitalOut_Callback _reset;
  SPI_TFT_ILI9341_DigitalOut_Callback _dc;
  SPI_TFT_ILI9341_WaitMilliseconds_Callback _waitms;

  unsigned char* font;
  
  unsigned char spi_port; 
  unsigned int orientation;
  unsigned int char_x;
  unsigned int char_y;
  unsigned char spi_num;
  
  // character location
  uint16_t _column;
  uint16_t _row;

  // font colours
  uint16_t _foreground;
  uint16_t _background;
  char *_path;
  
  // pixel location
  short _x;
  short _y;
  
  // window location
  short _x1;
  short _x2;
  short _y1;
  short _y2;
  
} SPI_TFT_ILI9341_DriverStruct;




/** Create a SPI_TFT_ILI9341_DriverStruct object connected to SPI and three pins
 *
 * @param mosi pin connected to SDO of display
 * @param miso pin connected to SDI of display
 * @param sclk pin connected to RS of display 
 * @param cs pin connected to CS of display
 * @param reset pin connected to RESET of display
 * @param dc pin connected to WR of display
 * the IM pins have to be set to 1110 (3-0) 
 */ 
void SPI_TFT_ILI9341(
        SPI_TFT_ILI9341_DriverStruct *handle,
        SPI_TFT_ILI9341_SPITransmit_Callback spi,
        SPI_TFT_ILI9341_DigitalOut_Callback cs,
        SPI_TFT_ILI9341_DigitalOut_Callback reset,
        SPI_TFT_ILI9341_DigitalOut_Callback dc,
        SPI_TFT_ILI9341_WaitMilliseconds_Callback waitms);


/** Set foreground color
 * Effect font color
 *
 */
void SPI_TFT_ILI9341_set_foreground(SPI_TFT_ILI9341_DriverStruct *handle, uint16_t colour);


/** Set background color
 *
 */
void SPI_TFT_ILI9341_set_background(SPI_TFT_ILI9341_DriverStruct *handle, uint16_t colour);


/** Print text to screen
 *
 * @returns negative integer if failed, otherwise positive 
 *
 */
int SPI_TFT_ILI9341_printf(SPI_TFT_ILI9341_DriverStruct *handle, const char *format, ...);
 

/** Get the width of the screen in pixel
 *
 * @returns width of screen in pixel
 *
 */    
int SPI_TFT_ILI9341_width(SPI_TFT_ILI9341_DriverStruct *handle);


/** Get the height of the screen in pixel
 *
 * @returns height of screen in pixel 
 *
 */     
int SPI_TFT_ILI9341_height(SPI_TFT_ILI9341_DriverStruct *handle);
  

/** Draw a pixel at x,y with color 
 *  
 * @param x horizontal position
 * @param y vertical position
 * @param color 16 bit pixel color
 */    
void SPI_TFT_ILI9341_pixel(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y,int colour);

  
/** draw a circle
 *
 * @param x0,y0 center
 * @param r radius
 * @param color 16 bit color                                                                 *
 *
 */    
void SPI_TFT_ILI9341_circle(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y, int r, int colour); 


/** draw a filled circle
 *
 * @param x0,y0 center
 * @param r radius
 * @param color 16 bit color                                                                 *
 */    
void SPI_TFT_ILI9341_fillcircle(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y, int r, int colour); 

  
/** draw a 1 pixel line
 *
 * @param x0,y0 start point
 * @param x1,y1 stop point
 * @param color 16 bit color
 *
 */    
void SPI_TFT_ILI9341_line(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int x1, int y1, int colour);
  

/** draw a rect
 *
 * @param x0,y0 top left corner
 * @param x1,y1 down right corner
 * @param color 16 bit color
 *                                                   *
 */    
void SPI_TFT_ILI9341_rect(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int x1, int y1, int colour);

  
/** draw a filled rect
 *
 * @param x0,y0 top left corner
 * @param x1,y1 down right corner
 * @param color 16 bit color
 *
 */    
void SPI_TFT_ILI9341_fillrect(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int x1, int y1, int colour);

  
/** setup cursor position
 *
 * @param x x-position (top left)
 * @param y y-position 
 */   
void SPI_TFT_ILI9341_locate(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y);

  
/** Fill the screen with _backgroun color
 *
 */   
void SPI_TFT_ILI9341_cls(SPI_TFT_ILI9341_DriverStruct *handle);   

  
/** calculate the max number of char in a line
 *
 * @returns max columns
 * depends on actual font size
 *
 */    
int SPI_TFT_ILI9341_columns(SPI_TFT_ILI9341_DriverStruct *handle);

  
/** calculate the max number of columns
 *
 * @returns max column
 * depends on actual font size
 *
 */   
int SPI_TFT_ILI9341_rows(SPI_TFT_ILI9341_DriverStruct *handle);

  
/** put a char on the screen
 *
 * @param value char to print
 * @returns printed char
 *
 */
int SPI_TFT_ILI9341_putc(SPI_TFT_ILI9341_DriverStruct *handle, int value);

  
/** draw a character on given position out of the active font to the TFT
 *
 * @param x x-position of char (top left) 
 * @param y y-position
 * @param c char to print
 *
 */    
void SPI_TFT_ILI9341_character(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y, int c);

  
/** paint a bitmap on the TFT 
 *
 * @param x,y : upper left corner 
 * @param w width of bitmap
 * @param h high of bitmap
 * @param *bitmap pointer to the bitmap data
 *
 *   bitmap format: 16 bit R5 G6 B5
 * 
 *   use Gimp to create / load , save as BMP, option 16 bit R5 G6 B5            
 *   use winhex to load this file and mark data stating at offset 0x46 to end
 *   use edit -> copy block -> C Source to export C array
 *   paste this array into your program
 * 
 *   define the array as static const unsigned char to put it into flash memory
 *   cast the pointer to (unsigned char *) :
 *   tft.Bitmap(10,40,309,50,(unsigned char *)scala);
 */    
void SPI_TFT_ILI9341_Bitmap(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int x, unsigned int y, unsigned int w, unsigned int h, const unsigned char *bitmap);
  
  
/** paint a 16 bit BMP from filesytem on the TFT (slow) 
 *
 * @param x,y : position of upper left corner 
 * @param *Name_BMP name of the BMP file with drive: "/local/test.bmp"
 *
 * @returns 1 if bmp file was found and painted
 * @returns  0 if bmp file was found not found
 * @returns -1 if file is no bmp
 * @returns -2 if bmp file is no 16 bit bmp
 * @returns -3 if bmp file is to big for screen 
 * @returns -4 if buffer malloc go wrong
 *
 *   bitmap format: 16 bit R5 G6 B5
 * 
 *   use Gimp to create / load , save as BMP, option 16 bit R5 G6 B5
 *   copy to internal file system or SD card           
 */      
//int SPI_TFT_ILI9341_BMP_16(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int x, unsigned int y, const char *Name_BMP);  
  
  
/** select the font to use
 *
 * @param f pointer to font array 
 *                                                                              
 *   font array can created with GLCD Font Creator from http://www.mikroe.com
 *   you have to add 4 parameter at the beginning of the font array to use: 
 *   - the number of byte / char
 *   - the vertial size in pixel
 *   - the horizontal size in pixel
 *   - the number of byte per vertical line
 *   you also have to change the array to char[]
 *
 */  
void SPI_TFT_ILI9341_set_font(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char* f);

 
/** Set the orientation of the screen
 *  x,y: 0,0 is always top left 
 *
 * @param o direction to use the screen (0-3)  
 *
 */  
void SPI_TFT_ILI9341_set_orientation(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int o);


/** read out the manufacturer ID of the LCD
 *  can used for checking the connection to the display
 *  @returns ID
 */ 
//int SPI_TFT_ILI9341_read_ID(SPI_TFT_ILI9341_DriverStruct *handle);
 

/** draw a filled ellipse - source : http://enchantia.com/graphapp/doc/tech/ellipses.html
 *
 * @param xc,yc center point
 * @param a,b semi-major axis and semi-minor axis
 * @param color 16 bit color
 *
 */
void SPI_TFT_ILI9341_ellipse(SPI_TFT_ILI9341_DriverStruct *handle, int xc, int yc, int a, int b, int color);
 
 
/** draw a filled ellipse - source : http://enchantia.com/graphapp/doc/tech/ellipses.html
 *
 * @param xc,yc center point
 * @param a,b semi-major axis and semi-minor axis
 * @param color 16 bit color
 *
 */
//void SPI_TFT_ILI9341_fill_ellipse(SPI_TFT_ILI9341_DriverStruct *handle, int xc, int yc, int a, int b, int color);


#ifdef __cplusplus
} //extern "C"
#endif


#endif //_SPI_TFT_ILI9341_H
