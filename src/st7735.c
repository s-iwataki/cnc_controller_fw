#include "st7735.h"
#include "container_util.h"
#include "projdefs.h"
#ifdef AVR
#include <avr/pgmspace.h>
#include <util/delay.h>
#endif
#include "font.h"

#ifndef PROGMEM
#define PROGMEM
#define pgm_read_byte(x) (*(x))
#endif

#ifndef _delay_ms
#include "FreeRTOS.h"
#include "task.h"
static inline void _delay_ms(int ms){
    vTaskDelay(pdMS_TO_TICKS(ms));
}
#endif

typedef struct
{
    DIGITALOUT_t *reset;
    DIGITALOUT_t *cs;
    DIGITALOUT_t *rs;
    spi_bus_device_t device;
    spi_bus_driver_t *spi;
    graphic_apis_t graphic_api;
    uint8_t row_start;
    uint8_t col_start;
    uint8_t width;
    uint8_t height;
    uint8_t tabcolor;
} st7735_t;

static void writecommand(st7735_t *inst, uint8_t c)
{
    digitalout_reset(inst->rs);
    inst->spi->spi_do_transaction(inst->spi, &(inst->device), &c, 1, 0, 0);
}

static void writedata(st7735_t *inst, uint8_t c)
{
    digitalout_set(inst->rs);
    inst->spi->spi_do_transaction(inst->spi, &(inst->device), &c, 1, 0, 0);
}
static void writedataarray(st7735_t *inst, uint8_t *c, size_t sz)
{

    digitalout_set(inst->rs);
    inst->spi->spi_do_transaction(inst->spi, &(inst->device), c, sz, 0, 0);
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t
    Initcmd[] PROGMEM = {
        15,
        ST7735_SWRESET, DELAY, // 0 soft reset
        120,
        ST7735_SLPOUT, DELAY, // 1 exit sleep
        120,
        0x26, 1, // 2 set default gunma
        0x04,
        ST7735_FRMCTR1, 3 + DELAY, // 3 set frame rate
        0x0c, 0x14, 0x14,
        10,
        ST7735_PWCTR1, 3 + DELAY, // 4 set charge pump
        0x0c, 0x05, 0x84,
        10,
        ST7735_PWCTR2, DELAY + 1, // 5 set pwr
        0x02,
        10,
        ST7735_VMCTR1, DELAY + 1, // 6 VOMH VOML set
        0x0e,
        10,
        0xc7, 0x01, // 7@äÝč
        0x40,
        ST7735_COLMOD, 1 + DELAY, // 8 set color mode
        0x55,
        10,
        ST7735_CASET, 4, // 9 set col addr
        0x00, 0x00, 0x00, 0x7f,
        ST7735_RASET, 0x04, // 10 page addr set
        0x00, 0x00, 0x00, 0x9f,
        ST7735_MADCTL, 1, // 11 set scan dir
        0xc8,
        ST7735_GMCTRP1, 0x10, // 12 set gunma data
        0x36, 0x29, 0x12, 0x22,
        0x1c, 0x15, 0x42, 0xb7,
        0x2f, 0x13, 0x12, 0x0a,
        0x11, 0x0b, 0x06, 0x04,
        ST7735_GMCTRN1, 0x10, // 13 set gunma
        0x09, 0x16, 0x2d, 0x0d,
        0x13, 0x15, 0x40, 0x48,
        0x53, 0x0c, 0x1d, 0x25,
        0x2e, 0x34, 0x39, 0x39,
        ST7735_DISPON, DELAY, // 14 disp on
        255
        /*8,
        ST7735_SLPOUT,DELAY,//1 exit sleep
        120,
        0x26,1,				//2 set default gunma
        0x04,
        ST7735_COLMOD,1,	//3 set color mode
        0x05,
        ST7735_CASET,4,		//4 set col addr
        0x00,0x00,0x00,0x7f,
        ST7735_RASET,0x04,	//5 page addr set
        0x00,0x00,0x00,0x9f,
        ST7735_MADCTL,1,	//6 set scan dir
        0xc8,
        ST7735_DISPON,0x00,	//7 disp on
        ST7735_RAMWR,0x00	//8 ?*/
};
static const uint8_t
    Bcmd[] PROGMEM = {             // Initialization commands for 7735B screens
        18,                        // 18 commands in list:
        ST7735_SWRESET, DELAY,     //  1: Software reset, no args, w/delay
        50,                        //     50 ms delay
        ST7735_SLPOUT, DELAY,      //  2: Out of sleep mode, no args, w/delay
        255,                       //     255 = 500 ms delay
        ST7735_COLMOD, 1 + DELAY,  //  3: Set color mode, 1 arg + delay:
        0x05,                      //     16-bit color
        10,                        //     10 ms delay
        ST7735_FRMCTR1, 3 + DELAY, //  4: Frame rate control, 3 args + delay:
        0x00,                      //     fastest refresh
        0x06,                      //     6 lines front porch
        0x03,                      //     3 lines back porch
        10,                        //     10 ms delay
        ST7735_MADCTL, 1,          //  5: Memory access ctrl (directions), 1 arg:
        0x08,                      //     Row addr/col addr, bottom to top refresh
        ST7735_DISSET5, 2,         //  6: Display settings #5, 2 args, no delay:
        0x15,                      //     1 clk cycle nonoverlap, 2 cycle gate
                                   //     rise, 3 cycle osc equalize
        0x02,                      //     Fix on VTL
        ST7735_INVCTR, 1,          //  7: Display inversion control, 1 arg:
        0x0,                       //     Line inversion
        ST7735_PWCTR1, 2 + DELAY,  //  8: Power control, 2 args + delay:
        0x02,                      //     GVDD = 4.7V
        0x70,                      //     1.0uA
        10,                        //     10 ms delay
        ST7735_PWCTR2, 1,          //  9: Power control, 1 arg, no delay:
        0x05,                      //     VGH = 14.7V, VGL = -7.35V
        ST7735_PWCTR3, 2,          // 10: Power control, 2 args, no delay:
        0x01,                      //     Opamp current small
        0x02,                      //     Boost frequency
        ST7735_VMCTR1, 2 + DELAY,  // 11: Power control, 2 args + delay:
        0x3C,                      //     VCOMH = 4V
        0x38,                      //     VCOML = -1.1V
        10,                        //     10 ms delay
        ST7735_PWCTR6, 2,          // 12: Power control, 2 args, no delay:
        0x11, 0x15,
        ST7735_GMCTRP1, 16,     // 13: Magical unicorn dust, 16 args, no delay:
        0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
        0x21, 0x1B, 0x13, 0x19, //      these config values represent)
        0x17, 0x15, 0x1E, 0x2B,
        0x04, 0x05, 0x02, 0x0E,
        ST7735_GMCTRN1, 16 + DELAY, // 14: Sparkles and rainbows, 16 args + delay:
        0x0B, 0x14, 0x08, 0x1E,     //     (ditto)
        0x22, 0x1D, 0x18, 0x1E,
        0x1B, 0x1A, 0x24, 0x2B,
        0x06, 0x06, 0x02, 0x0F,
        10,                   //     10 ms delay
        ST7735_CASET, 4,      // 15: Column addr set, 4 args, no delay:
        0x00, 0x02,           //     XSTART = 2
        0x00, 0x81,           //     XEND = 129
        ST7735_RASET, 4,      // 16: Row addr set, 4 args, no delay:
        0x00, 0x02,           //     XSTART = 1
        0x00, 0x81,           //     XEND = 160
        ST7735_NORON, DELAY,  // 17: Normal display on, no args, w/delay
        10,                   //     10 ms delay
        ST7735_DISPON, DELAY, // 18: Main screen turn on, no args, w/delay
        255},                 //     255 = 500 ms delay

    Rcmd1[] PROGMEM = {        // Init for 7735R, part 1 (red or green tab)
        15,                    // 15 commands in list:
        ST7735_SWRESET, DELAY, //  1: Software reset, 0 args, w/delay
        150,                   //     150 ms delay
        ST7735_SLPOUT, DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                   //     500 ms delay
        ST7735_FRMCTR1, 3,     //  3: Frame rate ctrl - normal mode, 3 args:
        0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR2, 3,     //  4: Frame rate control - idle mode, 3 args:
        0x01, 0x2C, 0x2D,      //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR3, 6,     //  5: Frame rate ctrl - partial mode, 6 args:
        0x01, 0x2C, 0x2D,      //     Dot inversion mode
        0x01, 0x2C, 0x2D,      //     Line inversion mode
        ST7735_INVCTR, 1,      //  6: Display inversion ctrl, 1 arg, no delay:
        0x07,                  //     No inversion
        ST7735_PWCTR1, 3,      //  7: Power control, 3 args, no delay:
        0xA2,
        0x02,                         //     -4.6V
        0x84,                         //     AUTO mode
        ST7735_PWCTR2, 1,             //  8: Power control, 1 arg, no delay:
        0xC5,                         //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
        ST7735_PWCTR3, 2,             //  9: Power control, 2 args, no delay:
        0x0A,                         //     Opamp current small
        0x00,                         //     Boost frequency
        ST7735_PWCTR4, 2,             // 10: Power control, 2 args, no delay:
        0x8A,                         //     BCLK/2, Opamp current small & Medium low
        0x2A, ST7735_PWCTR5, 2,       // 11: Power control, 2 args, no delay:
        0x8A, 0xEE, ST7735_VMCTR1, 1, // 12: Power control, 1 arg, no delay:
        0x0E, ST7735_INVOFF, 0,       // 13: Don't invert display, no args, no delay
        ST7735_MADCTL, 1,             // 14: Memory access control (directions), 1 arg:
        0xC8,                         //     row addr/col addr, bottom to top refresh
        ST7735_COLMOD, 1,             // 15: set color mode, 1 arg, no delay:
        0x05},                        //     16-bit color

    Rcmd2green[] PROGMEM = { // Init for 7735R, part 2 (green tab only)
        2,                   //  2 commands in list:
        ST7735_CASET, 4,     //  1: Column addr set, 4 args, no delay:
        0x00, 0x02,          //     XSTART = 0
        0x00, 0x7F + 0x02,   //     XEND = 127
        ST7735_RASET, 4,     //  2: Row addr set, 4 args, no delay:
        0x00, 0x01,          //     XSTART = 0
        0x00, 0x9F + 0x01},  //     XEND = 159
    Rcmd2red[] PROGMEM = {   // Init for 7735R, part 2 (red tab only)
        2,                   //  2 commands in list:
        ST7735_CASET, 4,     //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,          //     XSTART = 0
        0x00, 0x7F,          //     XEND = 127
        ST7735_RASET, 4,     //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,          //     XSTART = 0
        0x00, 0x9F},         //     XEND = 159

    Rcmd2green144[] PROGMEM = { // Init for 7735R, part 2 (green 1.44 tab)
        2,                      //  2 commands in list:
        ST7735_CASET, 4,        //  1: Column addr set, 4 args, no delay:
        0x00, 0x00,             //     XSTART = 0
        0x00, 0x7F,             //     XEND = 127
        ST7735_RASET, 4,        //  2: Row addr set, 4 args, no delay:
        0x00, 0x00,             //     XSTART = 0
        0x00, 0x7F},            //     XEND = 127

    Rcmd3[] PROGMEM = {                                                                                                      // Init for 7735R, part 3 (red or green tab)
        4,                                                                                                                   //  4 commands in list:
        ST7735_GMCTRP1, 16,                                                                                                  //  1: Magical unicorn dust, 16 args, no delay:
        0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, ST7735_GMCTRN1, 16,  //  2: Sparkles and rainbows, 16 args, no delay:
        0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, ST7735_NORON, DELAY, //  3: Normal display on, no args, w/delay
        10,                                                                                                                  //     10 ms delay
        ST7735_DISPON, DELAY,                                                                                                //  4: Main screen turn on, no args w/delay
        100};                                                                                                                //     100 ms delay

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
static void send_command_list(st7735_t *inst, const uint8_t *addr)
{

    uint8_t num_commands, num_args;
    uint16_t ms;
    int index = 0;
    num_commands = pgm_read_byte(&addr[index++]); // Number of commands to follow
    while (num_commands--)
    { // For each command...
        inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
        writecommand(inst, pgm_read_byte(&addr[index++])); //   Read, issue command
        num_args = pgm_read_byte(&addr[index++]);          //   Number of args to follow
        ms = num_args & DELAY;                             //   If hibit set, delay follows args
        num_args &= ~DELAY;                                //   Mask out delay bit
        while (num_args--)
        {                                                   //   For each argument...
            writedata(inst, pgm_read_byte(&addr[index++])); //     Read, issue argument
        }
        inst->spi->spi_release_lock(inst->spi, &(inst->device));

        if (ms)
        {
            ms = pgm_read_byte(&addr[index++]); // Read post-command delay time (ms)
            if (ms == 255)
                ms = 500; // If 255, delay for 500 ms
            _delay_ms(ms);
        }
    }
}
static void lut_init(st7735_t *inst)
{
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    writecommand(inst, 0x2d);
    int lut;
    for (lut = 0; lut < 32; lut++)
    {
        writedata(inst, lut);
    }
    for (lut = 0; lut < 64; lut++)
    {
        writedata(inst, lut);
    }
    for (lut = 0; lut < 32; lut++)
    {
        writedata(inst, lut);
    }
    writecommand(inst, ST7735_RAMWR);
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}

// Initialization code common to both 'B' and 'R' type displays
static void common_init(st7735_t *inst, const uint8_t *cmdList)
{
    inst->device.config.byte_order = SPI_MSB_FISRT;
    inst->device.config.mode = SPI_MODE0;
    inst->width = ST7735_TFTWIDTH;
    inst->height = ST7735_TFTHEIGHT_18;
    // toggle RST low to reset; CS low so it'll listen to us
    digitalout_reset(inst->cs);
    digitalout_reset(inst->reset);
    _delay_ms(120);
    digitalout_set(inst->reset);
    digitalout_set(inst->cs);

    if (cmdList)
        send_command_list(inst, cmdList);
}

// Initialization for ST7735B screens
static void init_b(st7735_t *inst)
{
    common_init(inst, Bcmd);
}
static void init(st7735_t *inst)
{
    common_init(inst, Initcmd);
    lut_init(inst);
}

// Initialization for ST7735R screens (green or red tabs)
static void init_r(st7735_t *inst, uint8_t options)
{
    common_init(inst, Rcmd1);
    if (options == INITR_GREENTAB)
    {
        send_command_list(inst, Rcmd2green);
        inst->col_start = 2;
        inst->row_start = 1;
    }
    else if (options == INITR_144GREENTAB)
    {
        inst->height = ST7735_TFTHEIGHT_144;
        send_command_list(inst, Rcmd2green144);
        inst->col_start = 2;
        inst->col_start = 3;
    }
    else
    {
        // colstart, rowstart left at default '0' values
        send_command_list(inst, Rcmd2red);
        inst->row_start = 0;
        inst->col_start = 0;
    }
    send_command_list(inst, Rcmd3);

    // if black, change MADCTL color filter
    if (options == INITR_BLACKTAB)
    {
        inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
        writecommand(inst, ST7735_MADCTL);
        writedata(inst, 0xc0);
        inst->spi->spi_release_lock(inst->spi, &(inst->device));
    }

    inst->tabcolor = options;
}

static void set_addr_window(st7735_t *inst, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t data[4];
    writecommand(inst, ST7735_CASET); // Column addr set
    data[0] = 0x00;
    data[1] = x0 + inst->col_start; // XSTART
    data[2] = 0x00;
    data[3] = x1 + inst->col_start; // XEND
    writedataarray(inst, data, 4);

    writecommand(inst, ST7735_RASET); // Row addr set
    data[0] = 0x00;
    data[1] = y0 + inst->row_start; // XSTART
    data[2] = 0x00;
    data[3] = y1 + inst->row_start; // XEND
    writedataarray(inst, data, 4);

    writecommand(inst, ST7735_RAMWR); // write to RAM
}

static void ordering(uint8_t *x1, uint8_t *x2)
{
    if (*x1 > *x2)
    {
        uint8_t temp = *x1;
        *x1 = *x2;
        *x2 = temp;
    }
}

static void draw_hline(const struct graphic_apis_s *instance, uint8_t x1, uint8_t y1, uint8_t x2, uint16_t color)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    ordering(&x1, &x2);
    set_addr_window(inst, x1, y1, x2, y1);
    uint8_t data[2];
    data[0] = (color >> 8) & 0xff;
    data[1] = color & 0xff;
    for (int8_t i = 0; i < x2 - x1 + 1; i++)
    {
        writedataarray(inst, data, 2);
    }
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}
static void draw_vline(const struct graphic_apis_s *instance, uint8_t x1, uint8_t y1, uint8_t y2, uint16_t color)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    ordering(&y1, &y2);
    set_addr_window(inst, x1, y1, x1, y2);
    uint8_t data[2];
    data[0] = (color >> 8) & 0xff;
    data[1] = color & 0xff;
    for (int8_t i = 0; i < y2 - y1 + 1; i++)
    {
        writedataarray(inst, data, 2);
    }
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}
static void draw_line(const struct graphic_apis_s *instance, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
}
static void fill_rect_raw(st7735_t *inst, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    uint8_t dx = x2 - x1 + 1;
    uint8_t dy = y2 - y1 + 1;
    uint16_t area = dx * dy;
    set_addr_window(inst, x1, y1, x2, y2);
    uint8_t data[2];
    data[0] = (color >> 8) & 0xff;
    data[1] = color & 0xff;
    for (size_t i = 0; i < area; i++)
    {
        writedataarray(inst, data, 2);
    }
}
static void fill_rect(const struct graphic_apis_s *instance, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    ordering(&x1, &x2);
    ordering(&y1, &y2);
    fill_rect_raw(inst, x1, y1, x2, y2, color);
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}
static void draw_char(const struct graphic_apis_s *instance, uint8_t x, uint8_t y, uint8_t size, uint8_t c, uint16_t text_color, uint16_t background_color)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    for (int8_t i = 0; i < 6; i++)
    {
        uint8_t line;
        if (i < 5)
        {
            line = pgm_read_byte(font + (c * 5) + i);
        }
        else
        {
            line = 0x0;
        }
        for (int8_t j = 0; j < 8; j++, line >>= 1)
        {
            if (line & 0x1)
            {

                uint8_t x1 = x + (i * size);
                uint8_t y1 = y + (j * size);
                fill_rect_raw(inst, x1, y1, x1 + size - 1, y1 + size - 1, text_color);
            }
            else if (background_color != text_color)
            {

                uint8_t x1 = x + (i * size);
                uint8_t y1 = y + (j * size);
                fill_rect_raw(inst, x1, y1, x1 + size - 1, y1 + size - 1, background_color);
            }
        }
    }
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}

