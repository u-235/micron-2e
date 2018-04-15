/**
 * \file
 * \brief Интерфейс датчика
 * \date Создан 24.02.2018
 * \author: Nick Egorrov
 * \copyright GNU Public License 3
 * \details
 */

#ifndef SRC_SENSOR_H_
#define SRC_SENSOR_H_

#include "clock.h"

#ifdef _cplusplus
extern "C" {
#endif

#define SENSOR_INDEX_MAX 35

/**
 * Инициализация датчика. Параметры настройки извлекаются из EEPROM и
 * запускается начальный цикл накачки.
 */
extern void SensorInit();

/**
 * Обновление данных датчика в зависимости от времени.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
extern void SensorClockEvent(unsigned char event);

/**
 * Проверяет, была ли сработка датчика с момента предыдущего вызова функции.
 * \return Ненулевое значение если была сработка датчика.
 */
extern unsigned char SensorIsHit();

/**
 * Возвращает относительный уровень радиации.
 * \param index Индекс значения уровня. Диапазон от 0 до #SENSOR_INDEX_MAX.
 * \return Относительный уровень радиации в диапазоне от 0 до
 *       #DOSE_HISTOGRAM_MAX
 */
extern unsigned char SensorGetRelative(unsigned char index);

/**
 * Возвращает уровень радиации, выше которого функция SensorIsAlarm() выдаст
 * тревогу.
 * \return Уровень радиации в мкР/час.
 */
extern unsigned int SensorGetAlarmLevel();

/**
 * Устанавливает уровень радиации, выше которого функция SensorIsAlarm() выдаст
 * тревогу.
 * \param lvl Уровень радиации в мкР/час. При превышении значения 9999
 *      сбрасывается в ноль.
 */
extern void SensorSetAlarmLevel(unsigned int lvl);

/**
 * Увеличивает уровень радиации, выше которого функция SensorIsAlarm() выдаст
 * тревогу.
 * \param lvl Уровень радиации в мкР/час. При превышении значения 9999
 *      сбрасывается в ноль.
 */
extern void SensorIncAlarmLevel();

/**
 * Показывает, был ли превышен уровень радиации. Проверка происходит по каждому
 * секундному событию от часов.
 * \return Ноль если превышения не было.
 */
extern char SensorIsAlarm();

/**
 * Показывает состояние дтчика.
 * \return Ноль если датчик исправен.
 */
extern char SensorError();

/*************************************************************
 *      Dose section
 *************************************************************/

/**
 * Возвращает уровень радиации за указанный период в мкР/ч
 * \param period Период в секундах, от 1 до #SENSOR_INDEX_MAX + 1
 * \return Уровень радиации в мкР/ч.
 */
extern unsigned int SensorGetRadiation(unsigned char period);

/**
 * Возвращает измеренную дозу за последний завершившийся час.
 * \return Доза в мкР.
 */
extern unsigned long SensorGetDoseHour();

/**
 * Возвращает измеренную дозу за последние завершившиеся сутки с момента
 * сброса измерений функцией SensorClearDose().
 * \return Доза в мкР.
 */
extern unsigned long SensorGetDoseDay();

/**
 * Возвращает измеренную дозу за всё время с момента сброса измерений функцией
 * SensorClearDose().
 * \return Доза в мкР.
 */
extern unsigned long SensorGetDoseAll();

/**
 * Сброс измерений дозы.
 */
extern void SensorClearDose();

/*************************************************************
 *      Setup section
 *************************************************************/

/**
 * Возвращает длительность импульса накачки преобразователя; задаётся в
 * количестве циклов в RunCharge(), точное время неизвестно.
 * \return Длительность импульса накачки преобразователя.
 */
extern unsigned char SensorGetPulseDuration();

/**
 * Увеличивает длительность импульса накачки преобразователя. При достижении
 * #PUMP_PULSE_WIDTH_MAX сбрасывается в #PUMP_PULSE_WIDTH_MIN.
 * \see SensorGetPulseDuration()
 */
extern void SensorIncPulseDuration();

/**
 * Возвращает количество импульсов накачки при запуске от таймера.
 * \return Количество импульсов накачки.
 */
extern unsigned int SensorGetTicksPeriodik();

/**
 * Увеличивает количество импульсов накачки преобразователя при запуске от
 * таймера. При достижении #PUMP_TICKS_PERIODIK_MAX сбрасывается в
 * #PUMP_TICKS_PERIODIK_MIN.
 * \see SensorGetTicksPeriodik()
 */
extern void SensorIncTicksPeriodik();

/**
 * Возвращает количество импульсов накачки при сработке датчика.
 * \return Количество импульсов накачки.
 */
extern unsigned int SensorGetTicksHit();

/**
 * Увеличивает количество импульсов накачки преобразователя при сработке
 * датчика. При достижении #PUMP_TICKS_HIT_MAX сбрасывается в
 * #PUMP_TICKS_HIT_MIN.
 * \see SensorGetTicksHit()
 */
extern void SensorIncTicksHit();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_SENSOR_H_ */
