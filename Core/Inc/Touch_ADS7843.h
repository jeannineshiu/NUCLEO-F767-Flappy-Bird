/**
  *************************************************************************************
  * File Name          : Touch_ADS7843.h
  * Description        : 觸控LCD(型號TJCTM24028)，觸控面板部分 driver 標頭檔
  *************************************************************************************
  * NCU MIAT 實驗室製作
  *************************************************************************************
  * 使用者需自定義4個callback
  * 1個Touch_ADS7843_SPITransmitReceive_Callback
  * 1個Touch_ADS7843_DigitalIn_Callback(控制T_IRQ腳)
  * 1個Touch_ADS7843_DigitalOut_Callback(控制T_CS腳)
  * 1個Touch_ADS7843_WaitMilliseconds_Callback
  * Callbacks 透過 Touch_ADS7843() 來指定
  * 此library也需要用到 SPI_TFT_ILI9341 library，呼叫 Touch_ADS7843() 時也需要傳遞 SPI_TFT_ILI9341_DriverStruct 變數位址過去
  * 
  * ++ Touch_ADS7843_SPITransmitReceive_Callback ++
  *   型別: uint8_t callback_name(uint8_t)
  *   功能: 在blocking模式下，做1個byte的SPI transmit，並等待1個byte回傳
  *         SPI設定為CPOL=1、CPHA=1、DataSize=8bits、MSB first、傳輸速率測試過421 KBits/s或850 KBits/s可運作
  *   參數: 欲傳輸的data
  *   回傳: SPI slave回傳值
  * 
  * ++ Touch_ADS7843_DigitalIn_Callback ++
  *   型別: uint8_t callback_name(void)
  *   功能: 讀取 GPIO digital input
  *   回傳: high 回傳 1，low 回傳 0
  * 
  * ++ Touch_ADS7843_DigitalOut_Callback ++
  *   型別: void callback_name(uint8_t)
  *   功能: 控制 GPIO digital output
  *   參數: 1 則GPIO拉 high ，0 則拉 low
  * 
  * ++ Touch_ADS7843_WaitMilliseconds_Callback ++
  *   型別: void callback_name(uint16_t)
  *   功能: block 數個 milliseconds
  *   參數: 欲等待的 milliseconds
  * 
  *
  * 
  * EXAMPLE:
  * #include "SPI_TFT_ILI9341.h"
  * #include "fonts.h"
  * 
  * SPI_TFT_ILI9341_DriverStruct TFT;
  * Touch_ADS7843_DriverStruct TP;
  * 
  * void TFT_spi_write_callback(uint8_t *data, uint16_t nbytes) { HAL_SPI_Transmit(&hspi6, data, nbytes, 100); }
  * void TFT_digitalOut_dc_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_DC_GPIO_Port, SPI_TFT_ILI9341_DC_Pin, high_low); }
  * void TFT_digitalOut_cs_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_CS_GPIO_Port, SPI_TFT_ILI9341_CS_Pin, high_low); }
  * void TFT_digitalOut_reset_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_RESET_GPIO_Port, SPI_TFT_ILI9341_RESET_Pin, high_low); }
  * void TFT_waitms_callback(uint16_t ms) { HAL_Delay(ms); }
  * uint8_t touch_spi_write_callback(uint8_t data)
  * {
  *    static uint8_t ret;
  *    HAL_SPI_TransmitReceive(&hspi3, &data, &ret, 1, 10);
  *    return ret;
  * }
  * void touch_digitalOut_cs_callback(uint8_t high_low) { HAL_GPIO_WritePin( TOUCH_ADS7843_CS_GPIO_Port, TOUCH_ADS7843_CS_Pin, high_low); }
  * uint8_t touch_digitalIn_irq_callback(void) { return HAL_GPIO_ReadPin(TOUCH_ADS7843_IRQ_GPIO_Port, TOUCH_ADS7843_IRQ_Pin); }
  * 
  * int main(void) {
  * 
  *   //Initialize, including SPI, GPIO...
  *   // ...
  *   //end initialization
  *   
  *   SPI_TFT_ILI9341(&TFT, &TFT_spi_write_callback, &TFT_digitalOut_cs_callback, &TFT_digitalOut_reset_callback, &TFT_digitalOut_dc_callback, &TFT_waitms_callback);
  *   Touch_ADS7843(&TP, &touch_spi_write_callback, &touch_digitalOut_cs_callback, &touch_digitalIn_irq_callback, &TFT_waitms_callback, &TFT);
  *   SPI_TFT_ILI9341_set_background(&TFT, Black);    // set background to black
  *   SPI_TFT_ILI9341_set_foreground(&TFT, White);    // set chars to white
  *   SPI_TFT_ILI9341_set_orientation(&TFT, 3);
  *   Touch_ADS7843_TouchPanelCalibrate(&TP, Arial12x12);
  * 
  *   SPI_TFT_ILI9341_cls(&TFT);
  *   while (1)
  *   {
  *     if (Touch_ADS7843_onTouch(&TP))
  *     {
  *       if (Touch_ADS7843_ReadADS7843(&TP, NULL))
  *       {
  *		   Touch_ADS7843_GetDisplayPoint(&TP);
  *         SPI_TFT_ILI9341_locate(&TFT, 25, 0);
  *         SPI_TFT_ILI9341_printf(&TFT, "%03d", TP.display.x);
  *         SPI_TFT_ILI9341_locate(&TFT, 95, 0);
  *         SPI_TFT_ILI9341_printf(&TFT, "%03d", TP.display.y);
  *       }		
  *   }
  * 
  */

