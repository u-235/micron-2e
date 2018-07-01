/**
 * \file
 * \brief Драйвер монохромного дисплея.
 * \author Sylvain Bissonnette
 * \author Aheir, aheir\radiokot.ru
 * \author Nick Egorrov
 * \date 2003-2018
 * \copyright GNU Public License 3
 * \details
 */

#ifndef N3310LCD_H_INCLUDED
#define N3310LCD_H_INCLUDED

/* Используемый шрифт */
#include "font-8x5-en-rus.h"

/*************************************************************
 *      Display parameters
 *************************************************************/

/** Размер экрана по горизонтали, в точках. */
#define LCD_SIZE_HORZ           84u
/** Размер экрана по вертикали, в точках. */
#define LCD_SIZE_VERT           48u
/** Размер ячейки экрана по горизонтали. */
#define LCD_CELL_SIZE_HORZ      1u
/**
 * Размер ячейки экрана по вертикали. Ячейка - это область, которую занимает
 * один байт данных. Для дисплеев на основе pcd8544 этот параметр равен 8.
 */
#define LCD_CELL_SIZE_VERT      8u
/** Количество ячеек по горизонтали. */
#define LCD_CELL_AT_HORZ        (LCD_SIZE_HORZ/LCD_CELL_SIZE_HORZ)
/** Количество ячеек по вертикали. */
#define LCD_CELL_AT_VERT        (LCD_SIZE_VERT/LCD_CELL_SIZE_VERT)
/** Размер буфера дисплея. */
#define LCD_CACHSIZE            (LCD_CELL_AT_HORZ*LCD_CELL_AT_VERT)
/** Дополнительное пространство между символами по горизонтали. */
#define LCD_CHAR_SPASE_HORZ     1u
/** Дополнительное пространство между символами по вертикали. */
#define LCD_CHAR_SPASE_VERT     0u
/** Ширина символов по горизонтали с учётом дополнительного интервала. */
#define LCD_CHAR_WIDTH          (FONT_WIDTH + LCD_CHAR_SPASE_HORZ)
/** Ширина символов по вертикали с учётом дополнительного интервала. */
#define LCD_CHAR_HEIGHT         (FONT_HEIGHT + LCD_CHAR_SPASE_VERT)
/**
 * Количество символов, которое может вместить одна строка на экране при
 * использовании одинарной ширины символов.
 */
#define LCD_CHAR_COLUMN         (LCD_SIZE_HORZ / LCD_CHAR_WIDTH)
/**
 * Количество строк на экране при использовании одинарной высоты символов.
 */
#define LCD_CHAR_LINE           (LCD_SIZE_VERT / LCD_CHAR_HEIGHT)

/*************************************************************
 *      data types
 *************************************************************/

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
 * Тип для индексирования внутреннего буфера драйвера. Этот тип должен вмещать
 * в себя LCD_CACHSIZE.
 */
typedef unsigned int lcd_ind_t;

/*************************************************************
 *      macro
 *************************************************************/

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
 * Способ рисования графики - пиксель устанавливается, если до рисования этот
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
 * \brief Power on and initials part MCU and display.
 */
extern void LcdOn();

/**
 * \brief turn to sleep mode
 */
extern void LcdSleep();

/**
 * Power off.
 */
extern void LcdOff();

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
 * \param x Позиция по горизонтали в диапазоне от 0 до LCD_CHAR_COLUMN-1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до LCD_CHAR_LINE-1
 *  включительно.
 */
extern void LcdSetTextPos(lcd_size_t x, lcd_size_t y);

/**
 * Вывод символа в текущей позиции.
 * \param ch Выводимый символ. Кодировка зависит от применяемого файла шрифтов.
 * \param opt Опции вывода, может быть любой комбинацией флагов
 *       #LCD_TEXT_NEGATIVE, #LCD_TEXT_WIDE или #LCD_TEXT_HIGH.
 */
extern void LcdChr(char ch, char opt);

/**
 * Вывод изображения.
 * \param imageData Массив со снимком экрана.
 */
extern void LcdImage(const unsigned char *imageData);

/**
 * Рисование точки.
 * \param x Позиция по горизонтали от нуля до #LCD_SIZE_HORZ - 1
 * \param y Позиция по вертикали от нуля до #LCD_SIZE_VERT - 1
 * \param mode Режим рисования пикселя, может быть одним из #LCD_PIXEL_OFF,
 *  #LCD_PIXEL_ON или #LCD_PIXEL_XOR
 */
extern void LcdPixel(lcd_size_t x, lcd_size_t y, unsigned char mode);

/**
 * \brief Вывод текста.
 * \details
 * \param msg Указатель на строку в оперативной памяти.
 * \param opt Опции вывода, может быть любой комбинацией флагов
 *       #LCD_TEXT_NEGATIVE, #LCD_TEXT_WIDE или #LCD_TEXT_HIGH.
 * \param x Позиция по горизонтали в диапазоне от 0 до #LCD_CHAR_COLUMN - 1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до #LCD_CHAR_LINE - 1
 *  включительно.
 */
