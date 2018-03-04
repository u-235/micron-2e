/*****************************************************************************
 *
 *  Nokia 3310 display driver
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

/**********************************************
 ****************PCD8544 Driver*****************
 ***********************************************

 for original NOKIA 3310 & alternative "chinese" version of display

 48x84 dots, 6x14 symbols

 **********************************************/

#include "n3310lcd.h"
#include "pcd8544.h"
#include "target.h"
#include "font-8x5-en-rus.h"
#include "chip.h"

//***********************************************************
//Настройки контроллера дисплея и переменные для работы с ним
//***********************************************************

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

#ifndef LCD_NOKIA3310_CHINA_SHIFT_Y
#define LCD_NOKIA3310_CHINA_SHIFT_Y 5
#endif

#define PIXEL_OFF	0		//режимы отображения пикселя - используются в графических функциях
#define PIXEL_ON	1
#define PIXEL_XOR	2

#define LCD_X_RES               84	//разрешение экрана
#define LCD_Y_RES               48
#define LCD_CACHSIZE          LCD_X_RES*LCD_Y_RES/8

#define Cntr_X_RES              102    	//разрешение контроллера - предполагаемое - но работает))
#define Cntr_Y_RES              64
#define Cntr_buf_size           Cntr_X_RES*Cntr_Y_RES/8

unsigned char LcdCache[LCD_CACHSIZE];  //Cache buffer in SRAM 84*48 bits or 504 bytes
unsigned int LcdCacheIdx;  //Cache index

//power-down control: 0 - chip is active, 1 - chip is in PD-mode
static char fPowerDown = 0;

extern void LcdUpdate()  //Copies the LCD cache into the device RAM
{
        int i;
#ifdef LCD_NOKIA3310_CHINA
        char j;
#endif
        _mode_cmd();
        LcdSend(MAKE_ADDR_X(0));
        LcdSend(MAKE_ADDR_Y(0));
        _mode_data();

#ifdef LCD_NOKIA3310_CHINA                    		//если китайский дисплей - грузим пустую строку
        for (j = Cntr_X_RES; j > 0; j--) {
                LcdSend(0);
        }
#endif
#ifdef LCD_ROTATE
        for (i = LCD_CACHSIZE - 1; i >= 0; i--)  //грузим данные
#else
                        for (i = 0; i < LCD_CACHSIZE; i++)  //грузим данные
#endif
                        {
                LcdSend(LcdCache[i]);
#ifdef LCD_NOKIA3310_CHINA				//если дисплей китайский - догружаем каждую строку до размера его буфера
                if (++j == LCD_X_RES) {
                        for (j = (Cntr_X_RES - LCD_X_RES); j > 0; j--) {
                                LcdSend(0);
                        }
                        j = 0;
                }
#endif
        }
}

/**
 * Clears internal buffer the display
 */
extern void LcdClear()
{
        int i = LCD_CACHSIZE;

        while (i) {
                LcdCache[--i] = 0;
        }
}

static void PrivateInit()
{
        HardInit();
        _chip_enable();
        delay_ms(1);
        _clock_down();
        _reset_down();
        delay_ms(20);
        _reset_up();
        _chip_disable();

        _mode_cmd();
        LcdSend(MAKE_FUNC(FUNC_SET_EXTEND));
        LcdSend(EMAKE_TEMP(LCD_TEMPERATURE));
        LcdSend(CMAKE_SPI_MODE(SPI_MODE_MSB)); /* порядок битов в SPI для китая */
        LcdSend(EMAKE_BIAS(LCD_BIAS));
        LcdSend(EMAKE_CONTR(LCD_CONTRAST));
        LcdSend(CMAKE_SHIFT(LCD_NOKIA3310_CHINA_SHIFT_Y));
        LcdSend(MAKE_FUNC(FUNC_SET_BASIC | LCD_DIRECTION | FUNC_PWR_ACTIVE));
        LcdMode(LCD_MODE_NORMAL);
        LcdClear();
}

extern void LcdInit()
{
        LcdPwrOn();
}