#ifndef _Touch_ADS7843_H_
#define _Touch_ADS7843_H_

#include "SPI_TFT_ILI9341.h"
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct Touch_ADS7843_Coordinate
{
   int x;
   int y;
} Touch_ADS7843_Coordinate;


typedef struct Touch_ADS7843_Matrix
{
    int An,
        Bn,
        Cn,
        Dn,
        En,
        Fn,
        Divider ;
} Touch_ADS7843_Matrix;


typedef uint8_t (*Touch_ADS7843_SPITransmitReceive_Callback) (uint8_t);
typedef uint8_t (*Touch_ADS7843_DigitalIn_Callback) (void);
typedef void (*Touch_ADS7843_DigitalOut_Callback) (uint8_t);
typedef void (*Touch_ADS7843_WaitMilliseconds_Callback) (uint16_t);
typedef struct Touch_ADS7843_DriverStruct
{
   Touch_ADS7843_Coordinate display;
   Touch_ADS7843_Coordinate screen;
   SPI_TFT_ILI9341_DriverStruct *LCD;
   Touch_ADS7843_SPITransmitReceive_Callback _tp_spi;
   Touch_ADS7843_DigitalOut_Callback _tp_cs;
   Touch_ADS7843_DigitalIn_Callback _tp_irq;
   Touch_ADS7843_WaitMilliseconds_Callback _waitms;

   //private members, should not access them
   Touch_ADS7843_Coordinate DisplaySample[3];
   Touch_ADS7843_Coordinate ScreenSample[3];
   Touch_ADS7843_Matrix matrix;

} Touch_ADS7843_DriverStruct;




/*
 * Create a Touchscreen object connected to SPI and two pins.
 *
 * @param mosi,miso,sclk SPI
 * @param cs pin connected to CS of ADS7843
 * @param irq pin connected to IRQ of ADS7843
 * @param pointer to SPI_TFT constructor
 *
 */
void Touch_ADS7843(
    Touch_ADS7843_DriverStruct *handle, 
    Touch_ADS7843_SPITransmitReceive_Callback spi, 
    Touch_ADS7843_DigitalOut_Callback tp_cs, 
    Touch_ADS7843_DigitalIn_Callback tp_irq, 
    Touch_ADS7843_WaitMilliseconds_Callback waitms,
    SPI_TFT_ILI9341_DriverStruct *_LCD
 );


/*
 * Draw a 2 by 2 dot on the LCD screen.
 *
 * @param x (horizontal position)
 * @param y (vertical position)
 * @param color (16 bit pixel color)
 *
 */
//void Touch_ADS7843_DrawPoint(Touch_ADS7843_DriverStruct *handle, unsigned int Xpos,unsigned int Ypos,unsigned int color);


/*
 * Obtain averaged data from ADS7846.
 * does 9 consecutive reads and only stores averaged data
 * when the 9 points are within the treshold limits.
 *
 * @param screenPTR (pointer to store data)
 * @returns 1 on success
 * @returns 0 on failure
 *
 * If called with screenPTR = NULL - 'screen' variable is used, otherwise (parameter) is used.
 *
 */
uint8_t Touch_ADS7843_ReadADS7843(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Coordinate *screenPtr);


/*
 * Calibrate the touch panel.
 * Three crosshairs are drawn and need to be touched in sequence.  
 * A calibration matrix is set accordingly.
 *
 */
void Touch_ADS7843_TouchPanelCalibrate(Touch_ADS7843_DriverStruct *handle, const unsigned char *p_Arial12x12);


/*
 * If panel is being touched now, return 1; otherwise return 0
 *
 */
uint8_t Touch_ADS7843_OnTouch(Touch_ADS7843_DriverStruct *handle);


/*
 * Obtain real x,y coordinates.
 * The x,y coordinates are calculated using the calibration matrix.
 *
 */
uint8_t Touch_ADS7843_GetDisplayPoint(Touch_ADS7843_DriverStruct *handle);


/*
 * Read touchpanel screensample and matrix values.
 * 
 * In your code, create 2 structures using Matrix and screenPtr
 * and call this function with these structures.
 * the calibration values are returned into these structures.
 * Example:
 * Matrix matrix;
 * Coordinate ScreenSample[3];
 * GetCalibration(&matrix, &ScreenSample[0]);
 */
void Touch_ADS7843_GetCalibration(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Matrix *matrixPtr, Touch_ADS7843_Coordinate *screenPtr);


/*
 * Set touchpanel calibration using screensample and matrix values.
 * 
 * In your code, create 2 structures based on Matrix and screenPtr,
 * copy saved calibration values into these structures
 * and call this function with these structures.
 * Example:
 * Matrix matrix;
 * Coordinate ScreenSample[3];
 * <pseudocode> load matrix with values from external storage (eg eeprom).
 * <pseudocode> load ScreenSample with values from external storage (eg eeprom).
 * SetCalibration(&matrix, &ScreenSample[0]);
 */
void Touch_ADS7843_SetCalibration(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Matrix *matrixPtr, Touch_ADS7843_Coordinate *screenPtr);


#ifdef __cplusplus
} //extern "C"
#endif


#endif