static void fill_screen(const struct graphic_apis_s *instance, uint16_t color)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    fill_rect(instance, 0, 0, inst->width, inst->height, color);
}
static void draw_bmp_mono(const struct graphic_apis_s *instance, uint8_t x1, uint8_t y1, uint8_t w, uint8_t h, const uint8_t *data, uint16_t dot_color, uint16_t background_color)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));

    for (uint8_t i = 0; i < h / 8; i++)
    {
        uint8_t y0 = y1 + 8 * i;
        for (int8_t j = 0; j < w; j++)
        {
            uint8_t line = data[j + w * i];

            for (int8_t k = 0; k < 8; k++, line >>= 1)
            {
                if (line & 0x1)
                {
                    fill_rect_raw(inst, x1 + j, y0 + k, x1 + j, y0 + k, dot_color);
                }
                else if (background_color != dot_color)
                {
                    fill_rect_raw(inst, x1 + j, y0 + k, x1 + j, y0 + k, background_color);
                }
            }
        }
    }
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}
static void draw_bmp(const struct graphic_apis_s *instance, uint8_t x1, uint8_t y1, uint8_t w, uint8_t h, const uint16_t *data)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    set_addr_window(inst, x1, y1, x1 + w - 1, y1 + h - 1);
    uint16_t area = w * h;
    uint8_t pixeldata[2];
    for (size_t i = 0; i < area; i++)
    {
        pixeldata[0] = (data[i] >> 8) & 0xff;
        pixeldata[1] = data[i] & 0xff;
        writedataarray(inst, pixeldata, 2);
    }
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04
static void set_rotation(const struct graphic_apis_s *instance, uint8_t m)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    inst->spi->spi_aquire_lock(inst->spi, &(inst->device));
    writecommand(inst, ST7735_MADCTL);
    uint8_t rotation = m % 4; // can't be higher than 3
    switch (rotation)
    {
    case 0:
        if (inst->tabcolor == INITR_BLACKTAB)
        {
            writedata(inst, MADCTL_MX | MADCTL_MY | MADCTL_RGB);
        }
        else
        {
            writedata(inst, MADCTL_MX | MADCTL_MY | MADCTL_BGR);
        }
        inst->width = ST7735_TFTWIDTH;

        if (inst->tabcolor == INITR_144GREENTAB)
            inst->height = ST7735_TFTHEIGHT_144;
        else
            inst->height = ST7735_TFTHEIGHT_18;

        break;
    case 1:
        if (inst->tabcolor == INITR_BLACKTAB)
        {
            writedata(inst, MADCTL_MY | MADCTL_MV | MADCTL_RGB);
        }
        else
        {
            writedata(inst, MADCTL_MY | MADCTL_MV | MADCTL_BGR);
        }

        if (inst->tabcolor == INITR_144GREENTAB)
            inst->width = ST7735_TFTHEIGHT_144;
        else
            inst->width = ST7735_TFTHEIGHT_18;

        inst->height = ST7735_TFTWIDTH;
        break;
    case 2:
        if (inst->tabcolor == INITR_BLACKTAB)
        {
            writedata(inst, MADCTL_RGB);
        }
        else
        {
            writedata(inst, MADCTL_BGR);
        }
        inst->width = ST7735_TFTWIDTH;
        if (inst->tabcolor == INITR_144GREENTAB)
            inst->height = ST7735_TFTHEIGHT_144;
        else
            inst->height = ST7735_TFTHEIGHT_18;

        break;
    case 3:
        if (inst->tabcolor == INITR_BLACKTAB)
        {
            writedata(inst, MADCTL_MX | MADCTL_MV | MADCTL_RGB);
        }
        else
        {
            writedata(inst, MADCTL_MX | MADCTL_MV | MADCTL_BGR);
        }
        if (inst->tabcolor == INITR_144GREENTAB)
            inst->width = ST7735_TFTHEIGHT_144;
        else
            inst->width = ST7735_TFTHEIGHT_18;

        inst->height = ST7735_TFTWIDTH;
        break;
    }
    inst->spi->spi_release_lock(inst->spi, &(inst->device));
}

