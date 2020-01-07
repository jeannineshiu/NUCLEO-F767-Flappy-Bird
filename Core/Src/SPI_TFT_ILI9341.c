
#include "SPI_TFT_ILI9341.h"

#define BPP             16                  // Bits per pixel    
#define INBUF_SIZ       320*2               // Don't be smaller than 320*2
#define PRINTFBUF_SIZ   128                 // Buffer size used in SPI_TFT_ILI9341_printf()


// ********** Private functions prototype ***************************************************

/** Set draw window region to whole screen
 *
 */  
static void WindowMax(SPI_TFT_ILI9341_DriverStruct *handle);

  
/** draw a horizontal line
 *
 * @param x0 horizontal start
 * @param x1 horizontal stop
 * @param y vertical position
 * @param color 16 bit color                                               
 *
 */
static void hline(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int x1, int y, int colour);


/** draw a vertical line
 *
 * @param x horizontal position
 * @param y0 vertical start 
 * @param y1 vertical stop
 * @param color 16 bit color
 */
static void vline(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y0, int y1, int colour);

 
/** Set draw window region
 *
 * @param x horizontal position
 * @param y vertical position
 * @param w window width in pixel
 * @param h window height in pixels
 */    
static void window(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int x,unsigned int y, unsigned int w, unsigned int h);

 
/** Init the ILI9341 controller 
 *
 */    
static void tft_reset(SPI_TFT_ILI9341_DriverStruct *handle);
 

/** Write data to the LCD controller
 *
 * @param dat data written to LCD controller
 * 
 */   
static void wr_dat(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char value);


/** Write a command the LCD controller 
 *
 * @param cmd: command to be written   
 *
 */   
static void wr_cmd(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char value);

    
/** Read byte from the LCD controller
 *
 * @param cmd comand to controller
 * @returns data from LCD controller
 *  
 */    
static char rd_byte(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char cmd);


/** Read 32 bits from the LCD controller
 *
 * @param cmd comand to controller
 * @returns data from LCD controller
 *  
 */
static int rd_32(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char cmd);  
    
    
/** Write a value to the to a LCD register
 *
 * @param reg register to be written
 * @param val data to be written
 */   
//static void wr_reg(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char reg, unsigned char val);
    
    
// ********** Private functions prototype ************************************(END)**********






static uint8_t _buf[INBUF_SIZ];
static int _bufi;
#define INIBUF() _bufi = 0
#define WBUF(x) _buf[_bufi++] = (x)



void SPI_TFT_ILI9341(
          SPI_TFT_ILI9341_DriverStruct *handle,
          SPI_TFT_ILI9341_SPITransmit_Callback spi,
          SPI_TFT_ILI9341_DigitalOut_Callback cs,
          SPI_TFT_ILI9341_DigitalOut_Callback reset,
          SPI_TFT_ILI9341_DigitalOut_Callback dc,
          SPI_TFT_ILI9341_WaitMilliseconds_Callback waitms)
{
    handle->_spi = spi;
    handle->_cs = cs;
    handle->_reset = reset;
    handle->_dc = dc;
    handle->_waitms = waitms;
    handle->orientation = 0;
    handle->char_x = 0;
    //handle->_spi->format(8,3);                  // 8 bit spi mode 3
    //handle->_spi->frequency(30000000);          // 30 Mhz SPI clock 30000000
    tft_reset(handle);
}

void SPI_TFT_ILI9341_set_foreground(SPI_TFT_ILI9341_DriverStruct *handle, uint16_t colour) {
    handle->_foreground = colour;
}

void SPI_TFT_ILI9341_set_background(SPI_TFT_ILI9341_DriverStruct *handle, uint16_t colour) {
    handle->_background = colour;
}

int SPI_TFT_ILI9341_width(SPI_TFT_ILI9341_DriverStruct *handle)
{
    if (handle->orientation == 0 || handle->orientation == 2) return 240;
    else return 320;
}


int SPI_TFT_ILI9341_height(SPI_TFT_ILI9341_DriverStruct *handle)
{
    if (handle->orientation == 0 || handle->orientation == 2) return 320;
    else return 240;
}


