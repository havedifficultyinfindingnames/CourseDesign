#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// false for vertical
// true for horitonal
typedef struct {
    bool direction;
} LCD_settings_t;
extern LCD_settings_t LCD_settings;

typedef enum {
    L2R_U2D = 0,
    L2R_D2U = 4,
    R2L_U2D = 2,
    R2L_D2U = 6,
    U2D_L2R = 1,
    U2D_R2L = 3,
    D2U_L2R = 5,
    D2U_R2L = 7,
} direction_t;

enum {
    COLOR_WHITE         = 0xffff,
    COLOR_BLACK         = 0x0000,
    COLOR_BLUE          = 0x001f,
    COLOR_BRED          = 0xf81f,
    COLOR_GRED          = 0xffe0,
    COLOR_GBLUE         = 0x07ff,
    COLOR_RED           = 0xf800,
    COLOR_GREEN         = 0x07e0,
    COLOR_YELLOW        = 0xffe0,
    COLOR_LIGHTGREEN    = 0x841f,
};

typedef enum {
    FONT_SMALL,
    FONT_MEDIUM,
    FONT_LARGE,
    FONT_VERY_LARGE,
} font_size_t;

void LCD_set_register(uint16_t value);
void LCD_set_ram(uint16_t value);
uint16_t LCD_read_register(void);
uint16_t LCD_read_ram(void);
void LCD_write(uint16_t reg, uint16_t value);
uint16_t LCD_read(uint16_t reg);

void LCD_init(void);

void LCD_display_dir(bool direction);
void LCD_set_cursor(uint16_t x, uint16_t y);
void LCD_clear(uint16_t color);

void LCD_draw_point(uint16_t x, uint16_t y, uint16_t color);
void LCD_display_char(uint16_t x, uint16_t y, uint8_t character, font_size_t font_size, bool transparent);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */
