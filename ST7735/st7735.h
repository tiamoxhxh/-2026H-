/**
 * @file st7735.h
 * @brief ST7735 128x160 TFT LCD 驱动接口
 */
#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>

#define ST7735_WIDTH   (128U)
#define ST7735_HEIGHT  (160U)

#define ST7735_COLOR_BLACK    (0x0000U)
#define ST7735_COLOR_BLUE     (0x001FU)
#define ST7735_COLOR_RED      (0xF800U)
#define ST7735_COLOR_GREEN    (0x07E0U)
#define ST7735_COLOR_CYAN     (0x07FFU)
#define ST7735_COLOR_YELLOW   (0xFFE0U)
#define ST7735_COLOR_WHITE    (0xFFFFU)

void ST7735_Init(void);
void ST7735_FillScreen(uint16_t color);
void ST7735_FillRect(uint16_t x, uint16_t y, uint16_t width,
    uint16_t height, uint16_t color);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_DrawChar(uint16_t x, uint16_t y, char character,
    uint16_t foreground, uint16_t background);
void ST7735_DrawString(uint16_t x, uint16_t y, const char *text,
    uint16_t foreground, uint16_t background);
void ST7735_DrawUInt(uint16_t x, uint16_t y, uint32_t value,
    uint16_t foreground, uint16_t background);
void ST7735_DrawInt(uint16_t x, uint16_t y, int32_t value,
    uint16_t foreground, uint16_t background);
void ST7735_DrawChinese16(uint16_t x, uint16_t y,
    const uint8_t bitmap[32], uint16_t foreground, uint16_t background);
void ST7735_DrawChineseString(uint16_t x, uint16_t y, const char *utf8_text,
    uint16_t foreground, uint16_t background);

#endif /* ST7735_H */
