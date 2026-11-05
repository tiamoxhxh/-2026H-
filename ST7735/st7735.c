/**
 * @file st7735.c
 * @brief MSPM0G3507 SPI 驱动 ST7735 128x160 TFT LCD (轮询模式)
 */
#include "st7735.h"

#include <stdbool.h>
#include <stddef.h>

#include "clock.h"
#include "oledfont.h"
#include "ti_msp_dl_config.h"

#define ST7735_CMD_SWRESET  (0x01U)
#define ST7735_CMD_SLPOUT   (0x11U)
#define ST7735_CMD_NORON    (0x13U)
#define ST7735_CMD_INVOFF   (0x20U)
#define ST7735_CMD_DISPON   (0x29U)
#define ST7735_CMD_CASET    (0x2AU)
#define ST7735_CMD_RASET    (0x2BU)
#define ST7735_CMD_RAMWR    (0x2CU)
#define ST7735_CMD_MADCTL   (0x36U)
#define ST7735_CMD_COLMOD   (0x3AU)
#define ST7735_CMD_FRMCTR1  (0xB1U)
#define ST7735_CMD_FRMCTR2  (0xB2U)
#define ST7735_CMD_FRMCTR3  (0xB3U)
#define ST7735_CMD_INVCTR   (0xB4U)
#define ST7735_CMD_PWCTR1   (0xC0U)
#define ST7735_CMD_PWCTR2   (0xC1U)
#define ST7735_CMD_PWCTR3   (0xC2U)
#define ST7735_CMD_PWCTR4   (0xC3U)
#define ST7735_CMD_PWCTR5   (0xC4U)
#define ST7735_CMD_VMCTR1   (0xC5U)
#define ST7735_CMD_GMCTRP1  (0xE0U)
#define ST7735_CMD_GMCTRN1  (0xE1U)

#define ST7735_MADCTL_PORTRAIT_RGB  (0xC0U)
#define ST7735_COLMOD_RGB565        (0x05U)
#define ST7735_X_OFFSET             (2U)
#define ST7735_Y_OFFSET             (1U)

#define LCD_RES_PIN  (GPIO_LCD_PIN_LCD_RES_PIN)
#define LCD_DC_PIN   (GPIO_LCD_PIN_LCD_DC_PIN)
#define LCD_CS_PIN   (GPIO_LCD_PIN_LCD_CS_PIN)
#define LCD_BLK_PIN  (GPIO_LCD_PIN_LCD_BLK_PIN)

static void spiWriteByte(uint8_t data)
{
    while (DL_SPI_isTXFIFOFull(SPI_LCD_INST)) {}
    DL_SPI_transmitData8(SPI_LCD_INST, data);
}

static void spiWaitIdle(void)
{
    while (DL_SPI_isBusy(SPI_LCD_INST)) {}
}

static void writeCommand(uint8_t command)
{
    DL_GPIO_clearPins(GPIO_LCD_PORT, LCD_CS_PIN | LCD_DC_PIN);
    spiWriteByte(command);
    spiWaitIdle();
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_CS_PIN);
}

static void writeData(const uint8_t *p_data, size_t length)
{
    size_t i;
    if ((p_data == NULL) || (length == 0U)) return;
    DL_GPIO_clearPins(GPIO_LCD_PORT, LCD_CS_PIN);
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_DC_PIN);
    for (i = 0U; i < length; i++) spiWriteByte(p_data[i]);
    spiWaitIdle();
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_CS_PIN);
}

static void writeCommandData(uint8_t command, const uint8_t *p_data, size_t length)
{
    writeCommand(command);
    writeData(p_data, length);
}

static void setAddressWindow(uint16_t x_start, uint16_t y_start,
    uint16_t x_end, uint16_t y_end)
{
    uint8_t a[4];
    x_start += ST7735_X_OFFSET; x_end += ST7735_X_OFFSET;
    y_start += ST7735_Y_OFFSET; y_end += ST7735_Y_OFFSET;
    a[0] = (uint8_t)(x_start >> 8U); a[1] = (uint8_t)x_start;
    a[2] = (uint8_t)(x_end >> 8U);   a[3] = (uint8_t)x_end;
    writeCommandData(ST7735_CMD_CASET, a, 4);
    a[0] = (uint8_t)(y_start >> 8U); a[1] = (uint8_t)y_start;
    a[2] = (uint8_t)(y_end >> 8U);   a[3] = (uint8_t)y_end;
    writeCommandData(ST7735_CMD_RASET, a, 4);
    writeCommand(ST7735_CMD_RAMWR);
}

