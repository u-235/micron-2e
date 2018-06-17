/**
 * \file
 * \brief
 * \details
 * \warning Codepage cp-1251
 *
 * \date ������ 16.06.2018
 * \author Nick Egorrov
 */

#include "../compiler.h"
#include "../config.h"

/* ��� �� ����� ����� � ���� ����� ������ ������������� �� RU */
#undef CFG_LOCALE
#define CFG_LOCALE RU

#include "../msg.h"

/*************************************************************
 *      menu
 *************************************************************/

flash const char _LOCAL_NAME(msgMenuTitle)[] = "     ����     ";
flash const char _LOCAL_NAME(msgMenuAlarmOff)[] = "�������  ����.";
flash const char _LOCAL_NAME(msgMenuAlarm)[] = "�������%4u���";

flash const char _LOCAL_NAME(msgMenuSleepOff )[] = "���      ����.";
flash const char _LOCAL_NAME(msgMenuSleep )[] = "���    %4u���";

flash const char _LOCAL_NAME(msgMenuSoundOff )[] = "����     ����.";
flash const char _LOCAL_NAME(msgMenuSoundOn )[] = "����      ���.";

flash const char _LOCAL_NAME(msgMenuClearDose )[] = "�����  ����   ";

flash const char _LOCAL_NAME(msgMenuSetHours )[] = "����     %2u:  ";
flash const char _LOCAL_NAME(msgMenuSetMinutes )[] = "������     :%2u";

flash const char _LOCAL_NAME(msgMenuSetTiksPeriodic )[] = "������� %3u���";
flash const char _LOCAL_NAME(msgMenuSetTiksHit )[] = "��� ���.%3u���";
flash const char _LOCAL_NAME(msgMenuSetPulseDuration )[] = "������� %3u���";

/*************************************************************
 *      window
 *************************************************************/

flash const char _LOCAL_NAME(msgWindowRadiation )[] = "%5u";
flash const char _LOCAL_NAME(msgWindowMCR )[] = "���";
flash const char _LOCAL_NAME(msgWindowHour )[] = " � ";

flash const char _LOCAL_NAME(msgWindowTime )[] = "%02u:%02u:%02u";

flash const char _LOCAL_NAME(msgWindowCharge )[] = "^ %03u$";
flash const char _LOCAL_NAME(msgWindowVoltage )[] = "^%01u.%02uv";

flash const char _LOCAL_NAME(msgWindowDoseHour )[] = "�� ���%5lu���";
flash const char _LOCAL_NAME(msgWindowDoseDay )[] = "���.%7lu���";
flash const char _LOCAL_NAME(msgWindowDoseAll )[] = "%3u�%7lu���";

/*************************************************************
 *      prologue/epilogue
 *************************************************************/

flash const char _LOCAL_NAME(msgProloqueTitle )[] = "   ������-2�  ";
flash const char _LOCAL_NAME(msgProloqueLoad )[] = "��������";

flash const char _LOCAL_NAME(msgEpiloque )[] = "  ����������  ";

/*************************************************************
 *      alerts
 *************************************************************/

flash const char _LOCAL_NAME(msgAlertDanger )[] = "   �������    ";
flash const char _LOCAL_NAME(msgAlertRadiation )[] = "  ���������!  ";

flash const char _LOCAL_NAME(msgAlertPower )[] = " ������ �����!";
