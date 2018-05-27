/**
 * \file
 * \brief Объявления для графического интерфейса.
 * \details
 *
 * \date Создан 24.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#ifndef SRC_SCREENS_H_
#define SRC_SCREENS_H_

#ifdef _cplusplus
extern "C" {
#endif

/**
 * Константа для отображения основного окна.
 */
#define SCREEN_VIEW_MAIN        1U

/**
 * Константа для отображения меню.
 */
#define SCREEN_VIEW_MENU        2U

/**
 * Константа для отображения сообщения при включении.
 */
#define SCREEN_VIEW_INTRO       3U

/**
 * Константа для отображения сообщения при выключении.
 */
#define SCREEN_VIEW_BAY         4U

/**
 * Константа для отображения тревожного сообщения высокого уровня радиации.
 */
#define SCREEN_VIEW_ALERT_DOSE  5U

/**
 * Константа для отображения тревожного сообщения при разряде батарейки.
 */
#define SCREEN_VIEW_ALERT_POWER 6U

/**
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. clock.h
 */
extern void ScreenClockEvent(unsigned char event);

/**
 * Изменяет (если это возможно) отображаемый экран. Для отмены этого используйте
 * ScreenHide(). Эти функции только меняют внутреннее состояние модуля, что бы
 * изменения появились вызовите ScreenDraw().
 * \param view Требуемый экран, может быть одним из #SCREEN_VIEW_ALERT_DOSE,
 * #SCREEN_VIEW_ALERT_POWER, #SCREEN_VIEW_BAY, #SCREEN_VIEW_INTRO,
 * #SCREEN_VIEW_MAIN, #SCREEN_VIEW_MENU.
 */
extern void ScreenShow(unsigned char view);

/**
 * Отменяет (если это возможно) результат ScreenShow(). Эти функции только
 * меняют внутреннее состояние модуля, что бы изменения появились вызовите
 * ScreenDraw().
 * \param view Отменяемый экран, может быть одним из #SCREEN_VIEW_ALERT_DOSE,
 * #SCREEN_VIEW_ALERT_POWER, #SCREEN_VIEW_BAY, #SCREEN_VIEW_INTRO,
 * #SCREEN_VIEW_MAIN, #SCREEN_VIEW_MENU.
 */
extern void ScreenHide(unsigned char view);

/**
 * Перерисовка экрана.
 */
extern void ScreenDraw();

/**
 * Обработка кнопок текущим экраном.
 * \param key Код состояния кнопки, см. USER_KEY_xx в файле user.h
 * \see ScreenShow()
 */
extern void ScreenHandleKey(unsigned char key);

#ifdef _cplusplus
}
#endif

#endif /* SRC_SCREENS_H_ */
