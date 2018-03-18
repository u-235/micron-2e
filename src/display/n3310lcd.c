/**
 * \file
 * \brief Драйвер монохромного дисплея.
 * \author Sylvain Bissonnette
 * \author Aheir, aheir@radiokot.ru
 * \author Nick Egorrov
 * \date 2003-2018
 * \copyright GNU Public License 3
 * \bug В функции LcdChr() возможен выход за пределы буфера.
 * \todo Функцию LcdImage() следует доработать для вывода картинки различного
 * размера и с произвольной координаты экрана.
 * \attention Данный драйвер использует буфер в оперативной памяти размером
 * около 500 байт.
 * \details
 */

#define LCD_MONO_INSIDE_

#include "n3310lcd.h"
#include "pcd8544.h"
#include "target.h"
#include "chip.h"

/*************************************************************
 *      Параметры дисплея
 *************************************************************/

#ifndef LCD_DIRECTION
#define LCD_DIRECTION FUNC_DIR_VERT
#endif

#ifndef LCD_TEMPERATURE
#define LCD_TEMPERATURE 3u
#endif

#ifndef LCD_BIAS
#define LCD_BIAS 3u
#endif

#ifndef LCD_CONTRAST
#define LCD_CONTRAST 55u
#endif

/*#ifndef LCD_NOKIA3310_CHINA_SHIFT_Y
 *#define LCD_NOKIA3310_CHINA_SHIFT_Y 5
 *#endif
 */

/*************************************************************
 *      Variables
 *************************************************************/

static char ucBuff[LCD_CACHSIZE];
static lcd_ind_t buffIndex;
static char fPowerDown = 0;

/*************************************************************
 *      Level 0 : display depends function
 *************************************************************/

static void ChipOn()
{
        _mode_cmd();
        LcdSend(MAKE_FUNC(FUNC_SET_EXTEND | LCD_DIRECTION | FUNC_PWR_ACTIVE));
}

static void ChipOff()
{
        _mode_cmd();
        LcdSend(MAKE_FUNC(FUNC_SET_BASIC | LCD_DIRECTION | FUNC_PWR_DOWN));
}

static void ChipInit()
{
        delay_ms(1);
        _reset_down();
        delay_ms(110);
        _reset_up();

        ChipOn();
        LcdSend(EMAKE_TEMP(LCD_TEMPERATURE));
        LcdSend(EMAKE_BIAS(LCD_BIAS));
        LcdSend(EMAKE_CONTR(LCD_CONTRAST));
        /*
         *LcdSend(CMAKE_SPI_MODE(SPI_MODE_MSB));
         *LcdSend(CMAKE_SHIFT(LCD_NOKIA3310_CHINA_SHIFT_Y));
         */
        LcdSend(MAKE_FUNC(FUNC_SET_BASIC | LCD_DIRECTION | FUNC_PWR_ACTIVE));
        LcdMode(LCD_MODE_NORMAL);
        LcdClear();
}

extern void LcdContrast(unsigned char contrast)
{
        if (contrast > 0x7F) {
                return;
        }
        _mode_cmd();
        LcdSend(MAKE_FUNC(FUNC_SET_EXTEND | LCD_DIRECTION | FUNC_PWR_ACTIVE));
        LcdSend(EMAKE_CONTR(contrast));
        LcdSend(MAKE_FUNC(FUNC_SET_BASIC | LCD_DIRECTION | FUNC_PWR_ACTIVE));
}

extern void LcdMode(unsigned char mode)
{
        if (mode > 3) {
                return;
        }
        _mode_cmd();
        LcdSend(MAKE_MODE(mode));
}

/*************************************************************
 *      Level 1 : power management
 *************************************************************/

extern void LcdInit()
{
        HardInit();
        HardOn();
        ChipInit();
        fPowerDown = 0;
}

extern char LcdIsPwrDown()
{
        return fPowerDown;
}

extern void LcdPwrOff()
{
        LcdClear();
        LcdUpdate();
        ChipOff();
        HardOff();
        fPowerDown = 1;
}

extern void LcdPwrOn()
{
        HardOn();
        ChipInit();
        fPowerDown = 0;
}

