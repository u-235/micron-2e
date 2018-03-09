/*****************************************************************************
 *
 *  Nokia 3310 display driver
 *  Copyright (C) 2003  Sylvain Bissonnette
 *  Copyright (C) 2010  Aheir, aheir@radiokot.ru
 *  Copyright (C) 2018  Nick Egorrov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

/**
 * \file
 * \brief Драйвер монохромного дисплея.
 * \details
 *
 */

#define LCD_MONO_INSIDE_

#include "n3310lcd.h"
#include "pcd8544.h"
#include "target.h"
#include "font-8x5-en-rus.h"
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

#define LCD_SIZE_HORZ           84	//разрешение экрана
#define LCD_SIZE_VERT           48
#define LCD_CELL_SIZE_HORZ      1
#define LCD_CELL_SIZE_VERT      8

/*************************************************************
 *      Variables
 *************************************************************/

#define LCD_CELL_AT_HORZ        (LCD_SIZE_HORZ/LCD_CELL_SIZE_HORZ)
#define LCD_CELL_AT_VERT        (LCD_SIZE_VERT/LCD_CELL_SIZE_VERT)

#define LCD_CHAR_SPASE_HORZ     1
#define LCD_CHAR_SPASE_VERT     0
#define LCD_CHAR_WIDTH          (FONT_WIDTH + LCD_CHAR_SPASE_HORZ)
#define LCD_CHAR_HEIGHT         (FONT_HEIGHT + LCD_CHAR_SPASE_VERT)
#define LCD_CHAR_COLUMN         (LCD_SIZE_HORZ / LCD_CHAR_WIDTH)
#define LCD_CHAR_LINE           (LCD_SIZE_VERT / LCD_CHAR_HEIGHT)

#define LCD_CACHSIZE            (LCD_CELL_AT_HORZ*LCD_CELL_AT_VERT)

/*************************************************************
 *      Variables
 *************************************************************/

static char ucBuff[LCD_CACHSIZE];
static lcd_ind_t buffIndex;
//power-down control: 0 - chip is active, 1 - chip is in PD-mode
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

//contrast -> Contrast value from 0x00 to 0x7F
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

//режим дисплея: 0 - blank, 1 - all on, 2 - normal, 3 - inverse
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

extern void LcdPwrOff()  //выкл дисплея
{
        LcdClear();
        LcdUpdate();
        ChipOff();
        HardOff();
        fPowerDown = 1;
}

extern void LcdPwrOn()  //вкл дисплея
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
        int i = LCD_CACHSIZE;

        while (i) {
                ucBuff[--i] = 0;
        }
}

//Sets cursor location to xy location. Range: 1,1 .. 14,6
extern void LcdSetTextPos(lcd_size_t x, lcd_size_t y)
{
        if (x < LCD_CHAR_COLUMN && y < LCD_CHAR_LINE) {
                buffIndex = (lcd_ind_t) (y - 1) * LCD_CELL_AT_HORZ
                                + (x - 1) * LCD_CHAR_WIDTH;
        }
}

static unsigned char StretchRow(unsigned char row)
{
        unsigned char retval;
        retval = row & 0x80;
        retval |= ((row >> 1) & 0x60);
        retval |= ((row >> 1) & 0x18);
        retval |= ((row >> 1) & 0x06);
        retval |= ((row >> 1) & 0x01);
        return retval;
}

//Displays a character at current cursor location and increment cursor location
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

//вывод изображения
extern void LcdImage(const unsigned char *imageData)
{
        lcd_ind_t i;

        _mode_cmd();
        for (i = 0; i < LCD_CACHSIZE; i++) {
                ucBuff[i] = imageData[i];  //грузим данные
        }
}

//Displays a pixel at given absolute (x, y) location, mode -> Off, On or Xor
extern void LcdPixel(lcd_size_t x, lcd_size_t y, unsigned char mode)
{
        lcd_ind_t index;
        unsigned char offset, mask;

        if (x > LCD_SIZE_HORZ || y > LCD_SIZE_VERT) {
                return;
        }

        index = (lcd_ind_t) y / LCD_CELL_SIZE_VERT * LCD_SIZE_HORZ + x;  //считаем номер байта в массиве памяти дисплея
        offset = y & 0x03;  //считаем номер бита в этом байте

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

//Displays a string at current cursor location
extern void LcdStringEx(char *msg, char opt, lcd_size_t x, lcd_size_t y)
{
        LcdSetTextPos(x, y);
        while(*msg != 0) {
                        LcdChr(*msg++, opt);
        }
}

//Draws a line between two points on the display - по Брезенхейму
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

//рисуем круг по координатам с радиусом - по Брезенхейму
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

//рисуем прогресс-бар
extern void LcdBar(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char persent)
{
        lcd_size_t line;
        if (persent > 100) {
                return;
        }
        LcdLine(x1 + 2, y2, x2 - 2, y2, 1);  //down
        LcdLine(x2 - 2, y1, x2 - 2, y2, 1);  //right
        LcdLine(x1 + 2, y1, x1 + 2, y2, 1);  //left
        LcdLine(x1 + 2, y1, x2 - 2, y1, 1);  //up

        LcdLine(x2 - 1, y1 + 1, x2 - 1, y2 - 1, 1);  //right
        LcdLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, 1);  //left

        LcdLine(x2, y1 + 2, x2, y2 - 2, 1);  //right
        LcdLine(x1, y1 + 2, x1, y2 - 2, 1);  //left

        line = persent * (x2 - x1 - 7) / 100 - 1;
        LcdLine(x1 + 4, y1 + 2, x2 - 4, y2 - 2, 0);
        LcdLine(x1 + 4, y1 + 2, x1 + 4 + line, y2 - 2, 1);
}