void SPI_TFT_ILI9341_set_orientation(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int o)
{
    handle->orientation = o;
    wr_cmd(handle, 0x36);                     // MEMORY_ACCESS_CONTROL
    switch (o) {
        case 0:
            _buf[0] = 0x48;
            break;
        case 1:
            _buf[0] = 0x28;
            break;
        case 2:
            _buf[0] = 0x88;
            break;
        case 3:
            _buf[0] = 0xE8;
            break;
    }
    handle->_spi(_buf, 1);
    handle->_cs(1); 
    WindowMax(handle);
} 


// write command to tft register
void wr_cmd(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char cmd)
{
    handle->_dc(0);
    handle->_cs(0);
    _buf[0] = cmd;      // mbed lib
    handle->_spi(_buf, 1);
    handle->_dc(1);
}


void wr_dat(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char value)
{
   _buf[0] = value;
   handle->_spi(_buf, 1);
}


// the ILI9341 can read 
//char rd_byte(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char cmd)
//{
    //char r;
    //handle->_dc(0);
    //handle->_cs(0);
    //handle->__spi(cmd);      // mbed lib
    //handle->_cs(1);
    //r = handle->__spi(0xff);
    //handle->_cs(1);    
    //return r;
//}

//// read 32 bit
//int rd_32(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char cmd)
//{
    //int d;
    //char r;
    //handle->_dc(0);
    //handle->_cs(0);
    
    //d = cmd;
    ////d = d << 1;
    ////SPI::format(9,3);    // we have to add a dummy clock cycle
    //handle->__spi(d);
    ////handle->_spi->format(8,3);   
    //handle->_dc(1);
    //r = handle->__spi(0xff);
    //d = r;
    //r = handle->__spi(0xff);
    //d = (d << 8) | r;
    //r = handle->__spi(0xff);
    //d = (d << 8) | r;
    //r = handle->__spi(0xff);
    //d = (d << 8) | r;
    //handle->_cs(1);   
    //return d;
//}

//int SPI_TFT_ILI9341_read_ID(SPI_TFT_ILI9341_DriverStruct *handle)
//{
    //int r;
    ////r = rd_byte(0xd3);
    ////r = rd_byte(0xd3);
    ////r = rd_byte(0xd3);
    ////r = rd_byte(0xd3);
    //r=rd_32(handle,0xd3);
    //return r;
//}


// Init code based on MI0283QT datasheet
void tft_reset(SPI_TFT_ILI9341_DriverStruct *handle)
{
    handle->_cs(1);                           // cs high
    handle->_dc(1);                           // dc high 
    handle->_reset(0);                        // display reset

    //wait_us(50);
    handle->_waitms(1);
    handle->_reset(1);                       // end hardware reset
    handle->_waitms(5);

    wr_cmd(handle,0x01);                     // SW reset  
    handle->_waitms(5);
    wr_cmd(handle,0x28);                     // display off  

    /* Start Initial Sequence ----------------------------------------------------*/
    wr_cmd(handle,0xCF);                     
    INIBUF();
    WBUF(0x00);
    WBUF(0x83);
    WBUF(0x30);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xED);                     
    INIBUF();
    WBUF(0x64);
    WBUF(0x03);
    WBUF(0x12);
    WBUF(0x81);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xE8);                     
    INIBUF();
    WBUF(0x85);
    WBUF(0x01);
    WBUF(0x79);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xCB);                     
    INIBUF();
    WBUF(0x39);
    WBUF(0x2C);
    WBUF(0x00);
    WBUF(0x34);
    WBUF(0x02);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);

    wr_cmd(handle,0xF7);                     
    _buf[0] = 0x20;
    handle->_spi(_buf, 1);
    handle->_cs(1);

    wr_cmd(handle,0xEA);                     
    INIBUF();
    WBUF(0x00);
    WBUF(0x00);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xC0);                     // POWER_CONTROL_1
    _buf[0] = 0x26;
    handle->_spi(_buf, 1);
    handle->_cs(1);
 
    wr_cmd(handle,0xC1);                     // POWER_CONTROL_2
    _buf[0] = 0x11;
    handle->_spi(_buf, 1);
    handle->_cs(1);
    
    wr_cmd(handle,0xC5);                     // VCOM_CONTROL_1
    INIBUF();
    WBUF(0x35);
    WBUF(0x3E);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xC7);                     // VCOM_CONTROL_2
    _buf[0] = 0xBE;
    handle->_spi(_buf, 1);
    handle->_cs(1); 
    
    wr_cmd(handle,0x36);                     // MEMORY_ACCESS_CONTROL
    _buf[0] = 0x48;
    handle->_spi(_buf, 1);
    handle->_cs(1); 
    
    wr_cmd(handle,0x3A);                     // COLMOD_PIXEL_FORMAT_SET
    _buf[0] = 0x55;
    handle->_spi(_buf, 1);
    handle->_cs(1);
    
    wr_cmd(handle,0xB1);                     // Frame Rate
    INIBUF();
    WBUF(0x00);
    WBUF(0x1B);               
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xF2);                     // Gamma Function Disable
    _buf[0] = 0x08;
    handle->_spi(_buf, 1);
    handle->_cs(1); 
    
    wr_cmd(handle,0x26);                     
    _buf[0] = 0x01;
    handle->_spi(_buf, 1);
    handle->_cs(1); 
    
    wr_cmd(handle,0xE0);                     // positive gamma correction
    INIBUF();
    WBUF(0x1F); 
    WBUF(0x1A); 
    WBUF(0x18); 
    WBUF(0x0A); 
    WBUF(0x0F); 
    WBUF(0x06); 
    WBUF(0x45); 
    WBUF(0x87); 
    WBUF(0x32); 
    WBUF(0x0A); 
    WBUF(0x07); 
    WBUF(0x02); 
    WBUF(0x07);
    WBUF(0x05); 
    WBUF(0x00);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0xE1);                     // negativ gamma correction
    INIBUF();
    WBUF(0x00); 
    WBUF(0x25); 
    WBUF(0x27); 
    WBUF(0x05); 
    WBUF(0x10); 
    WBUF(0x09); 
    WBUF(0x3A); 
    WBUF(0x78); 
    WBUF(0x4D); 
    WBUF(0x05); 
    WBUF(0x18); 
    WBUF(0x0D); 
    WBUF(0x38);
    WBUF(0x3A); 
    WBUF(0x1F);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    WindowMax(handle);
    
    //wr_cmd(handle,0x34);                     // tearing effect off
    //handle->_cs(1);
    
    //wr_cmd(handle,0x35);                     // tearing effect on
    //handle->_cs(1);
    
    wr_cmd(handle,0xB7);                       // entry mode
    _buf[0] = 0x07;
    handle->_spi(_buf, 1);
    handle->_cs(1);
    
    wr_cmd(handle,0xB6);                       // display function control
    INIBUF();
    WBUF(0x0A);
    WBUF(0x82);
    WBUF(0x27);
    WBUF(0x00);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    
    wr_cmd(handle,0x11);                     // sleep out
    handle->_cs(1);
    
    handle->_waitms(100);
    
    wr_cmd(handle,0x29);                     // display on
    handle->_cs(1);
    
    handle->_waitms(100);
    
 }


void SPI_TFT_ILI9341_pixel(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y,int colour)
{
    wr_cmd(handle, 0x2A);
    INIBUF();
    WBUF(x >> 8);
    WBUF(x);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    wr_cmd(handle, 0x2B);
    INIBUF();
    WBUF(y >> 8);
    WBUF(y); 
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    wr_cmd(handle, 0x2C);  // send pixel
    INIBUF();
    WBUF(colour >> 8);
    WBUF(colour & 0xff);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
}


void window(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int x,unsigned int y, unsigned int w, unsigned int h)
{
    wr_cmd(handle,0x2A);
    INIBUF();
    WBUF(x >> 8);
    WBUF(x);
    WBUF((x+w-1) >> 8);
    WBUF(x+w-1);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    wr_cmd(handle,0x2B);
    INIBUF();
    WBUF(y >> 8);
    WBUF(y);
    WBUF((y+h-1) >> 8);
    WBUF(y+h-1);
    handle->_spi(_buf, _bufi);
    handle->_cs(1);

}


void WindowMax(SPI_TFT_ILI9341_DriverStruct *handle)
{
    window(handle, 0, 0, SPI_TFT_ILI9341_width(handle), SPI_TFT_ILI9341_height(handle));
}



void SPI_TFT_ILI9341_cls(SPI_TFT_ILI9341_DriverStruct *handle)  
{
   // we can use the fillrect function 
   //SPI_TFT_ILI9341_fillrect(handle, 0, 0, SPI_TFT_ILI9341_width(handle)-1, SPI_TFT_ILI9341_height(handle)-1, handle->_background);
   int x = SPI_TFT_ILI9341_width(handle) - 1;
   int h = SPI_TFT_ILI9341_height(handle);
   int i;
   for (i = 0; i < h; ++i)
       hline(handle, 0, x, i, handle->_background);
}


