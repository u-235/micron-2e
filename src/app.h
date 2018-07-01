/**
 * \file
 * \brief
 * \details
 *
 * \author Nick Egorrov
 * \date created on: 16.06.2018
 * \copyright GNU Public License 3
 */

#ifndef SRC_APP_H_
#define SRC_APP_H_

#ifdef _cplusplus
extern "C" {
#endif

/**
 * Режим работы - выключен. В этом режиме отключено всё кроме часов реального
 * времени.
 */
#define APP_MODE_OFF  0
/**
 * Режим работы - экономичный. Радиация регистрируется, но дисплей и подсветка
 * выключены.
 */
#define APP_MODE_SAVE 1
/**
 * Режим работы - полностью включен.
 */
#define APP_MODE_ON   2

/**
 * Максимальное значение времени задержки перехода в режим энергосбережения.
 * Это значение не может быть больше чем 255*APP_SAVE_DELAY_STEP.
 */
#define APP_SAVE_DELAY_MAX            600U

/**
 * Значение времени задержки перехода в режим энергосбережения по умолчанию.
 */
#define APP_SAVE_DELAY_DEFAULT        30U

/**
 * Шаг отсчёта времени задержки перехода в режим энергосбережения. Эта величина
 * используется для уменьшения размера переменной, отвечающей за отсчёт
 * задержки.
 */
#define APP_SAVE_DELAY_STEP           10U

/**
 * Change device mode.
 * \param mode One of #APP_MODE_ON, #APP_MODE_SAVE or
 * #APP_MODE_OFF.
 */
extern void AppSetMode(unsigned char mode);

/**
 * Получение времени задержки перехода в режим пониженного потребления питания.
 * \return время задержки в секундах.
 */
extern unsigned int AppGetSaveTime();

/**
 * Установка времени задержки перехода в режим пониженного потребления питания.
 * \param tm время задержки в секундах. Если \p tm больше #APP_SAVE_DELAY_MAX,
 *      то записывается ноль.
 */
extern void AppSetSaveTime(unsigned int tm);

/**
 * Увеличение времени задержки перехода в режим пониженного потребления питания.
 * Для значения задержки до 60 секунд увеличение происходит на 10, далее на 60
 * секунд.Если задержка больше #APP_SAVE_DELAY_MAX, то записывается ноль.
 */
extern void AppIncSaveTime();

#ifdef _cplusplus
}
#endif

#endif /* SRC_APP_H_ */
