/**
 * \file
 * \brief
 * \details
 *
 * \date Создан 25.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#ifndef SRC_USER_H_
#define SRC_USER_H_

#ifdef _cplusplus
extern "C" {
#endif

#define USER_KEY_OK_DOWN        0x02
#define USER_KEY_OK_UP          0x03
#define USER_KEY_MENU_DOWN      0x04
#define USER_KEY_MENU_UP        0x05
#define USER_KEY_POWER_DOWN     0x06
#define USER_KEY_POWER_UP       0x07
#define USER_KEY_PLUS_DOWN      0x12
#define USER_KEY_PLUS_UP        0x13

#define USER_ALERT_DOSE         1U
#define USER_ALERT_SENSOR       2U
#define USER_ALERT_POWER        3U

/**
 * Включение, инициализация и загрузка настроек из EEPROM.
 */
extern void UserOn();

extern void UserSleep();

#define UserOff()

/**
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
extern void UserClockEvent(unsigned char event);

extern unsigned char UserGetKey();

extern void UserAlert(unsigned char alert);

extern void UserAlertCansel(unsigned char alert);

extern char UserIsSoundEnable();

extern void UserSetSoundEnable(char on);

/**
 *
 * \param command
 *      - 0 - выкл светодиодов
 */
extern void UserLight(unsigned char command);

#ifdef _cplusplus
}
#endif

#endif /* SRC_USER_H_ */
