/**
 * \file
 * \brief
 * \details
 *
 * \author Nick Egorrov
 * \date created on: 24.02.2018
 * \copyright GNU Public License 3
 */

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_

#ifdef _cplusplus
extern "C" {
#endif

/**
 * Состояние счетчика секунд изменилось.
 */
#define CLOCK_EVENT_SECOND 0x01
/**
 * Состояние счетчика минут изменилось.
 */
#define CLOCK_EVENT_MINUTE 0x02
/**
 * Состояние счетчика часов изменилось.
 */
#define CLOCK_EVENT_HOUR   0x04
/**
 * Состояние счетчика дней изменилось.
 */
#define CLOCK_EVENT_DAY    0x08
/**
 * Произошло событие доли секунды.
 */
#define CLOCK_EVENT_PORTION 0x10
/**
 * Сколько событий #CLOCK_EVENT_PORTION происходит за секунду
 */
#define CLOCK_DIVIDER 4U

/**
 * Инициализация часов.
 */
extern void ClockOn();

#define ClockSleep()
#define ClockOff()

/**
 * \brief Было ли обновление.
 * \details Показывает, изменилось ли состояние счётчиков с момента предыдущего
 * вызова функции.
 * \return Набор флагов CLOCK_EVENT_xx если состояние часов обновилось или ноль.
 */
extern unsigned char ClockGetEvent();

/**
 * \brief Возвращает секунды.
 * \details Функция возвращает значение счетчика секунд.
 * \return Значение секунд от 0 до 59.
 */
extern unsigned char ClockGetSeconds();

/**
 * \brief Устанавливает секунды.
 * \details Функция изменяет значение счетчика секунд, если новое значение
 * находится в диапазоне от 0 до 59 включительно.
 * \param s Новое значение секунд от 0 до 59.
 * \return Действительное значение счетчика секунд.
 */
extern unsigned char ClockSetSeconds(unsigned char s);

/**
 * \brief Увеличивает секунды.
 * \details Функция увеличивает счётчик секунд на 1 и, при достижении значения
 * 60, сбрасывает его в ноль.
 * \return Значение секунд от 0 до 59.
 */
extern unsigned char ClockIncSeconds();

/**
 * \brief Возвращает минуты.
 * \details Функция возвращает значение счетчика минут.
 * \return Значение минут от 0 до 59.
 */
extern unsigned char ClockGetMimutes();

/**
 * \brief Устанавливает минуты.
 * \details Функция изменяет значение счетчика минут, если новое значение
 * находится в диапазоне от 0 до 59 включительно.
 * \param m Новое значение минут от 0 до 59.
 * \return Действительное значение счетчика минут.
 */
extern unsigned char ClockSetMinutes(unsigned char m);

/**
 * \brief Увеличивает минуты.
 * \details Функция увеличивает счётчик минут на 1 и, при достижении значения
 * 60, сбрасывает его в ноль.
 * \return Значение минут от 0 до 59.
 */
extern unsigned char ClockIncMinutes();

/**
 * \brief Возвращает часы.
 * \details Функция возвращает значение счетчика часов.
 * \return Значение часов от 0 до 59.
 */
extern unsigned char ClockGetHours();

/**
 * \brief Устанавливает часы.
 * \details Функция изменяет значение счетчика часов, если новое значение
 * находится в диапазоне от 0 до 23 включительно.
 * \param h Новое значение часов от 0 до 23.
 * \return Действительное значение счетчика часов.
 */
extern unsigned char ClockSetHours(unsigned char h);

/**
 * \brief Увеличивает часы.
 * \details Функция увеличивает счётчик часов на 1 и, при достижении значения
 * 24, сбрасывает его в ноль.
 * \return Значение часов от 0 до 23.
 */
extern unsigned char ClockIncHours();

/**
 * \brief Возвращает дни.
 * \details Функция возвращает значение счетчика дней.
 * \return Значение дней.
 */
extern unsigned int ClockGetDays();

/**
 * Сбрасывает счётчик дней.
 */
extern void ClockClearDays();

#ifdef _cplusplus
}
#endif

#endif /* SRC_CLOCK_H_ */
