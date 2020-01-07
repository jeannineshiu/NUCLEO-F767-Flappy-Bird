#include "Touch_ADS7843.h"

#define THRESHOLD 2
#define    SPI_RD_DELAY    1
#define    CHX             0xd1    // 12 bit mode
#define    CHY             0x90



// ********** Private functions prototype ***************************************************
/*
* Obtain raw x,y data from ADS7846
*
* @param pointer to raw x and y coordinates (pointer to store data)
* @returns x (horizontal position)
* @returns y (vertical position)
*
*/
static void TP_GetAdXY(Touch_ADS7843_DriverStruct *handle, int *x,int *y);

/*
* Obtain raw single channel data from ADS7846 (Called by TP_GetADXY)
*
* @param channel to be read (CHX or CHY)
* @returns raw scaled down value (return value range must be between 0 and 1024)
*
*/
static int Read_XY(Touch_ADS7843_DriverStruct *handle, uint8_t XY);

/*
* Draw a calibration crosshair on the LCD screen
*
* @param x (horizontal position)
* @param y (vertical position)
*
*/
static void DrawCross(Touch_ADS7843_DriverStruct *handle, unsigned int Xpos,unsigned int Ypos);

/*
* Set the calibration matrix
*
* @param displayPTR (pointer to display data)
* @param screenPTR  (pointer to screen data)
* @param matrixPTR  (pointer to calibration matrix)
*
* @returns 0 when matrix.divider != 0
* @returns 1 when matrix.divider = 0
*
*/
static uint8_t setCalibrationMatrix(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Coordinate * displayPtr, Touch_ADS7843_Coordinate * screenPtr, Touch_ADS7843_Matrix * matrixPtr);

// ********** Private functions prototype *****************************(END)*****************






void Touch_ADS7843(
    Touch_ADS7843_DriverStruct *handle, 
    Touch_ADS7843_SPITransmitReceive_Callback spi, 
    Touch_ADS7843_DigitalOut_Callback tp_cs, 
    Touch_ADS7843_DigitalIn_Callback tp_irq, 
    Touch_ADS7843_WaitMilliseconds_Callback waitms,
    SPI_TFT_ILI9341_DriverStruct *_LCD)
{
    handle->LCD = _LCD;
    handle->_tp_spi = spi;
    handle->_tp_cs = tp_cs;
    handle->_tp_irq = tp_irq;
    handle->_waitms = waitms;
    handle->DisplaySample[0].x=45;
    handle->DisplaySample[0].y=45;
    handle->DisplaySample[1].x=45;
    handle->DisplaySample[1].y=210;
    handle->DisplaySample[2].x=190;
    handle->DisplaySample[2].y=190;
    handle->ScreenSample[0].x=45;
    handle->ScreenSample[0].y=45;
    handle->ScreenSample[1].x=45;
    handle->ScreenSample[1].y=210;
    handle->ScreenSample[2].x=190;
    handle->ScreenSample[2].y=190;
    handle->_tp_cs(1);
    //handle->_tp_spi->frequency(850000);  //500000
    //handle->_tp_spi->format(8,3);                    // 8 bit spi mode 0
}

uint8_t Touch_ADS7843_OnTouch(Touch_ADS7843_DriverStruct *handle) { return !(handle->_tp_irq()); }

int Read_XY(Touch_ADS7843_DriverStruct *handle, uint8_t XY)
{
    uint8_t msb, lsb;
    unsigned int Temp;

    handle->_tp_cs(0);
    //wait_us(SPI_RD_DELAY);
    for(Temp=0; Temp<3; ++Temp);
    handle->_tp_spi(XY);
    //wait_us(SPI_RD_DELAY);
    for(Temp=0; Temp<3; ++Temp);
    msb = handle->_tp_spi(0x00);  // msb
    //wait_us(SPI_RD_DELAY);
    for(Temp=0; Temp<3; ++Temp);
    lsb = handle->_tp_spi(0x00);  // lsb
    handle->_tp_cs(1);
    Temp = ((msb << 8 ) | lsb);
    Temp >>= 3;
    Temp &= 0xfff;
    Temp /= 4;                  // Scaling : return value range must be between 0 and 1024
    return(Temp);
}


void TP_GetAdXY(Touch_ADS7843_DriverStruct *handle, int *x,int *y)
{
    int adx, ady, temp;
    adx = Read_XY(handle, CHX);
    //wait_us(1);
    for(temp=0; temp<3; ++temp);
    ady = Read_XY(handle, CHY);
    *x = adx;
    *y = ady;
}

//void Touch_ADS7843_DrawPoint(Touch_ADS7843_DriverStruct *handle, unsigned int Xpos,unsigned int Ypos,unsigned int color)
//{
////    LCD->wr_reg(0x03, 0x1030);
////    LCD->SPI::write(0x03);
//    WindowMax(LCD);
//    SPI_TFT_ILI9341_pixel(LCD,Xpos,Ypos,color);
//    SPI_TFT_ILI9341_pixel(LCD,Xpos+1,Ypos,color);
//    SPI_TFT_ILI9341_pixel(LCD,Xpos,Ypos+1,color);
//    SPI_TFT_ILI9341_pixel(LCD,Xpos+1,Ypos+1,color);
//}

