/**
 * \file
 * \brief
 * \details
 *
 * \date создан 16.06.2018
 * \author Nick Egorrov
 */

#ifndef SRC_MSG_H_
#define SRC_MSG_H_

#ifdef _cplusplus
extern "C" {
#endif

/* Макросы для добавления к имени суффикса из CFG_LOCALE */
#define _LOCAL_NAME(msg) __LOCAL_NAME(msg, CFG_LOCALE)
#define __LOCAL_NAME(msg, sf) ___LOCAL_NAME(msg, sf)
#define ___LOCAL_NAME(msg, sf) msg ## _ ## sf

/*************************************************************
 *      menu
 *************************************************************/

extern flash const char _LOCAL_NAME(msgMenuTitle) [];
extern flash const char _LOCAL_NAME(msgMenuAlarmOff) [];
extern flash const char _LOCAL_NAME(msgMenuAlarm) [];

extern flash const char _LOCAL_NAME(msgMenuSleepOff) [];
extern flash const char _LOCAL_NAME(msgMenuSleep) [];

extern flash const char _LOCAL_NAME(msgMenuSoundOff) [];
extern flash const char _LOCAL_NAME(msgMenuSoundOn) [];

extern flash const char _LOCAL_NAME(msgMenuClearDose) [];

extern flash const char _LOCAL_NAME(msgMenuSetHours) [];
extern flash const char _LOCAL_NAME(msgMenuSetMinutes) [];

extern flash const char _LOCAL_NAME(msgMenuSetTiksPeriodic) [];
extern flash const char _LOCAL_NAME(msgMenuSetTiksHit) [];
extern flash const char _LOCAL_NAME(msgMenuSetPulseDuration) [];

/*************************************************************
 *      window
 *************************************************************/

extern flash const char _LOCAL_NAME(msgWindowRadiation) [];
extern flash const char _LOCAL_NAME(msgWindowMCR) [];
extern flash const char _LOCAL_NAME(msgWindowHour) [];

extern flash const char _LOCAL_NAME(msgWindowTime) [];

extern flash const char _LOCAL_NAME(msgWindowCharge) [];
extern flash const char _LOCAL_NAME(msgWindowVoltage) [];

extern flash const char _LOCAL_NAME(msgWindowDoseHour) [];
extern flash const char _LOCAL_NAME(msgWindowDoseDay) [];
extern flash const char _LOCAL_NAME(msgWindowDoseAll) [];

/*************************************************************
 *      prologue/epilogue
 *************************************************************/

extern flash const char _LOCAL_NAME(msgProloqueTitle) [];
extern flash const char _LOCAL_NAME(msgProloqueLoad) [];

extern flash const char _LOCAL_NAME(msgEpiloque) [];

/*************************************************************
 *      alerts
 *************************************************************/

extern flash const char _LOCAL_NAME(msgAlertDanger) [];
extern flash const char _LOCAL_NAME(msgAlertRadiation) [];

extern flash const char _LOCAL_NAME(msgAlertPower) [];

#ifdef _cplusplus
}
#endif

#endif /* SRC_MSG_H_ */