void ST7735_Init(void)
{
    DL_GPIO_clearPins(GPIO_LCD_PORT, LCD_RES_PIN);
    delay_ms(1);
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_RES_PIN);
    delay_ms(120);

    writeCommand(ST7735_CMD_SWRESET); delay_ms(150);
    writeCommand(ST7735_CMD_SLPOUT);  delay_ms(200);
    writeCommand(ST7735_CMD_FRMCTR1); { uint8_t d[] = {0x01,0x2C,0x2D}; writeData(d,3); }
    writeCommand(ST7735_CMD_FRMCTR2); { uint8_t d[] = {0x01,0x2C,0x2D}; writeData(d,3); }
    writeCommand(ST7735_CMD_FRMCTR3); { uint8_t d[] = {0x01,0x2C,0x2D,0x01,0x2C,0x2D}; writeData(d,6); }
    writeCommand(ST7735_CMD_INVCTR);  { uint8_t d[] = {0x07}; writeData(d,1); }
    writeCommand(ST7735_CMD_PWCTR1);  { uint8_t d[] = {0xA2,0x02,0x84}; writeData(d,3); }
    writeCommand(ST7735_CMD_PWCTR2);  { uint8_t d[] = {0xC5}; writeData(d,1); }
    writeCommand(ST7735_CMD_PWCTR3);  { uint8_t d[] = {0x0A,0x00}; writeData(d,2); }
    writeCommand(ST7735_CMD_PWCTR4);  { uint8_t d[] = {0x8A,0x2A}; writeData(d,2); }
    writeCommand(ST7735_CMD_PWCTR5);  { uint8_t d[] = {0x8A,0xEE}; writeData(d,2); }
    writeCommand(ST7735_CMD_VMCTR1);  { uint8_t d[] = {0x0E}; writeData(d,1); }
    writeCommand(ST7735_CMD_INVOFF);
    writeCommand(ST7735_CMD_MADCTL);  { uint8_t d[] = {ST7735_MADCTL_PORTRAIT_RGB}; writeData(d,1); }
    writeCommand(ST7735_CMD_COLMOD);  { uint8_t d[] = {ST7735_COLMOD_RGB565}; writeData(d,1); }
    writeCommand(ST7735_CMD_CASET);   { uint8_t d[] = {0x00,0x02,0x00,0x81}; writeData(d,4); }
    writeCommand(ST7735_CMD_RASET);   { uint8_t d[] = {0x00,0x01,0x00,0xA0}; writeData(d,4); }
    writeCommand(ST7735_CMD_GMCTRP1); { uint8_t d[] = {0x0F,0x1A,0x0F,0x18,0x2F,0x28,0x20,0x22,0x1F,0x1B,0x23,0x37,0x00,0x07,0x02,0x10}; writeData(d,16); }
    writeCommand(ST7735_CMD_GMCTRN1); { uint8_t d[] = {0x0F,0x1B,0x0F,0x17,0x33,0x2C,0x29,0x2E,0x30,0x30,0x39,0x3F,0x00,0x07,0x03,0x10}; writeData(d,16); }
    writeCommand(ST7735_CMD_NORON);   delay_ms(10);
    writeCommand(ST7735_CMD_DISPON);  delay_ms(100);

    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_BLK_PIN);
}

void ST7735_FillScreen(uint16_t color)
{
    ST7735_FillRect(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

void ST7735_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint16_t i, n = (uint16_t)(w * h);
    uint8_t hi = (uint8_t)(color >> 8U), lo = (uint8_t)color;
    setAddressWindow(x, y, (uint16_t)(x + w - 1U), (uint16_t)(y + h - 1U));
    DL_GPIO_clearPins(GPIO_LCD_PORT, LCD_CS_PIN);
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_DC_PIN);
    for (i = 0U; i < n; i++) { spiWriteByte(hi); spiWriteByte(lo); }
    spiWaitIdle();
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_CS_PIN);
}

void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t d[] = {(uint8_t)(color >> 8U), (uint8_t)color};
    setAddressWindow(x, y, x, y);
    writeData(d, 2);
}

void ST7735_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg)
{
    uint8_t i, j, line;
    uint8_t buf[12 * 16 * 2];
    uint16_t idx = 0;
    if (ch < ' ' || ch > '~') ch = ' ';
    for (i = 0; i < 12; i++) {
        line = asc2_1206[(uint8_t)(ch - ' ')][i];
        for (j = 0; j < 16; j++) {
            uint16_t c = (line & (1U << (15U - j))) ? fg : bg;
            buf[idx++] = (uint8_t)(c >> 8U);
            buf[idx++] = (uint8_t)c;
        }
    }
    setAddressWindow(x, y, (uint16_t)(x + 11U), (uint16_t)(y + 15U));
    DL_GPIO_clearPins(GPIO_LCD_PORT, LCD_CS_PIN);
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_DC_PIN);
    for (i = 0; i < idx; i++) spiWriteByte(buf[i]);
    spiWaitIdle();
    DL_GPIO_setPins(GPIO_LCD_PORT, LCD_CS_PIN);
}

void ST7735_DrawString(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg)
{
    while (*text) {
        if (x + 12 > ST7735_WIDTH) { x = 0; y += 16; }
        ST7735_DrawChar(x, y, *text++, fg, bg);
        x += 12;
    }
}

void ST7735_DrawUInt(uint16_t x, uint16_t y, uint32_t v, uint16_t fg, uint16_t bg)
{
    char buf[12]; int i = 0;
    do { buf[i++] = (char)('0' + (v % 10U)); v /= 10U; } while (v && i < 11);
    while (i--) ST7735_DrawChar(x + (uint16_t)(i * 12), y, buf[i], fg, bg);
}

void ST7735_DrawInt(uint16_t x, uint16_t y, int32_t v, uint16_t fg, uint16_t bg)
{
    if (v < 0) { ST7735_DrawChar(x, y, '-', fg, bg); x += 12; v = -v; }
    ST7735_DrawUInt(x, y, (uint32_t)v, fg, bg);
}