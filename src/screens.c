/*****************************************************************************
 *
 *  micron 2 v 1.2.6
 *  Copyright (C) 2018  Nick Egorrov
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include "config.h"
#include <stdio.h>
#include "alarm.h"
#include "clock.h"
#include "compiler.h"
#include "display/n3310lcd.h"
#include "sensor.h"
#include "power.h"

static struct _mFlags {
        unsigned char menu_active :1;
        unsigned char need_update :1;
} flags;

/* Текстовый буфер для вывода на LCD */
static char buf[15];

extern char IsMenuActive()
{
        return flags.menu_active;
}

extern void SetMenuActive(char act)
{
        flags.menu_active = act;
}

extern char IsNeedUpdate()
{
        return flags.need_update;
}

extern void SetNeedUpdate(char act)
{
        flags.need_update = act;
}

void menu_modification_check(char selected)
{
        switch (selected) {
        case 1:
                IncAlarmLevel();
                break;
        case 2:
                IncSleepTime();
                break;
        case 3:
                SetSoundEnable(!IsSoundEnable());
                break;
        case 4:
                SensorClearDose();
                ClockClearDays();
                TIMSK = 0x41;
                AsyncBeep(1);
                delay_ms(100);
                break;
        case 5:
                ClockIncHours();
                break;
        case 6:
                ClockIncMinutes();
                break;
        case 7:
                SensorIncTicksPeriodik();
                break;
        case 8:
                SensorIncTicksHit();
                break;
        case 9:
                SensorIncPulseDuration();
                break;
        }
}
/***********************************************************************************************/

unsigned char DrawMenu(unsigned char menu_select)
{
        format(buf, PSTR("     МЕНЮ     "));
        LcdStringInv(buf, 0, 0);

        if (menu_select < 5) {
                if (GetAlarmLevel() == 0) {
                        format(buf, PSTR("Тревога  откл."));
                } else {
                        format(buf, PSTR("Тревога%4uмкР"), GetAlarmLevel());
                }

                if (menu_select == 1) {
                        LcdStringInv(buf, 0, 2);
                } else {
                        LcdString(buf, 0, 2);
                }

                if (GetSleepTime() == 0) {
                        format(buf, PSTR("Сон      откл."));
                } else {
                        format(buf, PSTR("Сон    %4uсек"), GetSleepTime());
                }
                if (menu_select == 2) {
                        LcdStringInv(buf, 0, 3);
                } else {
                        LcdString(buf, 0, 3);
                }

                if (IsSoundEnable() == 0) {
                        format(buf, PSTR("Звук     откл."));
                } else {
                        format(buf, PSTR("Звук      вкл."));
                }

                if (menu_select == 3) {
                        LcdStringInv(buf, 0, 4);
                } else {
                        LcdString(buf, 0, 4);
                }

                format(buf, PSTR("Сброс  дозы   "));
                if (menu_select == 4) {
                        LcdStringInv(buf, 0, 5);
                } else {
                        LcdString(buf, 0, 5);
                }

        } else if ((menu_select > 4) && (menu_select < 7)) {  //страница меню2

                format(buf, PSTR("Часы     %2u:  "), ClockGetHours());
                if (menu_select == 5) {
                        LcdStringInv(buf, 0, 2);
                } else {
                        LcdString(buf, 0, 2);
                }

                format(buf, PSTR("Минуты     :%2u"), ClockGetMimutes());
                if (menu_select == 6) {
                        LcdStringInv(buf, 0, 3);
                } else {
                        LcdString(buf, 0, 3);
                }
        } else {  // страница меню3

                format(buf, PSTR("Накачка %3uимп"), SensorGetTicksPeriodik());
                if (menu_select == 7) {
                        LcdStringInv(buf, 0, 2);
                } else {
                        LcdString(buf, 0, 2);
                }

                format(buf, PSTR("При имп.%3uимп"), SensorGetTicksHit());
                if (menu_select == 8) {
                        LcdStringInv(buf, 0, 3);
                } else {
                        LcdString(buf, 0, 3);
                }

                format(buf, PSTR("Импульс %3uмкс"), SensorGetPulseDuration());
                if (menu_select == 9) {
                        LcdStringInv(buf, 0, 4);
                } else {
                        LcdString(buf, 0, 4);
                }
        }

        LcdUpdate();
        if (menu_select > 9) {
                flags.menu_active = 0;
                menu_select = 1;
                flags.need_update = 1;
        }
        return menu_select;
}
/***********************************************************************************************/

void main_window_draw(unsigned char chrg_tick)
{
        unsigned char i, j;

        /*
         if (flags.imp) {

         //    format (buf, "#");
         //    LcdString(buf, 14,5);

         if ((!LcdIsPwrDown()) & (flags.sound_bit)) {
         flags.beep = 1;
         TIMSK = 0x41;
         led_refresh(1);

         }
         flags.imp = 0;
         }
         */

        format(buf, PSTR("%5u"), SensorGetRadiation(SENSOR_INDEX_MAX + 1));
        LcdStringBold(buf, 0, 1);
        format(buf, PSTR("мкР"));
        LcdString(buf, 11, 1);
        format(buf, PSTR(" ч "));
        LcdString(buf, 11, 2);
        LcdLine(66, 16, 83, 16, 1);

        //    LcdLine (0,23, 30,23, 1);

        j = 0;
        for (i = 0; i <= SENSOR_INDEX_MAX; i++) {
                unsigned char lvl = SensorGetRelative(i);
                LcdLine(j, 38, j, 38 - lvl, 1);
                j++;
                LcdLine(j, 38, j, 38 - lvl, 1);
                j++;
        }
        LcdLine(0, 38, 36, 38, 1);

        format(buf, PSTR("%02u:%02u:%02u"), ClockGetHours(), ClockGetMimutes(),
                        ClockGetSeconds());  //часы
        LcdString(buf, 0, 0);
        // LcdLine (0,8, 83,8, 1);

//  .Батарейка
        if (chrg_tick > 5) {
                format(buf, PSTR("^ %03u$"), GetCharge());
        } else {
                format(buf, PSTR("^%01u.%02uv"), GetVoltage() / 10,
                                GetVoltage() % 10);
        }
        LcdString(buf, 8, 0);

        if (chrg_tick <= 3) {
                format(buf, PSTR("За час%5luмкР"), SensorGetDoseHour());
        } else if (chrg_tick > 3 && chrg_tick <= 6) {
                format(buf, PSTR("Сут.%7luмкР"), SensorGetDoseDay());
        } else if (chrg_tick > 6) {
                //(all_doze + day_doze) так как используем мкР для дозы, то выделил 7 разрядов
                // то есть максим. отобр. значение - 9999999 мкР, при ЕРФ - 76 лет работы :D, при жизни возле 4-го  блока ЧАЭС - полгода  минимум!
                //
                format(buf, PSTR("%3uД%7luмкР"), ClockGetDays(),
                                SensorGetDoseAll());
        }
        LcdString(buf, 0, 5);

        LcdUpdate();
}

extern void DrawIntro()
{
        LcdClear();
        format(buf, PSTR("   Микрон-2   "));
        LcdStringInv(buf, 0, 2);
        LcdUpdate();
        format(buf, "Загрузка");
        LcdString(buf, 0, 5);
        LcdUpdate();
}

extern void DrawBay()
{
        LcdClear();
        format(buf, PSTR("  Выключение  "));
        LcdStringInv(buf, 0, 2);
        LcdUpdate();
}
