/**
 * \file
 * \brief
 * \details
 * \warning Codepage utf-8
 *
 * \date создан 17.06.2018
 * \author Nick Egorrov
 */

#include "../compiler.h"
#include "../config.h"

#undef CFG_LOCALE
#define CFG_LOCALE EN

#include "../msg.h"

/*************************************************************
 *      menu
 *************************************************************/

flash const char _LOCAL_NAME(msgMenuTitle)[] = "     MENU     ";
flash const char _LOCAL_NAME(msgMenuAlarmOff)[] = "Alarm      off";
flash const char _LOCAL_NAME(msgMenuAlarm)[] = "Alarm   %4umkR";

flash const char _LOCAL_NAME(msgMenuSleepOff )[] = "Sleep      off";
flash const char _LOCAL_NAME(msgMenuSleep )[] = "Sleep   %4usec";

flash const char _LOCAL_NAME(msgMenuSoundOff )[] = "Sound      off";
flash const char _LOCAL_NAME(msgMenuSoundOn )[] = "Sound       on";

flash const char _LOCAL_NAME(msgMenuClearDose )[] = "Clear dose    ";

flash const char _LOCAL_NAME(msgMenuSetHours )[] = "Hours   %2u:  ";
flash const char _LOCAL_NAME(msgMenuSetMinutes )[] = "Minutes   :%2u";

flash const char _LOCAL_NAME(msgMenuSetTiksPeriodic )[] = "Pump   %3u imp";
flash const char _LOCAL_NAME(msgMenuSetTiksHit )[] = "Hit    %3u imp";
flash const char _LOCAL_NAME(msgMenuSetPulseDuration )[] = "Impulse %3umks";

/*************************************************************
 *      window
 *************************************************************/

flash const char _LOCAL_NAME(msgWindowRadiation )[] = "%5u";
flash const char _LOCAL_NAME(msgWindowMCR )[] = "mkR";
flash const char _LOCAL_NAME(msgWindowHour )[] = " h ";

flash const char _LOCAL_NAME(msgWindowTime )[] = "%02u:%02u:%02u";

flash const char _LOCAL_NAME(msgWindowCharge )[] = "^ %03u$";
flash const char _LOCAL_NAME(msgWindowVoltage )[] = "^%01u.%02uv";

flash const char _LOCAL_NAME(msgWindowDoseHour )[] = "hour %5lumkR";
flash const char _LOCAL_NAME(msgWindowDoseDay )[] = "day%7lumkR";
flash const char _LOCAL_NAME(msgWindowDoseAll )[] = "%3uD%7lumkR";

/*************************************************************
 *      prologue/epilogue
 *************************************************************/

flash const char _LOCAL_NAME(msgProloqueTitle )[] = "   MICRON-2e  ";
flash const char _LOCAL_NAME(msgProloqueLoad )[] = " ** start  ** ";

flash const char _LOCAL_NAME(msgEpiloque )[] = "  power off   ";

/*************************************************************
 *      alerts
 *************************************************************/

flash const char _LOCAL_NAME(msgAlertDanger )[] = "    DANGER    ";
flash const char _LOCAL_NAME(msgAlertRadiation )[] = "  RADIATION!  ";

flash const char _LOCAL_NAME(msgAlertPower )[] = "  LOW CHARGE!  ";