extern void LcdContrast(unsigned char contrast)  //contrast -> Contrast value from 0x00 to 0x7F
{
        if (contrast > 0x7F) {
                return;
        }
        _mode_cmd();
        LcdSend(MAKE_FUNC(FUNC_SET_EXTEND | LCD_DIRECTION | FUNC_PWR_ACTIVE));
        LcdSend(EMAKE_CONTR(contrast));
        LcdSend(MAKE_FUNC(FUNC_SET_BASIC | LCD_DIRECTION | FUNC_PWR_ACTIVE));
}

extern void LcdMode(unsigned char mode)  //режим дисплея: 0 - blank, 1 - all on, 2 - normal, 3 - inverse
{
        if (mode > 3) {
                return;
        }
        _mode_cmd();
        LcdSend(MAKE_MODE(mode));
}

extern void LcdImage(const unsigned char *imageData)  //вывод изображения
{
        unsigned int i;

        _mode_cmd();
        for (i = 0; i < LCD_CACHSIZE; i++) {
                LcdCache[i] = imageData[i];  //грузим данные
        }
}

extern void LcdPixel(unsigned char x, unsigned char y, unsigned char mode)  //Displays a pixel at given absolute (x, y) location, mode -> Off, On or Xor
{
        int index;
        unsigned char offset, data;

        if (x > LCD_X_RES) {
                return;  //если передали в функцию муть - выходим
        }
        if (y > LCD_Y_RES) {
                return;
        }

        index = (((int) (y) / 8) * 84) + x;  //считаем номер байта в массиве памяти дисплея
        offset = y - ((y / 8) * 8);  //считаем номер бита в этом байте

        data = LcdCache[index];  //берем байт по найденному индексу

        if (mode == PIXEL_OFF) {
                data &= (~(0x01 << offset));  //редактируем бит в этом байте
        } else if (mode == PIXEL_ON) {
                data |= (0x01 << offset);
        } else if (mode == PIXEL_XOR) {
                data ^= (0x01 << offset);
        }

        LcdCache[index] = data;  //загружаем байт назад
}