void DrawCross(Touch_ADS7843_DriverStruct *handle, unsigned int Xpos,unsigned int Ypos)
{
    SPI_TFT_ILI9341_line(handle->LCD,Xpos-15,Ypos,Xpos-2,Ypos,White);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos+2,Ypos,Xpos+15,Ypos,White);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos,Ypos-15,Xpos,Ypos-2,White);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos,Ypos+2,Xpos,Ypos+15,White);

    SPI_TFT_ILI9341_line(handle->LCD,Xpos-15,Ypos+15,Xpos-7,Ypos+15,DarkGrey);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos-15,Ypos+7,Xpos-15,Ypos+15,DarkGrey);

    SPI_TFT_ILI9341_line(handle->LCD,Xpos-15,Ypos-15,Xpos-7,Ypos-15,DarkGrey);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos-15,Ypos-7,Xpos-15,Ypos-15,DarkGrey);

    SPI_TFT_ILI9341_line(handle->LCD,Xpos+7,Ypos+15,Xpos+15,Ypos+15,DarkGrey);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos+15,Ypos+7,Xpos+15,Ypos+15,DarkGrey);

    SPI_TFT_ILI9341_line(handle->LCD,Xpos+7,Ypos-15,Xpos+15,Ypos-15,DarkGrey);
    SPI_TFT_ILI9341_line(handle->LCD,Xpos+15,Ypos-15,Xpos+15,Ypos-7,DarkGrey);
}

uint8_t Touch_ADS7843_ReadADS7843(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Coordinate *screenPtr)
{
    int m0,m1,m2,TP_X[1],TP_Y[1],temp[3];
    uint8_t count=0;
    int buffer[2][9]={{0},{0}};
    if (!screenPtr) screenPtr = &(handle->screen);
    do
    {
        TP_GetAdXY(handle,TP_X,TP_Y);
        buffer[0][count]=TP_X[0];
        buffer[1][count]=TP_Y[0];
        count++;
    }
    while(!handle->_tp_irq() && (count < 9));
    if(count==9)
    {
        temp[0]=(buffer[0][0]+buffer[0][1]+buffer[0][2])/3;
        temp[1]=(buffer[0][3]+buffer[0][4]+buffer[0][5])/3;
        temp[2]=(buffer[0][6]+buffer[0][7]+buffer[0][8])/3;
        m0=temp[0]-temp[1];
        m1=temp[1]-temp[2];
        m2=temp[2]-temp[0];
        m0=m0>0?m0:(-m0);
        m1=m1>0?m1:(-m1);
        m2=m2>0?m2:(-m2);
        if( (m0>THRESHOLD)  &&  (m1>THRESHOLD)  &&  (m2>THRESHOLD) ) return 0;
        if(m0<m1)
        {
            if(m2<m0)
                screenPtr->x=(temp[0]+temp[2])/2;
            else
                screenPtr->x=(temp[0]+temp[1])/2;
        }
        else if(m2<m1)
            screenPtr->x=(temp[0]+temp[2])/2;
        else
            screenPtr->x=(temp[1]+temp[2])/2;

        temp[0]=(buffer[1][0]+buffer[1][1]+buffer[1][2])/3;
        temp[1]=(buffer[1][3]+buffer[1][4]+buffer[1][5])/3;
        temp[2]=(buffer[1][6]+buffer[1][7]+buffer[1][8])/3;
        m0=temp[0]-temp[1];
        m1=temp[1]-temp[2];
        m2=temp[2]-temp[0];
        m0=m0>0?m0:(-m0);
        m1=m1>0?m1:(-m1);
        m2=m2>0?m2:(-m2);
        if( (m0>THRESHOLD)  &&  (m1>THRESHOLD)  &&  (m2>THRESHOLD) ) return 0;

        if(m0<m1)
        {
            if(m2<m0)
                screenPtr->y=(temp[0]+temp[2])/2;
            else
                screenPtr->y=(temp[0]+temp[1])/2;
        }
        else if(m2<m1)
            screenPtr->y=(temp[0]+temp[2])/2;
        else
            screenPtr->y=(temp[1]+temp[2])/2;
        return 1;
    }
    return 0;
}

