/**
 * \file
 * \brief
 * \details
 * \warning Codepage cp-1251
 *
 * \date создан 16.06.2018
 * \author Nick Egorrov
 */

#include "../compiler.h"
#include "../config.h"

/* Что бы имена строк в этом файле всегда заканчивались на RU */
#undef CFG_LOCALE
#define CFG_LOCALE RU

#include "../msg.h"

/*************************************************************
 *      menu
 *************************************************************/

flash const char _LOCAL_NAME(msgMenuTitle)[] = "     МЕНЮ     ";
flash const char _LOCAL_NAME(msgMenuAlarmOff)[] = "Тревога  откл.";
flash const char _LOCAL_NAME(msgMenuAlarm)[] = "Тревога%4uмкР";

flash const char _LOCAL_NAME(msgMenuSleepOff )[] = "Сон      откл.";
flash const char _LOCAL_NAME(msgMenuSleep )[] = "Сон    %4uсек";

flash const char _LOCAL_NAME(msgMenuSoundOff )[] = "Звук     откл.";
flash const char _LOCAL_NAME(msgMenuSoundOn )[] = "Звук      вкл.";

flash const char _LOCAL_NAME(msgMenuClearDose )[] = "Сброс  дозы   ";

flash const char _LOCAL_NAME(msgMenuSetHours )[] = "Часы     %2u:  ";
flash const char _LOCAL_NAME(msgMenuSetMinutes )[] = "Минуты     :%2u";

flash const char _LOCAL_NAME(msgMenuSetTiksPeriodic )[] = "Накачка %3uимп";
flash const char _LOCAL_NAME(msgMenuSetTiksHit )[] = "При имп.%3uимп";
flash const char _LOCAL_NAME(msgMenuSetPulseDuration )[] = "Импульс %3uмкс";

/*************************************************************
 *      window
 *************************************************************/

flash const char _LOCAL_NAME(msgWindowRadiation )[] = "%5u";
flash const char _LOCAL_NAME(msgWindowMCR )[] = "мкР";
flash const char _LOCAL_NAME(msgWindowHour )[] = " ч ";

flash const char _LOCAL_NAME(msgWindowTime )[] = "%02u:%02u:%02u";

flash const char _LOCAL_NAME(msgWindowCharge )[] = "^ %03u$";
flash const char _LOCAL_NAME(msgWindowVoltage )[] = "^%01u.%02uv";

flash const char _LOCAL_NAME(msgWindowDoseHour )[] = "За час%5luмкР";
flash const char _LOCAL_NAME(msgWindowDoseDay )[] = "Сут.%7luмкР";
flash const char _LOCAL_NAME(msgWindowDoseAll )[] = "%3uД%7luмкР";

/*************************************************************
 *      prologue/epilogue
 *************************************************************/

flash const char _LOCAL_NAME(msgProloqueTitle )[] = "   Микрон-2е  ";
flash const char _LOCAL_NAME(msgProloqueLoad )[] = "Загрузка";

flash const char _LOCAL_NAME(msgEpiloque )[] = "  Выключение  ";

/*************************************************************
 *      alerts
 *************************************************************/

flash const char _LOCAL_NAME(msgAlertDanger )[] = "   ОПАСНОЕ    ";
flash const char _LOCAL_NAME(msgAlertRadiation )[] = "  ИЗЛУЧЕНИЕ!  ";

flash const char _LOCAL_NAME(msgAlertPower )[] = " НИЗКИЙ ЗАРЯД!";