/*************************************************************
 *      Level 2 : basic function
 *************************************************************/

/**
 *
 */
#ifdef LCD_ROTATE
extern void LcdUpdate()
{
        lcd_size_t x, y;
        char *pb;

        pb = &ucBuff[LCD_CACHSIZE - 1];

        for (y = 0; y < LCD_CELL_AT_VERT; y++) {
                _mode_cmd();
                LcdSend(MAKE_ADDR_Y(y));
                LcdSend(MAKE_ADDR_X(0));
                _mode_data();
                for (x = 0; x < LCD_CELL_AT_HORZ; x++) {
                        LcdSend(*pb--);
                }
        }
}
#else
extern void LcdUpdate()
{
        lcd_size_t x, y;
        char *pb;

        pb = &ucBuff[0];

        for (y = 0; y < LCD_CELL_AT_VERT; y++) {
                _mode_cmd();
                LcdSend(MAKE_ADDR_Y(y));
                LcdSend(MAKE_ADDR_X(0));
                _mode_data();
                for (x = 0; x < LCD_CELL_AT_HORZ; x++) {
                        LcdSend(*pb++);
                }
        }
}
#endif

/**
 * Clears internal buffer the display
 */
extern void LcdClear()
{
        lcd_ind_t i = LCD_CACHSIZE;

        while (i) {
                ucBuff[--i] = 0;
        }
}

extern void LcdSetTextPos(lcd_size_t x, lcd_size_t y)
{
        if (x < LCD_CHAR_COLUMN && y < LCD_CHAR_LINE) {
                buffIndex = (lcd_ind_t) x * LCD_CHAR_WIDTH / LCD_CELL_SIZE_HORZ
                                + y * LCD_CHAR_HEIGHT * LCD_SIZE_HORZ
                                                / LCD_CELL_SIZE_HORZ
                                                / LCD_CELL_SIZE_VERT;
        }
}

static unsigned char StretchRow(unsigned char row)
{
        unsigned char retval;
        retval = row & 0x80;
        row >>= 1;
        retval |= (row & 0x60);
        row >>= 1;
        retval |= (row & 0x18);
        row >>= 1;
        retval |= (row & 0x06);
        row >>= 1;
        retval |= (row & 0x01);
        return retval;
}

extern void LcdChr(char ch, char opt)
{
        unsigned char i, top, bottom;
        char *glif;

        glif = GetGlif(ch);

        for (i = 0; i < LCD_CHAR_WIDTH; i++) {
                if (i < FONT_WIDTH) {
                        top = _get_row(glif);
                } else {
                        /* last iteration for space between characters */
                        top = 0;
                }

                if ((opt & LCD_TEXT_NEGATIVE) != 0) {
                        top = ~top;
                }

                if ((opt & LCD_TEXT_HIGH) != 0) {
                        bottom = StretchRow(top << 4);
                        top = StretchRow(top);
                        /*TODO возможен выход за пределы буфера!!! */
                        ucBuff[buffIndex + LCD_CELL_AT_HORZ] = bottom;
                }
                ucBuff[buffIndex++] = top;

                if ((opt & LCD_TEXT_WIDE) == 0) {
                        if ((opt & LCD_TEXT_HIGH) != 0) {
                                /*TODO возможен выход за пределы буфера!!! */
                                ucBuff[buffIndex + LCD_CELL_AT_HORZ] = bottom;
                        }
                        ucBuff[buffIndex++] = top;
                }
        }
}

extern void LcdImage(const unsigned char *imageData)
{
        lcd_ind_t i;

        _mode_cmd();
        for (i = 0; i < LCD_CACHSIZE; i++) {
                ucBuff[i] = imageData[i];
        }
}

extern void LcdPixel(lcd_size_t x, lcd_size_t y, unsigned char mode)
{
        lcd_ind_t index;
        unsigned char offset, mask;

        if (x > LCD_SIZE_HORZ || y > LCD_SIZE_VERT) {
                return;
        }

        index = (lcd_ind_t) (y / LCD_CELL_SIZE_VERT) * LCD_SIZE_HORZ + x;
        offset = y % LCD_CELL_SIZE_VERT;

        mask = 0x01 << offset;
        if (mode == LCD_PIXEL_OFF) {
                ucBuff[index] &= ~mask;
        } else if (mode == LCD_PIXEL_ON) {
                ucBuff[index] |= mask;
        } else if (mode == LCD_PIXEL_XOR) {
                ucBuff[index] ^= mask;
        }
}