extern void LcdStringEx(char *msg, char opt, lcd_size_t x, lcd_size_t y);

#ifdef __DOXYGEN__
/**
 * Вывод строки в обычном виде.
 * \param msg Указатель на строку в оперативной памяти.
 * \param x Позиция по горизонтали в диапазоне от 0 до #LCD_CHAR_COLUMN - 1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до #LCD_CHAR_LINE - 1
 *  включительно.
 */
extern void LcdString(char *msg, lcd_size_t x, lcd_size_t y);

/**
 * Вывод строки с инверсией.
 * \param msg Указатель на строку в оперативной памяти.
 * \param x Позиция по горизонтали в диапазоне от 0 до #LCD_CHAR_COLUMN - 1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до #LCD_CHAR_LINE - 1
 *  включительно.
 */
extern void LcdStringInv(char *msg, lcd_size_t x, lcd_size_t y);

/**
 * Вывод строки высоким и широким шрифтом в обычном виде.
 * \param msg Указатель на строку в оперативной памяти.
 * \param x Позиция по горизонтали в диапазоне от 0 до #LCD_CHAR_COLUMN/2 - 1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до #LCD_CHAR_LINE/2 - 1
 *  включительно.
 */
extern void LcdStringBold(char *msg, lcd_size_t x, lcd_size_t y);

/**
 * Вывод строки высоким и широким шрифтом с инверсией.
 * \param msg Указатель на строку в оперативной памяти.
 * \param x Позиция по горизонтали в диапазоне от 0 до #LCD_CHAR_COLUMN/2 - 1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до #LCD_CHAR_LINE/2 - 1
 *  включительно.
 */
extern void LcdStringBoldInv(char *msg, lcd_size_t x, lcd_size_t y);

/**
 * Вывод строки высоким шрифтом в обычном виде.
 * \param msg Указатель на строку в оперативной памяти.
 * \param x Позиция по горизонтали в диапазоне от 0 до #LCD_CHAR_COLUMN - 1
 *  включительно.
 * \param y Позиция по вертикали в диапазоне от 0 до #LCD_CHAR_LINE/2 - 1
 *  включительно.
 */
extern void LcdStringBig(char *msg, lcd_size_t x, lcd_size_t y);
#else /* real */
#define LcdString(msg, x, y) LcdStringEx(msg, 0,x, y)
#define LcdStringInv(msg, x, y) LcdStringEx(msg, LCD_TEXT_NEGATIVE,x, y)
#define LcdStringBold(msg, x, y) \
        LcdStringEx(msg, LCD_TEXT_WIDE | LCD_TEXT_HIGH,x, y)
#define LcdStringBoldInv(msg, x, y) \
        LcdStringEx(msg, LCD_TEXT_WIDE | LCD_TEXT_HIGH | LCD_TEXT_NEGATIVE,x, y)
#define LcdStringBig(msg, x, y) \
        LcdStringEx(msg, LCD_TEXT_HIGH,x, y)
#endif

/**
 * Рисование линии между двух точек.
 * \param x1 Позиция первой точки по горизонтали от нуля до #LCD_SIZE_HORZ - 1
 * \param y1 Позиция первой точки по вертикали от нуля до #LCD_SIZE_VERT - 1
 * \param x2 Позиция второй точки по горизонтали от нуля до #LCD_SIZE_HORZ - 1
 * \param y2 Позиция второй точки по вертикали от нуля до #LCD_SIZE_VERT - 1
 * \param mode Режим рисования пикселя, может быть одним из #LCD_PIXEL_OFF,
 *  #LCD_PIXEL_ON или #LCD_PIXEL_XOR
 */
extern void LcdLine(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char mode);

/**
 * Рисование круга.
 * \param x Позиция центра круга по горизонтали от нуля до #LCD_SIZE_HORZ - 1
 * \param y Позиция центра круга по вертикали от нуля до #LCD_SIZE_VERT - 1
 * \param radius Радиус круга.
 * \param mode Режим рисования пикселя, может быть одним из #LCD_PIXEL_OFF,
 *  #LCD_PIXEL_ON или #LCD_PIXEL_XOR
 */
extern void LcdCircle(lcd_size_t x, lcd_size_t y, lcd_size_t radius,
                unsigned char mode);

/**
 * Рисование прогресс-бара с заполнением его на "процент".
 * \param x1 Позиция первой точки по горизонтали от нуля до #LCD_SIZE_HORZ - 1
 * \param y1 Позиция первой точки по вертикали от нуля до #LCD_SIZE_VERT - 1
 * \param x2 Позиция второй точки по горизонтали от нуля до #LCD_SIZE_HORZ - 1
 * \param y2 Позиция второй точки по вертикали от нуля до #LCD_SIZE_VERT - 1
 * \param persent Процент заполнения.
 */
extern void LcdBar(lcd_size_t x1, lcd_size_t y1, lcd_size_t x2, lcd_size_t y2,
                unsigned char persent);

#endif // N3310LCD_H_INCLUDED
