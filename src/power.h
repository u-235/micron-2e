/**
 * \file
 * \brief Интерфейс управления питанием.
 * \details
 * \date Создан 24.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#ifndef SRC_POWER_H_
#define SRC_POWER_H_

#ifdef _cplusplus
extern "C" {
#endif

/**
 * Режим работы - выключен. В этом режиме отключено всё кроме часов реального
 * времени.
 */
#define POWER_MODE_OFF  0
/**
 * Режим работы - экономичный. Радиация регистрируется, но дисплей и подсветка
 * выключены.
 */
#define POWER_MODE_SAVE 1
/**
 * Режим работы - полностью включен.
 */
#define POWER_MODE_ON   2

/**
 * Инициализация модуля. Для включения устройства нужно вызвать
 * PowerSetMode().
 */
extern void PowerInit();

/**
 * Переключение режима работы устройства.
 * \param mode Один из режимов #POWER_MODE_ON, #POWER_MODE_SAVE или
 * #POWER_MODE_OFF.
 */
extern void PowerSetMode(unsigned char mode);

/**
 * Получение режима работы устройства.
 * \return Один из режимов #POWER_MODE_ON, #POWER_MODE_SAVE или #POWER_MODE_OFF.
 */
extern unsigned char PowerGetMode();

/**
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. clock.h
 */
extern void PowerClockEvent(unsigned char event);

/**
 * Перезапуск счетчика задержки перехода в режим пониженного потребления
 * питания.
 */
void PowerStartSaveTimer();

/**
 * Максимальное значение времени задержки перехода в режим энергосбережения.
 * Это значение не может быть больше чем 255*POWER_SAVE_DELAY_STEP.
 */
#define POWER_SAVE_DELAY_MAX            600U

/**
 * Значение времени задержки перехода в режим энергосбережения по умолчанию.
 */
#define POWER_SAVE_DELAY_DEFAULT        30U

/**
 * Шаг отсчёта времени задержки перехода в режим энергосбережения. Эта величина
 * используется для уменьшения размера переменной, отвечающей за отсчёт
 * задержки.
 */
#define POWER_SAVE_DELAY_STEP           10U

/**
 * Получение времени задержки перехода в режим пониженного потребления питания.
 * \return время задержки в секундах.
 */
extern unsigned int PowerGetSaveTime();

/**
 * Установка времени задержки перехода в режим пониженного потребления питания.
 * \param tm время задержки в секундах. Если \p tm больше #POWER_SAVE_DELAY_MAX,
 *      то записывается ноль.
 */
extern void PowerSetSaveTime(unsigned int tm);

/**
 * Увеличение времени задержки перехода в режим пониженного потребления питания.
 * Для значения задержки до 60 секунд увеличение происходит на 10, далее на 60
 * секунд.Если задержка больше #POWER_SAVE_DELAY_MAX, то записывается ноль.
 */
extern void PowerIncSaveTime();

/**
 * Проверка состояния источника питания.
 * \return 0 если уровень питания в норме.
 */
extern char PowerCheck();

/**
 *
 * \return Battery voltage in hundredth of volts.
 */
extern unsigned int PowerVoltage();

/**
 *
 * \return Battery charge in percent.
 */
extern unsigned char PowerCharge();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_POWER_H_ */
