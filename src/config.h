/**
 * \file
 * \date Создан 23.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 * \defgroup config Конфигурация
 * Здесь собраны опции, позволяющие изменить аппаратную конфигурацию устройства
 * и сделать предварительную настройку. В большинстве случаев нет небходимости
 * править другие файлы. Если нужно адаптировать программу для другого
 * микроконтроллера, то в первую очередь следует изменить файл compiler.h.
 *
 * @{
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

/*********************************************************//**
 * \defgroup config_power Опции питания
 * \ingroup config
 *
 * @{
 *************************************************************/

/**
 * Если определено - работаем c литиевого аккумулятора и меряем напряжение
 * внутри микроконтроллера. Так же происходит коррекция накачки при изменениях
 * напряжения.
 * Иначе используется вариант питания от батарейки через повышающий
 * преобразователь и АЦП меряет напряжение на внешнем входе.
 */
#ifdef __DOXYGEN__
#endif
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
#else /* Для питания от батарейки */
#define POWER_VOLTAGE_LOW       100
#define POWER_VOLTAGE_HIGH      150
#define POWER_LEVEL_ALARM       10
#endif

/** @} */

/*********************************************************//**
 * \defgroup config_charge Опции преобразователя
 * \ingroup config
 * Эти настройки затрагивают работу преобразователя напряжения для датчика
 * радиации.
 *
 * Если в схеме в качестве ключа используется MOSFET, то следует задать макрос
 * #PUMP_SWITCH_MOSFET.
 * @{
 *************************************************************/

/**
 *  Если определено - в преобразователе используется MOSFET, иначе -
 *  биполярник. Это влияет на расчёт режимов работы преобразователя в схеме с
 *  питанием от аккумулятора.
 */
#ifdef __DOXYGEN__
#endif
#define PUMP_SWITCH_MOSFET

#ifdef PUMP_SWITCH_MOSFET
/**
 * Падение напряжения на ключе преобразователя в сотых долях вольта. Для MOSFET
 *  равно нулю, для биполярного транзистора около 30-40.
 */
#define SWITCH_DROP_VOLTAGE 0
#else
#define SWITCH_DROP_VOLTAGE 35
#endif

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
 * ограничение при настройке прибора. В текущей реализации sensor.c это
 * значение не может быть больше 255.
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
 * как ограничение при настройке прибора. В текущей реализации sensor.c это
 * значение не может быть больше 255.
 */
#define PUMP_TICKS_HIT_DEFAULT          8
#define PUMP_TICKS_HIT_MIN              2
#define PUMP_TICKS_HIT_MAX              25
#else /* Для питания от батарейки */
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

/** @} */

/*********************************************************//**
 * \defgroup config_pins Определения выводов
 * \ingroup config
 * Для каждой ножки, который используется для непосредственного ввода или
 * вывода, определяется макрос. Этот макрос включает последнюю букву имени
 * порта и через запятую номер разряда от 0 до 7 включительно. Это нужно для
 * использования макросами из compiler.h.
 *
 * Пример для вывода, используемого для управления светодиодом.
 * \code
 * // Используем пин 5 порта D
 * #define OUT_LED_BLUE    D, 5
 *
 * // Где-то в программе настраиваем пин на вывод
 * _pin_off(OUT_LED_BLUE);
 * _dir_out(OUT_LED_BLUE);
 *
 * // В другом месте устанавливаем 1 на ножке
 * _pin_on(OUT_LED_BLUE);
 * \endcode
 * \see config_lcd
 * @{
 *************************************************************/

/** Синий светодиод. */
#define OUT_LED_BLUE    D, 5
/** Красный светодиод. */
#define OUT_LED_RED     D, 0
/** Динамик. */
#define OUT_BEEPER      C, 1
/** Транзистор преобразователя напряжения для датчика. */
#define OUT_PUMP_SWITCH D, 1
/** Основная кнопка, должна использовать вход внешнего прерывания 0. */
#define IN_KEY_INT      D, 2
/** Дополнительная кнопка. */
#define IN_KEY_PLUS     C, 2
/** Сигнал от датчика, должен использовать вход внешнего прерывания 1. */
#define IN_SENSOR       D, 3

/** @} */

/*********************************************************//**
 * \defgroup config_lcd Настройки дисплея
 * \ingroup config
 * Определения выводов микроконтроллера для управления дисплеем и выбор
 * конфигурации. Драйвер поддерживает и аппаратный и программный SPI, а так же
 * поворот выводимой информации на 180 градусов.
 * \see config_pins
 * @{
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
 */
#ifdef __DOXYGEN__
#define LCD_NOKIA3310_CHINA
#endif

/** @} */

/*********************************************************//**
 * \defgroup config_other Разное
 * \ingroup config
 *
 * @{
 *************************************************************/

/**
 * Максимальное значение гистограммы. Фактически определяет высоту вывода
 * гистограммы в пикселях. Высота вывода больше максимального значения на один
 * пиксель.
 */
#define DOSE_HISTOGRAM_MAX 15

/**
 * Чувствительность датчика, импульсов на мкР
 */
#define SENSOR_SENSITIVITY 100

/**
 * Язык интерфейса. Может быть RU или EN
 */
#define CFG_LOCALE RU

#ifndef F_CPU
# warning "F_CPU not defined for calculate delays"
# define F_CPU 8000000UL
#endif

/** @} */
/** @} */

#endif /* SRC_CONFIG_H_ */
