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

/**
 * Если определено - работаем по алгоритмам "китайского" дисплея, иначе -
 * оригинального
 *
 */
/* #define LCD_NOKIA3310_CHINA */

/**
 *  Если определено - в преобразователе используется MOSFET, иначе -
 *  биполярник. Это влияет на расчет режимов работы преобразователя в схеме с
 *  питанием от аккумулятора.
 */
#define PUMP_SWITCH_MOSFET

/**
 * Если определено - работаем c литиевого аккума и меряем напряжение внутри
 * микроконтроллера. Так же происходит коррекция накачки при изменениях
 * напряжения.
 * Иначе используется вариант питания от батарейки через повышающий
 * преобразователь и АЦП меряет напряжение на внешнем входе.
 */
#define POWER_LION

/*************************************************************
 *      Опции питания
 *************************************************************/

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

#ifdef POWER_LION
/**
 * Длительность положительнного импульса накачки. Обычно достаточно 2.
 * Если очень хороший транс - можно и 1 поставить.
 */
#define PUMP_PULSE_WIDTH_DEFAULT        2
#define PUMP_PULSE_WIDTH_MIN            1
#define PUMP_PULSE_WIDTH_MAX            10
/**
 * Количество импульсов принудительной накачки       (330 максимум)
 * у меня 10 и всё прекрасно работает, а при 13 завышает, порой до 30 мкР/ч при 15...
 */
#define PUMP_TICKS_PERIODIK_DEFAULT     10
#define PUMP_TICKS_PERIODIK_MIN         3
#define PUMP_TICKS_PERIODIK_MAX         40
/**
 * Количество импульсов накачки при импульсе         (330 максимум)
 * При ЕРФ 8 - отлично. А если меньше или больше - либо замолкает, либо завышает.
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

//#define blue_led_pin  PORTD.5
//#define red_led_pin   PORTD.0
//#define beep_pin      PORTC.1
//#define trans         PORTD.1
//#define key           PIND.2 (int0)
//#define key_plus      PINC.2 (key +)
//#define impulse       PIND.3 (int1)
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

// Примечание: порты с "кривой" адресацией (типа порт G и т.п.) не поддерживаются
// выбор команда-данные; любой пин любого порта
#define LCD_DC B, 4
// сброс экрана; любой пин любого порта
#define LCD_RST C, 3
// выход данных SPI, должен быть соответствующий пин аппаратного SPI, если он используется
#define LCD_MOSI B, 3
// тактирование SPI, должен быть соответствующий пин аппаратного SPI, если он используется
#define LCD_CLK B, 5
// ChipSelect SPI, должен быть соответствующий пин аппаратного SPI и настроен на выход
#define LCD_SS B, 2
// питание экрана; любой пин любого порта
#define LCD_PWR C, 0

/**
 *
 */
#define LCD_ROTATE

/**
 * если определено - используем программный SPI, иначе - аппаратный
 */
#define SOFT_SPI

/**
 * Высота гистограммы.
 */
#define DOSE_HISTOGRAM_SIZE 16

/**
 * Чувствительность датчика, импульсов на мкР
 */
#define SENSOR_SENSITIVITY 100