void SPI_TFT_ILI9341_circle(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int r, int color)
{

    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        SPI_TFT_ILI9341_pixel(handle, x0-x, y0+y, color);
        SPI_TFT_ILI9341_pixel(handle, x0+x, y0+y, color);
        SPI_TFT_ILI9341_pixel(handle, x0+x, y0-y, color);
        SPI_TFT_ILI9341_pixel(handle, x0-x, y0-y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);

}

void SPI_TFT_ILI9341_fillcircle(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        vline(handle, x0-x, y0-y, y0+y, color);
        vline(handle, x0+x, y0-y, y0+y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}


void hline(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int x1, int y, int color)
{
    int w;
    w = x1 - x0 + 1;
    window(handle,x0,y,w,1);
    wr_cmd(handle,0x2C);  // send pixel
    int j;
    INIBUF();
    for (j=0; j<w; j++) {
        WBUF(color >> 8);
        WBUF(color & 0xff);
    }
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    WindowMax(handle);
    return;
}

void vline(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y0, int y1, int color)
{
    int h;
    h = y1 - y0 + 1;
    window(handle,x,y0,1,h);
    wr_cmd(handle,0x2C);  // send pixel
    INIBUF();
    int y;
    for (y=0; y<h; y++) {
        WBUF(color >> 8);
        WBUF(color & 0xff);
    } 
    handle->_spi(_buf, _bufi);
    handle->_cs(1);
    WindowMax(handle);
    return;

}


void SPI_TFT_ILI9341_line(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int x1, int y1, int color)
{
    //WindowMax();
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    if (dx == 0) {        /* vertical line */
        if (y1 > y0) vline(handle,x0,y0,y1,color);
        else vline(handle,x0,y1,y0,color);
        return;
    }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    if (dy == 0) {        /* horizontal line */
        if (x1 > x0) hline(handle,x0,x1,y0,color);
        else  hline(handle,x1,x0,y0,color);
        return;
    }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            SPI_TFT_ILI9341_pixel(handle, x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        SPI_TFT_ILI9341_pixel(handle, x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            SPI_TFT_ILI9341_pixel(handle, x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        SPI_TFT_ILI9341_pixel(handle, x0, y0, color);
    }
    return;
}


void SPI_TFT_ILI9341_rect(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int x1, int y1, int color)
{

    if (x1 > x0) hline(handle,x0,x1,y0,color);
    else  hline(handle,x1,x0,y0,color);

    if (y1 > y0) vline(handle,x0,y0,y1,color);
    else vline(handle,x0,y1,y0,color);

    if (x1 > x0) hline(handle,x0,x1,y1,color);
    else  hline(handle,x1,x0,y1,color);

    if (y1 > y0) vline(handle,x1,y0,y1,color);
    else vline(handle,x1,y1,y0,color);

    return;
}



void SPI_TFT_ILI9341_fillrect(SPI_TFT_ILI9341_DriverStruct *handle, int x0, int y0, int x1, int y1, int color)
{
    for(; y0<y1; ++y0)
        hline(handle, x0, x1, y0, color);
}


void SPI_TFT_ILI9341_locate(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y)
{
    handle->char_x = x;
    handle->char_y = y;
}



int SPI_TFT_ILI9341_columns(SPI_TFT_ILI9341_DriverStruct *handle)
{
    return SPI_TFT_ILI9341_width(handle) / handle->font[1];
}



int SPI_TFT_ILI9341_rows(SPI_TFT_ILI9341_DriverStruct *handle)
{
    return SPI_TFT_ILI9341_height(handle) / handle->font[2];
}


int SPI_TFT_ILI9341_printf(SPI_TFT_ILI9341_DriverStruct *handle, const char *format, ...)
{
    static int n, i;
    static char b[PRINTFBUF_SIZ];

    va_list args;
    va_start(args, format);
    n = vsprintf(b, format, args);
    va_end (args);

    if(n <= 0) return n;
    if(n > INBUF_SIZ) return -1;
    for(i=0; i<n; ++i)
        SPI_TFT_ILI9341_putc(handle, b[i]);
    return n;
}

int SPI_TFT_ILI9341_putc(SPI_TFT_ILI9341_DriverStruct *handle, int value)
{
    if (value == '\n') {    // new line
        handle->char_x = 0;
        handle->char_y = handle->char_y + handle->font[2];
        if (handle->char_y >= SPI_TFT_ILI9341_height(handle) - handle->font[2]) {
            handle->char_y = 0;
        }
    } else {
        SPI_TFT_ILI9341_character(handle, handle->char_x, handle->char_y, value);
    }
    return value;
}


void SPI_TFT_ILI9341_character(SPI_TFT_ILI9341_DriverStruct *handle, int x, int y, int c)
{
    unsigned int hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = handle->font[0];                    // bytes / char
    hor = handle->font[1];                       // get hor size of font
    vert = handle->font[2];                      // get vert size of font
    bpl = handle->font[3];                       // bytes per line

    if (handle->char_x + hor > SPI_TFT_ILI9341_width(handle)) {
        handle->char_x = 0;
        handle->char_y = handle->char_y + vert;
        if (handle->char_y >= SPI_TFT_ILI9341_height(handle) - handle->font[2]) {
            handle->char_y = 0;
        }
    }
    window(handle, handle->char_x, handle->char_y, hor, vert); // char box
    wr_cmd(handle, 0x2C);  // send pixel
    zeichen = handle->font + ((c -32) * offset + 4); // start of char bitmap
    w = zeichen[0];                          // width of actual char
    for (j=0; j<vert; j++) {  //  vert line
        INIBUF();
        for (i=0; i<hor; i++) {   //  horz line
            z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00) {
                WBUF(handle->_background >> 8);
                WBUF(handle->_background & 0xff);
            } else {
                WBUF(handle->_foreground >> 8);
                WBUF(handle->_foreground & 0xff);
            }
        }
        handle->_spi(_buf, _bufi);
    }
    handle->_cs(1);
    WindowMax(handle);
    if ((w + 2) < hor) {                   // x offset to next char
        handle->char_x += w + 2;
    } else handle->char_x += hor;
}


void SPI_TFT_ILI9341_set_font(SPI_TFT_ILI9341_DriverStruct *handle, unsigned char* f)
{
    handle->font = f;
}



void SPI_TFT_ILI9341_Bitmap(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int x, unsigned int y, unsigned int w, unsigned int h, const unsigned char *bitmap)
{
    unsigned int j;
    int padd;
    unsigned short *bitmap_ptr = (unsigned short *)bitmap;
    unsigned short pix_temp;
    
    unsigned int i;
    
    // the lines are padded to multiple of 4 bytes in a bitmap
    padd = -1;
    do {
        padd ++;
    } while (2*(w + padd)%4 != 0);
    window(handle, x, y, w, h);
    bitmap_ptr += ((h - 1)* (w + padd));
    wr_cmd(handle, 0x2C);  // send pixel
    for (j = 0; j < h; j++) {         //Lines
        INIBUF();
        for (i = 0; i < w; i++) {     // one line
            pix_temp = *bitmap_ptr;
            WBUF(pix_temp >> 8);
            WBUF(pix_temp);
            bitmap_ptr++;
        }
        handle->_spi(_buf, _bufi);
        bitmap_ptr -= 2*w;
        bitmap_ptr -= padd;
    }
    handle->_cs(1);
    WindowMax(handle);
}


// local filesystem is not implemented in kinetis board , but you can add a SD card
//int SPI_TFT_ILI9341_BMP_16(SPI_TFT_ILI9341_DriverStruct *handle, unsigned int x, unsigned int y, const char *Name_BMP)
//{
//
//#define OffsetPixelWidth    18
//#define OffsetPixelHeigh    22
//#define OffsetFileSize      34
//#define OffsetPixData       10
//#define OffsetBPP           28
//
//    char filename[50];
//    unsigned char BMP_Header[54];
//    unsigned short BPP_t;
//    unsigned int PixelWidth,PixelHeigh,start_data;
//    unsigned int    i,off;
//    int padd,j;
//    unsigned short *line;
//
//    // get the filename
//    i=0;
//    while (*Name_BMP!='\0') {
//        filename[i++]=*Name_BMP++;
//    }
//    filename[i] = 0;  
//    
//    FILE *Image = fopen((const char *)&filename[0], "rb");  // open the bmp file
//    if (!Image) {
//        return(0);      // error file not found !
//    }
//
//    fread(&BMP_Header[0],1,54,Image);      // get the BMP Header
//
//    if (BMP_Header[0] != 0x42 || BMP_Header[1] != 0x4D) {  // check magic byte
//        fclose(Image);
//        return(-1);     // error no BMP file
//    }
//
//    BPP_t = BMP_Header[OffsetBPP] + (BMP_Header[OffsetBPP + 1] << 8);
//    if (BPP_t != 0x0010) {
//        fclose(Image);
//        return(-2);     // error no 16 bit BMP
//    }
//
//    PixelHeigh = BMP_Header[OffsetPixelHeigh] + (BMP_Header[OffsetPixelHeigh + 1] << 8) + (BMP_Header[OffsetPixelHeigh + 2] << 16) + (BMP_Header[OffsetPixelHeigh + 3] << 24);
//    PixelWidth = BMP_Header[OffsetPixelWidth] + (BMP_Header[OffsetPixelWidth + 1] << 8) + (BMP_Header[OffsetPixelWidth + 2] << 16) + (BMP_Header[OffsetPixelWidth + 3] << 24);
//    if (PixelHeigh > height() + y || PixelWidth > width() + x) {
//        fclose(Image);
//        return(-3);      // to big
//    }
//
//    start_data = BMP_Header[OffsetPixData] + (BMP_Header[OffsetPixData + 1] << 8) + (BMP_Header[OffsetPixData + 2] << 16) + (BMP_Header[OffsetPixData + 3] << 24);
//
//    line = (unsigned short *) malloc (2 * PixelWidth); // we need a buffer for a line
//    if (line == NULL) {
//        return(-4);         // error no memory
//    }
//
//    // the bmp lines are padded to multiple of 4 bytes
//    padd = -1;
//    do {
//        padd ++;
//    } while ((PixelWidth * 2 + padd)%4 != 0);
//
//    window(x, y,PixelWidth ,PixelHeigh);
//    wr_cmd(0x2C);  // send pixel
//    for (j = PixelHeigh - 1; j >= 0; j--) {               //Lines bottom up
//        off = j * (PixelWidth  * 2 + padd) + start_data;   // start of line
//        fseek(Image, off ,SEEK_SET);
//        fread(line,1,PixelWidth * 2,Image);       // read a line - slow 
//        for (i = 0; i < PixelWidth; i++) {        // copy pixel data to TFT
//            SPI::write(line[i] >> 8);
//            SPI::write(line[i]);
//        } 
//     }
//    _cs = 1;
//    SPI::format(8,3);
//    free (line);
//    fclose(Image);
//    WindowMax();
//    return(1);
//}

void SPI_TFT_ILI9341_ellipse(SPI_TFT_ILI9341_DriverStruct *handle, int xc, int yc, int a, int b, int color)
{
    /* e(x,y) = b^2*x^2 + a^2*y^2 - a^2*b^2 */
    //wr_reg(0x03, 0x1030);
    //SPI::write(0x28);
    //WindowMax();
    int x = 0, y = b;
    long a2 = (long)a*a, b2 = (long)b*b;
    long crit1 = -(a2/4 + a%2 + b2);
    long crit2 = -(b2/4 + b%2 + a2);
    long crit3 = -(b2/4 + b%2);
    long t = -a2*y;                         // e(x+1/2,y-1/2) - (a^2+b^2)/4
    long dxt = 2*b2*x, dyt = -2*a2*y;
    long d2xt = 2*b2, d2yt = 2*a2;
 
    while (y>=0 && x<=a) {
        SPI_TFT_ILI9341_pixel(handle, xc+x, yc+y, color);
        if (x!=0 || y!=0)
            SPI_TFT_ILI9341_pixel(handle, xc-x, yc-y, color);
        if (x!=0 && y!=0) {
            SPI_TFT_ILI9341_pixel(handle, xc+x, yc-y, color);
            SPI_TFT_ILI9341_pixel(handle, xc-x, yc+y, color);
        }
        if (t + b2*x <= crit1 ||            // e(x+1,y-1/2) <= 0
                t + a2*y <= crit3)          // e(x+1/2,y) <= 0
        {
            x++; dxt+=d2xt; t+=dxt;
        }
        else if (t - a2*y > crit2)          // e(x+1/2,y-1) > 0
        {
            y--; dyt+=d2yt; t+=dyt;
        }
        else {
            x++; dxt+=d2xt; t+=dxt;
            y--; dyt+=d2yt; t+=dyt;
        }
    }
}

//void SPI_TFT_ILI9341_fill_ellipse(SPI_TFT_ILI9341_DriverStruct *handle, int xc, int yc, int a, int b, int color)
//{
//    /* e(x,y) = b^2*x^2 + a^2*y^2 - a^2*b^2 */
//    int x = 0, y = b;
//    int rx = x, ry = y;
//    unsigned int width = 1;
//    unsigned int height = 1;
//    long a2 = (long)a*a, b2 = (long)b*b;
//    long crit1 = -(a2/4 + a%2 + b2);
//    long crit2 = -(b2/4 + b%2 + a2);
//    long crit3 = -(b2/4 + b%2);
//    long t = -a2*y;                         // e(x+1/2,y-1/2) - (a^2+b^2)/4
//    long dxt = 2*b2*x, dyt = -2*a2*y;
//    long d2xt = 2*b2, d2yt = 2*a2;
// 
//    if (b == 0) {
//        SPI_TFT_ILI9341_fillrect(handle, xc-a, yc, 2*a+1, 1, color);
//        return;
//    }
// 
//#define INCX() x++; dxt += d2xt; t += dxt
//#define INCY() y--; dyt += d2yt; t += dyt
//    while (y>=0 && x<=a) {
//        if (t + b2*x <= crit1 ||            // e(x+1,y-1/2) <= 0
//                t + a2*y <= crit3) {        // e(x+1/2,y) <= 0
//            if (height == 1)
//                ;                           // draw nothing
//            else if (ry*2+1 > (height-1)*2) {
//                SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc-ry, width, height-1, color); 
//                SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc+ry+1, width, 1-height, color);
//                ry -= height-1;
//                height = 1;
//            } else {
//                SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc-ry, width, ry*2+1, color);
//                ry -= ry;
//                height = 1;
//            }
//            INCX();
//            rx++;
//            width += 2;
//        } else if (t - a2*y > crit2) {      // e(x+1/2,y-1) > 0
//            INCY();
//            height++;
//        } else {
//            if (ry*2+1 > height*2) {
//                SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc-ry, width, height, color);
//                SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc+ry+1, width, -height, color);
//            } else {
//                SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc-ry, width, ry*2+1, color);
//            }
//            INCX();
//            INCY();
//            rx++;
//            width += 2;
//            ry -= height;
//            height = 1;
//        }
//    }
// 
//    if (ry > height) {
//        SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc-ry, width, height, color);
//        SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc+ry+1, width, -height, color);
//    } else {
//        SPI_TFT_ILI9341_fillrect(handle, xc-rx, yc-ry, width, ry*2+1, color);
//    }
//
//    x = 0; y = b;
//    width = 1;
//    a2 = (long)a*a; b2 = (long)b*b;
//    crit1 = -(a2/4 + a%2 + b2);
//    crit2 = -(b2/4 + b%2 + a2);
//    crit3 = -(b2/4 + b%2);
//    t = -a2*y; /* e(x+1/2,y-1/2) - (a^2+b^2)/4 */
//    dxt = 2*b2*x, dyt = -2*a2*y;
//    d2xt = 2*b2, d2yt = 2*a2;
//
//    while (y>=0 && x<=a) {
//        if (t + b2*x <= crit1 ||     /* e(x+1,y-1/2) <= 0 */
//            t + a2*y <= crit3) {     /* e(x+1/2,y) <= 0 */
//            INCX();
//            width += 2;
//        }
//        else if (t - a2*y > crit2) { /* e(x+1/2,y-1) > 0 */
//            hline(handle, xc-x, yc-y, width, color);
//            if (y!=0)
//                hline(handle, xc-x, yc+y, width, color);
//            INCY();
//        }
//        else {
//            hline(handle, xc-x, yc-y, width, color);
//            if (y!=0)
//                hline(handle, xc-x, yc+y, width, color);
//            INCX();
//            INCY();
//            width += 2;
//        }
//    }
//    if (b == 0)
//        hline(handle, xc-a, yc, 2*a+1, color);
//}

