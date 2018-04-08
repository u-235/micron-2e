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

/**
 * Инициализация. Загрузка настрое из EEPROM.
 */
extern void UserInit();

extern unsigned int UserGetAlarmLevel();

extern void UserSetAlarmLevel(unsigned int lvl);

extern void UserIncAlarmLevel();

extern void UserAlarmCheck();

extern char UserIsAlarm();

extern void UserSetCanselAlarm(char on);

extern char UserIsCanselAlarm();

/**
 *
 * \return Ноль если звук выключен
 */
extern char UserIsSoundEnable();

extern void UserAsyncBeep(char on);

extern char UserIsAsyncBeep();

extern void UserSetBlackLight(char on);

extern char UserIsBlackLight();

/**
 *
 * \param on Ноль для выключения звука.
 */
extern void UserSetSoundEnable(char on);

extern void UserSensorCheck(unsigned char second);

/**
 *
 * \param command
 *      - 0 - выкл светодиодов
 *      - 1 - восстановление состояния перед выключением.
 *      - 2 - тревога, красный цвет
 *      - 3 - подсветка вкл
 */
extern void UserLight(unsigned char command);

extern void UserAlarmSound();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_USER_H_ */