/*************************************************************
 *      Level 3 : extends function
 *************************************************************/

extern void LcdStringEx(char *msg, char opt, lcd_size_t x, lcd_size_t y)
{
        LcdSetTextPos(x, y);
        while (*msg != 0) {
                LcdChr(*msg++, opt);
        }
}

extern void LcdLine(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char mode)
{
        lcd_pos_t dy = 0;
        lcd_pos_t dx = 0;
        lcd_pos_t stepx = 0;
        lcd_pos_t stepy = 0;
        lcd_pos_t fraction = 0;

        if (x1 > LCD_SIZE_HORZ || x2 > LCD_SIZE_HORZ || y1 > LCD_SIZE_VERT
                        || y2 > LCD_SIZE_VERT) {
                return;
        }

        dy = y2 - y1;
        dx = x2 - x1;
        if (dy < 0) {
                dy = -dy;
                stepy = -1;
        } else {
                stepy = 1;
        }
        if (dx < 0) {
                dx = -dx;
                stepx = -1;
        } else {
                stepx = 1;
        }
        dy <<= 1;
        dx <<= 1;
        LcdPixel(x1, y1, mode);
        if (dx > dy) {
                fraction = dy - (dx >> 1);
                while (x1 != x2) {
                        if (fraction >= 0) {
                                y1 += stepy;
                                fraction -= dx;
                        }
                        x1 += stepx;
                        fraction += dy;
                        LcdPixel(x1, y1, mode);
                }
        } else {
                fraction = dx - (dy >> 1);
                while (y1 != y2) {
                        if (fraction >= 0) {
                                x1 += stepx;
                                fraction -= dy;
                        }
                        y1 += stepy;
                        fraction += dx;
                        LcdPixel(x1, y1, mode);
                }
        }
}

extern void LcdCircle(lcd_size_t x, lcd_size_t y, lcd_size_t radius,
                unsigned char mode)
{
        lcd_pos_t xc = 0;
        lcd_pos_t yc = 0;
        lcd_pos_t p = 0;

        if (x > LCD_SIZE_HORZ || y > LCD_SIZE_VERT) {
                return;
        }

        yc = radius;
        p = 3 - (radius << 1);
        while (xc <= yc) {
                LcdPixel(x + xc, y + yc, mode);
                LcdPixel(x + xc, y - yc, mode);
                LcdPixel(x - xc, y + yc, mode);
                LcdPixel(x - xc, y - yc, mode);
                LcdPixel(x + yc, y + xc, mode);
                LcdPixel(x + yc, y - xc, mode);
                LcdPixel(x - yc, y + xc, mode);
                LcdPixel(x - yc, y - xc, mode);
                if (p < 0) p += (xc++ << 2) + 6;
                else p += ((xc++ - yc--) << 2) + 10;
        }
}

extern void LcdBar(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char persent)
{
        lcd_size_t line;
        if (persent > 100) {
                return;
        }
        LcdLine(x1 + 2, y2, x2 - 2, y2, 1);
        LcdLine(x2 - 2, y1, x2 - 2, y2, 1);
        LcdLine(x1 + 2, y1, x1 + 2, y2, 1);
        LcdLine(x1 + 2, y1, x2 - 2, y1, 1);

        LcdLine(x2 - 1, y1 + 1, x2 - 1, y2 - 1, 1);
        LcdLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, 1);

        LcdLine(x2, y1 + 2, x2, y2 - 2, 1);
        LcdLine(x1, y1 + 2, x1, y2 - 2, 1);

        line = persent * (x2 - x1 - 7) / 100 - 1;
        LcdLine(x1 + 4, y1 + 2, x2 - 4, y2 - 2, 0);
        LcdLine(x1 + 4, y1 + 2, x1 + 4 + line, y2 - 2, 1);
}