extern void LcdLine(int x1, int y1, int x2, int y2, unsigned char mode)  //Draws a line between two points on the display - по Брезенхейму
{
        signed int dy = 0;
        signed int dx = 0;
        signed int stepx = 0;
        signed int stepy = 0;
        signed int fraction = 0;

        if (x1 > LCD_X_RES || x2 > LCD_X_RES || y1 > LCD_Y_RES || y2 > LCD_Y_RES) {
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

extern void LcdCircle(char x, char y, char radius, unsigned char mode)  //рисуем круг по координатам с радиусом - по Брезенхейму
{
        signed char xc = 0;
        signed char yc = 0;
        signed char p = 0;

        if (x > LCD_X_RES || y > LCD_Y_RES) {
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

extern void Batt(int x1, int y1)  //рисуем батарейку
{
        LcdLine(x1, y1, (x1 + 5), y1, 1);  //up
        LcdLine(x1 + 1, (y1 + 9), (x1 + 1 + 25), (y1 + 9), 1);  //down
        LcdLine(x1 + 1, y1, x1 + 1, (y1 + 9), 1);  //left
        LcdLine((x1 + 1 + 25), y1, (x1 + 1 + 25), (y1 + 9), 1);  //right
        LcdLine(x1, y1 + 3, x1, y1 + 6, 1);  // пимпочка
}

extern void LcdBar(int x1, int y1, int x2, int y2, unsigned char persent)  //рисуем прогресс-бар
{
        unsigned char line;
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

extern void LcdSetTextPos(unsigned char x, unsigned char y)  //Sets cursor location to xy location. Range: 1,1 .. 14,6
{
        if (x <= 14 && y <= 6) {
                LcdCacheIdx = ((int) (y) - 1) * 84 + ((int) (x) - 1) * 6;
        }
}

//Displays a character at current cursor location and increment cursor location
extern void LcdChr(char ch, char inv)
{
        unsigned char i, row;
        const char *glif;

        glif = GetGlif(ch);

        for (i = 0; i < 5; i++) {
                row = _get_row(glif);
                if (inv) {
                        row = ~row;
                }
                LcdCache[LcdCacheIdx++] = row;
        }

        if (inv) {
                row = 0xff;
        } else {
                row = 0;
        }
        //добавляем пробел между символами
        LcdCache[LcdCacheIdx++] = row;
}

unsigned char StretchRow(unsigned char row)
{
        unsigned char retval;

        retval = (row & 0x01) * 3;
        retval |= (row & 0x02) * 6;
        retval |= (row & 0x04) * 12;
        retval |= (row & 0x08) * 24;
        return retval;
}

extern void LcdChrBold(char ch, char inv)  //Displays a bold character at current cursor location and increment cursor location
{
        unsigned char i, a = 0, b = 0, row = 0;
        const char *glif;

        glif = GetGlif(ch);

        for (i = 0; i < 5; i++) {
                row = _get_row(glif);
                if (inv) {
                        row = ~row;
                }

                b = StretchRow(row);  //"растягиваем" столбец на два байта
                row >>= 4;
                a = StretchRow(row);

                LcdCache[LcdCacheIdx] = b;  //копируем байты в экранный буфер
                LcdCache[LcdCacheIdx + 1] = b;  //дублируем для получения жирного шрифта
                LcdCache[LcdCacheIdx + 84] = a;
                LcdCache[LcdCacheIdx + 85] = a;
                LcdCacheIdx = LcdCacheIdx + 2;
        }

        if (inv) {
                row = 0xff;
        } else {
                row = 0;
        }
        //для пробела между символами
        LcdCache[LcdCacheIdx++] = row;
        LcdCache[LcdCacheIdx++] = row;
}

extern void LcdChrBig(char ch, char inv)  //Displays a character at current cursor location and increment cursor location
{
        unsigned char i;
        unsigned char a = 0, b = 0, row = 0;
        const char *glif;

        glif = GetGlif(ch);

        for (i = 0; i < 5; i++) {
                row = _get_row(glif);
                if (inv) {
                        row = ~row;
                }

                b = StretchRow(row);  //"растягиваем" столбец на два байта
                row >>= 4;
                a = StretchRow(row);

                LcdCache[LcdCacheIdx] = b;
                LcdCache[LcdCacheIdx + 84] = a;
                LcdCacheIdx = LcdCacheIdx + 1;
        }

        if (inv) {
                row = 0xff;
        } else {
                row = 0;
        }
        LcdCache[LcdCacheIdx++] = row;
}

extern void LcdString(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 14 || y > 6) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 15 - x; i++) {
                if (msg[i]) {
                        LcdChr(msg[i], 0);
                }
        }
}

extern void LcdStringInv(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 14 || y > 6) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 15 - x; i++) {
                if (msg[i]) {
                        LcdChr(msg[i], 1);
                }
        }
}

extern void LcdStringBold(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 13 || y > 5) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 14 - x; i++) {
                if (msg[i]) {
                        LcdChrBold(msg[i], 0);
                }
        }
}

extern void LcdStringBoldInv(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 13 || y > 5) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 14 - x; i++) {
                if (msg[i]) {
                        LcdChrBold(msg[i], 1);
                }
        }
}

extern void LcdStringBig(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 14 || y > 5) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 15 - x; i++) {
                if (msg[i]) {
                        LcdChrBig(msg[i], 0);
                }
        }
}

extern char LcdIsPwrDown()
{
        return fPowerDown;
}

extern void LcdPwrOff()  //выкл дисплея
{
        LcdClear();
        LcdUpdate();
        _mode_cmd();
        LcdSend(MAKE_FUNC(FUNC_SET_BASIC | LCD_DIRECTION | FUNC_PWR_DOWN));
        HardOff();
        fPowerDown = 1;
}

extern void LcdPwrOn()  //вкл дисплея
{
        HardOn();
        PrivateInit();
        fPowerDown = 0;
}