static uint8_t get_width(const struct graphic_apis_t *instance)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    return inst->width;
}

static uint8_t get_height(const struct graphic_apis_t *instance)
{
    st7735_t *inst = container_of(instance, st7735_t, graphic_api);
    return inst->height;
}

static void on_bus_aquired(struct spi_bus_device_s *d)
{
    st7735_t *inst = container_of(d, st7735_t, device);
    digitalout_reset(inst->cs);
}
static void on_bus_released(struct spi_bus_device_s *d)
{
    st7735_t *inst = container_of(d, st7735_t, device);
    digitalout_set(inst->cs);
}
static void on_transfer_completed(struct spi_bus_device_s *d)
{
}
static st7735_t instance;
graphic_apis_t *st7735_init(ST7735_DISPLAY_TYPE_t type, spi_bus_driver_t *bus, DIGITALOUT_t *reset, DIGITALOUT_t *cs, DIGITALOUT_t *rs)
{
    instance.cs = cs;
    instance.reset = reset;
    instance.rs = rs;
    instance.device.config.byte_order = SPI_MSB_FISRT;
    instance.device.config.mode = SPI_MODE0;
    instance.device.on_bus_aquired = on_bus_aquired;
    instance.device.on_bus_released = on_bus_released;
    instance.device.on_transfer_completed = on_transfer_completed;
    instance.graphic_api.draw_bmp = draw_bmp;
    instance.graphic_api.draw_bmp_mono = draw_bmp_mono;
    instance.graphic_api.draw_char = draw_char;
    instance.graphic_api.draw_hline = draw_hline;
    instance.graphic_api.draw_line = draw_line;
    instance.graphic_api.draw_vline = draw_vline;
    instance.graphic_api.fill_rect = fill_rect;
    instance.graphic_api.fill_screen = fill_screen;
    instance.graphic_api.set_rotation = set_rotation;
    instance.graphic_api.get_height = get_height;
    instance.graphic_api.get_width = get_width;
    instance.spi = bus;
    switch (type)
    {
    case ST7735R_18GREENTAB:
        init_r(&instance, INITR_18GREENTAB);
        break;
    case ST7735R_18REDTAB:
        init_r(&instance, INITR_18REDTAB);
        break;
    case ST7735R_18BLACKTAB:
        init_r(&instance, INITR_18BLACKTAB);
        break;
    case ST7735RR_144GREENTAB:
        init_r(&instance, INITR_144GREENTAB);
        break;
    case ST7735B:
        init_b(&instance);
        instance.tabcolor = 0;
    case ST7735:
        init(&instance);
        instance.tabcolor = 0;
    default:
        break;
    }
    return &(instance.graphic_api);
}