uint8_t setCalibrationMatrix(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Coordinate * displayPtr, Touch_ADS7843_Coordinate * screenPtr, Touch_ADS7843_Matrix * matrixPtr)
{
    uint8_t retTHRESHOLD = 0 ;
    // K = (Xs0 - Xs2)*(Ys1 - Ys2) - (Xs1 - Xs2)*(Ys0 - Ys2)
    matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                         ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
    if( matrixPtr->Divider == 0 )
    {
        retTHRESHOLD = 1;
    }
    else
    {
        //                 (Xd0 - Xd2)*(Ys1 - Ys2) - (Xd1 - Xd2)*(Ys0 - Ys2)
        //            A = ---------------------------------------------------
        //                                   K
        matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                          ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
        //                 (Xs0 - Xs2)*(Xd1 - Xd2) - (Xd0 - Xd2)*(Xs1 - Xs2)
        //            B = ---------------------------------------------------
        //                                   K
        matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) -
                        ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
        //                 Ys0*(Xs2*Xd1 - Xs1*Xd2) + Ys1*(Xs0*Xd2 - Xs2*Xd0) + Ys2*(Xs1*Xd0 - Xs0*Xd1)
        //            C = ----------------------------------------------------------------------------
        //                                   K
        matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                        (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                        (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
        //                 (Yd0 - Yd2)*(Ys1 - Ys2) - (Yd1 - Yd2)*(Ys0 - Ys2)
        //            D = ---------------------------------------------------
        //                                   K
        matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) -
                        ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
        //                 (Xs0 - Xs2)*(Yd1 - Yd2) - (Yd0 - Yd2)*(Xs1 - Xs2)
        //            E = ---------------------------------------------------
        //                                   K
        matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) -
                        ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
        //                 Ys0*(Xs2*Yd1 - Xs1*Yd2) + Ys1*(Xs0*Yd2 - Xs2*Yd0) + Ys2*(Xs1*Yd0 - Xs0*Yd1)
        //            F = ----------------------------------------------------------------------------
        //                                   K
        matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                        (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                        (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
    }
    return( retTHRESHOLD ) ;
}

uint8_t Touch_ADS7843_GetDisplayPoint(Touch_ADS7843_DriverStruct *handle)
{
    uint8_t retTHRESHOLD = 0 ;

    if( handle->matrix.Divider != 0 )
    {
        // XD = AX+BY+C
        handle->display.x = ( (handle->matrix.An * handle->screen.x) +
                              (handle->matrix.Bn * handle->screen.y) +
                               handle->matrix.Cn
                            ) / handle->matrix.Divider ;
        // YD = DX+EY+F
        handle->display.y = ( (handle->matrix.Dn * handle->screen.x) +
                      (handle->matrix.En * handle->screen.y) +
                       handle->matrix.Fn
                    ) / handle->matrix.Divider ;
    }
    else
    {
        retTHRESHOLD = 1;
    }
    return(retTHRESHOLD);
}

void Touch_ADS7843_TouchPanelCalibrate(Touch_ADS7843_DriverStruct *handle, const unsigned char *p_Arial12x12)
{
    uint8_t i;
    Touch_ADS7843_Coordinate screen_cal;
    setCalibrationMatrix(handle, &(handle->DisplaySample[0]), &(handle->ScreenSample[0]), &(handle->matrix));
    SPI_TFT_ILI9341_set_font(handle->LCD, (unsigned char*) p_Arial12x12);
    for(i=0;i<3;i++)
    {
        SPI_TFT_ILI9341_cls(handle->LCD);
        SPI_TFT_ILI9341_locate(handle->LCD, 10, 10);
        SPI_TFT_ILI9341_printf(handle->LCD, "Touch crosshair to calibrate");
        //wait_ms(500);
        handle->_waitms(500);
        DrawCross(handle, handle->DisplaySample[i].x, handle->DisplaySample[i].y);
        do {} while (!Touch_ADS7843_ReadADS7843(handle,&screen_cal));
        handle->ScreenSample[i].x = screen_cal.x;
        handle->ScreenSample[i].y = screen_cal.y;
    }
    setCalibrationMatrix(handle, &(handle->DisplaySample[0]), &(handle->ScreenSample[0]), &(handle->matrix));
    SPI_TFT_ILI9341_cls(handle->LCD);
}

void Touch_ADS7843_GetCalibration(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Matrix *matrixPtr, Touch_ADS7843_Coordinate *screenPtr)
{
    uint8_t i;
    Touch_ADS7843_Matrix * mp1;
    mp1 = &(handle->matrix);
    *matrixPtr = *mp1;
    for(i=0;i<3;i++)
    {
        screenPtr[i].x = handle->ScreenSample[i].x;
        screenPtr[i].y = handle->ScreenSample[i].y;
    }
}

void Touch_ADS7843_SetCalibration(Touch_ADS7843_DriverStruct *handle, Touch_ADS7843_Matrix *matrixPtr, Touch_ADS7843_Coordinate *screenPtr)
{
    uint8_t i;
    Touch_ADS7843_Matrix * mp1;
    mp1 = &(handle->matrix);
    *mp1 = *matrixPtr;
    for(i=0;i<3;i++)
    {
        handle->ScreenSample[i].x = screenPtr[i].x;
        handle->ScreenSample[i].y = screenPtr[i].y;
    }
    setCalibrationMatrix(handle, &(handle->DisplaySample[0]), &(handle->ScreenSample[0]), &(handle->matrix)) ;
}


