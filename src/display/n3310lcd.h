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

#ifndef N3310LCD_H_INCLUDED
#define N3310LCD_H_INCLUDED

/**
 * Тип для задания координат экрана без знака. Этот тип используется для
 * описания аргументов функций использующих координаты.
 */
typedef unsigned char lcd_size_t;
/**
 * Тип для задания позиции на экране со знаком. Этот тип используется внутри
 * некоторых фунциий для вычислений.
 */
typedef signed char lcd_pos_t;
/**
 * Тип для индексирования внутреннего буфера драйвера.
 */
typedef unsigned int lcd_ind_t;

/**
 * \def LCD_MODE_BLANK
 * Режим дисплея - пустой экран, вне зависимости от содержимого памяти дисплея.
 *
 * \def LCD_MODE_FILLED
 * Режим дисплея - чёрный экран, вне зависимости от содержимого памяти дисплея.
 *
 * \def LCD_MODE_NORMAL
 * Режим дисплея - нормальное отображение содержимого памяти дисплея.
 *
 * \def LCD_MODE_INVERSE
 * Режим дисплея - негативное отображение содержимого памяти дисплея.
 */
#define LCD_MODE_BLANK          0x00
#define LCD_MODE_FILLED         0x01
#define LCD_MODE_NORMAL         0x02
#define LCD_MODE_INVERSE        0x03

/**
 * \def LCD_PIXEL_OFF
 * Способ рисования графики - пиксели сбрасываются.
 *
 * \def LCD_PIXEL_ON
 * Способ рисования графики - пиксели устанавливаются.
 *
 * \def LCD_PIXEL_XOR
 * Способ рисования графики - пиксель устанавливается если до рисования это
 * пиксель сброшен. Иначе пиксель сбрасывается.
 */
#define LCD_PIXEL_OFF       0
#define LCD_PIXEL_ON        1
#define LCD_PIXEL_XOR       2

/**
 * \def LCD_TEXT_NEGATIVE
 * Текст отрисовывается негативно. Этот флаг можно комбинировать с
 * #LCD_TEXT_WIDE и #LCD_TEXT_HIGH.
 *
 * \def LCD_TEXT_WIDE
 * Символы рисуются двойной ширины. Этот флаг можно комбинировать с
 * #LCD_TEXT_NEGATIVE и #LCD_TEXT_HIGH.
 *
 * \def LCD_TEXT_HIGH
 * Символы рисуются двойной высоты. Этот флаг можно комбинировать с
 * #LCD_TEXT_NEGATIVE и #LCD_TEXT_WIDE.
 */
#define LCD_TEXT_NEGATIVE       (1 << 0)
#define LCD_TEXT_WIDE           (1 << 1)
#define LCD_TEXT_HIGH           (1 << 2)

/**
 * Установка контрастности.
 * \param contrast Значение контрастности от 0 до 0x7f
 */
extern void LcdContrast(unsigned char contrast);

/**
 * Установка режима работы дисплея.
 * \param mode Один из режимов #LCD_MODE_BLANK, #LCD_MODE_FILLED,
 *      #LCD_MODE_NORMAL или #LCD_MODE_INVERSE
 */
extern void LcdMode(unsigned char mode);

/**
 * \brief Initials part MCU and display.
 */
extern void LcdInit();

/**
 * Display state.
 * \return 0 - chip is active, non zero - chip is in PD-mode
 */
extern char LcdIsPwrDown();

/**
 * Выключение дисплея.
 */
extern void LcdPwrOff();

/**
 * Включение дисплея.
 */
extern void LcdPwrOn();

/**
 * \brief Flush internal buffer into display.
 */
extern void LcdUpdate();

/**
 * \brief Clear internal buffer.
 * \details To apply change call LcdUpdate().
 */
extern void LcdClear();

/**
 * Установка позиции вывода символа.
 * \param x Позиция по горизонтали в диапазоне от 0 до 13 включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до 5 включительно.
 */
extern void LcdSetTextPos(lcd_size_t x, lcd_size_t y);

/**
 * Вывод символа в текущей позиции.
 * \param ch Выводимый символ. Кодировка зависит от применяемого файла.
 * \param opt Опции вывода, может быть любой комбинацией флагов
 *       #LCD_TEXT_NEGATIVE, #LCD_TEXT_WIDE или #LCD_TEXT_HIGH
 */
extern void LcdChr(char ch, char opt);

//вывод изображения
extern void LcdImage(const unsigned char *imageData);

//Displays a pixel at given absolute (x, y) location, mode -> Off, On or Xor
extern void LcdPixel(lcd_size_t x, lcd_size_t y, unsigned char mode);

//Displays a string at current cursor location
extern void LcdStringEx(char *msg, char opt, lcd_size_t x, lcd_size_t y);

#define LcdString(msg, x, y) LcdStringEx(msg, 0,x, y)

#define LcdStringInv(msg, x, y) LcdStringEx(msg, LCD_TEXT_NEGATIVE,x, y)

#define LcdStringBold(msg, x, y) \
        LcdStringEx(msg, LCD_TEXT_WIDE | LCD_TEXT_HIGH,x, y)

#define LcdStringBoldInv(msg, x, y) \
        LcdStringEx(msg, LCD_TEXT_WIDE | LCD_TEXT_HIGH | LCD_TEXT_NEGATIVE,x, y)

#define LcdStringBig(msg, x, y) \
        LcdStringEx(msg, LCD_TEXT_HIGH,x, y)

//Draws a line between two points on the display
extern void LcdLine(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char mode);

//рисуем круг с координатами центра и радиусом
extern void LcdCircle(lcd_size_t x, lcd_size_t y, lcd_size_t radius,
                unsigned char mode);

// рисует прогресс-бар и заполняет его на "процент"
extern void LcdBar(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char persent);

#endif // N3310LCD_H_INCLUDED
