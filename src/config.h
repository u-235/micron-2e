/*****************************************************************************
 *
 *  micron 2 v 1.2.6
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

/*************************************************************
 *      Опции сборки
 *************************************************************/

/**
 *
 */
/* #define non_alarm_inversion */

/*************************************************************
 *      Опции питания
 *************************************************************/

/**
 * Если определено - работаем c литиевого аккумулятора и меряем напряжение
 * внутри микроконтроллера. Так же происходит коррекция накачки при изменениях
 * напряжения.
 * Иначе используется вариант питания от батарейки через повышающий
 * преобразователь и АЦП меряет напряжение на внешнем входе.
 */
#define POWER_LION

/**
 * \def POWER_VOLTAGE_LOW
 * Минимальный уровень напряжения питания в сотых долях вольта. Влияет на
 * вычисление уровня заряда и на подстройку накачки преобразователя.
 *
 * \def POWER_VOLTAGE_HIGH
 * Максимальный уровень напряжения питания в сотых долях вольта. Влияет на
 * вычисление уровня заряда и на подстройку накачки преобразователя.
 *
 * \def POWER_LEVEL_ALARM
 * Уровень оставшегося заряда в процентах, ниже которого начинается тревога
 * "низкий уровень заряда"
 */
#ifdef POWER_LION
#define POWER_VOLTAGE_LOW       320
#define POWER_VOLTAGE_HIGH      420
#define POWER_LEVEL_ALARM       10
#else /************************************************************************/
#define POWER_VOLTAGE_LOW       100
#define POWER_VOLTAGE_HIGH      150
#define POWER_LEVEL_ALARM       10
#endif

/*************************************************************
 *      Опции преобразователя
 *************************************************************/

/**
 *  Если определено - в преобразователе используется MOSFET, иначе -
 *  биполярник. Это влияет на расчёт режимов работы преобразователя в схеме с
 *  питанием от аккумулятора.
 */
#define PUMP_SWITCH_MOSFET

#ifdef POWER_LION
/**
 * \def PUMP_PULSE_WIDTH_DEFAULT
 * Длительность положительнного импульса накачки; задаётся в количестве циклов
 * в RunCharge(), точное время неизвестно. Обычно достаточно 2.
 *
 * \def PUMP_PULSE_WIDTH_MIN
 * Минимальная длительность импульса накачки. Используется как ограничение при
 * настройке прибора.
 *
 * \def PUMP_PULSE_WIDTH_MAX
 * Максимальнаая длительность импульса накачки. Используется как ограничение при
 * настройке прибора.
 */
#define PUMP_PULSE_WIDTH_DEFAULT        2
#define PUMP_PULSE_WIDTH_MIN            1
#define PUMP_PULSE_WIDTH_MAX            10
/**
 * \def PUMP_TICKS_PERIODIK_DEFAULT
 * Количество импульсов принудительной накачки. Эта накачка происходит по
 * таймеру.
 *
 * \def PUMP_TICKS_PERIODIK_MIN
 * Минимальное количество импульсов принудительной накачки. Используется как
 * ограничение при настройке прибора.
 *
 * \def PUMP_TICKS_PERIODIK_MAX
 * Максимальное количество импульсов принудительной накачки. Используется как
 * ограничение при настройке прибора.
 */
#define PUMP_TICKS_PERIODIK_DEFAULT     10
#define PUMP_TICKS_PERIODIK_MIN         3
#define PUMP_TICKS_PERIODIK_MAX         40
/**
 * \def PUMP_TICKS_HIT_DEFAULT
 * Количество импульсов накачки при сработке датчика.
 *
 * \def PUMP_TICKS_HIT_MIN
 * Минимальное количество импульсов накачки при сработке датчика. Используется
 * как ограничение при настройке прибора.
 *
 * \def PUMP_TICKS_HIT_MAX
 * Максимальное количество импульсов накачки при сработке датчика. Используется
 * как ограничение при настройке прибора.
 */
#define PUMP_TICKS_HIT_DEFAULT          8
#define PUMP_TICKS_HIT_MIN              2
#define PUMP_TICKS_HIT_MAX              25
#else /************************************************************************/
#define PUMP_PULSE_WIDTH_DEFAULT        2
#define PUMP_PULSE_WIDTH_MIN            1
#define PUMP_PULSE_WIDTH_MAX            10
#define PUMP_TICKS_PERIODIK_DEFAULT     100
#define PUMP_TICKS_PERIODIK_MIN         80
#define PUMP_TICKS_PERIODIK_MAX         254
#define PUMP_TICKS_HIT_DEFAULT          16
#define PUMP_TICKS_HIT_MIN              5
#define PUMP_TICKS_HIT_MAX              50
#endif

#ifdef PUMP_SWITCH_MOSFET
/**
 * Падение напряжения на ключе преобразователя в сотых долях вольта. Для MOSFET
 *  равно нулю, для биполярного транзистора около 30-40.
 */
#define SWITCH_DROP_VOLTAGE 0
#else
#define SWITCH_DROP_VOLTAGE 35
#endif

/*************************************************************
 *      Pin's definition for sensor, LEDs and keys
 *************************************************************/

#define OUT_LED_BLUE    D, 5
#define OUT_LED_RED     D, 0
#define OUT_BEEPER      C, 1
#define OUT_PUMP_SWITCH D, 1
#define IN_KEY_INT      D, 2
#define IN_KEY_PLUS     C, 2
#define IN_SENSOR       D, 3

/*************************************************************
 *      LCD Port & pinout setup.
 *************************************************************/

/**
 * Вывод команда/данные (D/C). Может быть любым выводом любого порта.
 */
#define LCD_DC B, 4
/**
 * Вывод сброса дисплея (/RES). Может быть любым выводом любого порта.
 */
#define LCD_RST C, 3
/**
 * Вывод последовательных данных (SDIN). Если используется аппаратный SPI, то
 * должен быть только MOSI. Для программного SPI - любой вывод любого порта.
 */
#define LCD_MOSI B, 3
/**
 * Вывод тактирования последовательных данных (SCLK). Если используется
 * аппаратный SPI, то должен быть только SCK. Для программного SPI - любой
 * вывод любого порта.
 */
#define LCD_CLK B, 5
/**
 * Вывод выбора дисплея (/SCE). Если используется аппаратный SPI, то должен быть
 * только /SS. Для программного SPI - любой вывод любого порта.
 */
#define LCD_SS B, 2
/**
 * Вывод включения питания дисплея. Может быть любым выводом любого порта.
 */
#define LCD_PWR C, 0

/**
 * Если определено - информация выводится на экран с поворотом 180 градусов.
 */
#define LCD_ROTATE

/**
 * Если определено - используем программный SPI, иначе - аппаратный.
 */
#define LCD_SOFT_SPI

/**
 * Если определено - работаем по алгоритмам "китайского" дисплея, иначе -
 * оригинального.
 * #define LCD_NOKIA3310_CHINA
 */

/*************************************************************
 *      Other options
 *************************************************************/

/**
 * Высота вывода гистограммы в пикселях.
 */
#define DOSE_HISTOGRAM_SIZE 16

/**
 * Чувствительность датчика, импульсов на мкР
 */
#define SENSOR_SENSITIVITY 100
