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

/**
 * \file
 * \brief
 * \details
 *
 * \date created on: 25.02.2018
 * \author: nick
 */

#include "config.h"
#include "clock.h"
#include "sensor.h"
#include "alarm.h"

#include "compiler.h"
#include "display/n3310lcd.h"
#include "screens.h"

static struct _aFlags {
        unsigned char red :1;
        unsigned char blue :1;
        unsigned char sound_enable :1;
        unsigned char beep :1;
        unsigned char beep_active :1;
        unsigned char alarm :1;
        unsigned char user_cancel_alarm :1;
        unsigned char black_light_enable :1;
} flags;

static eeprom char eeSoundEnable = 1;
static eeprom int eeAlarmLevel = 50;  // при каком уровне включать тревогу
static int alarmLevel;

extern void InitAlarm()
{
        TCCR0 = 0x05;
        TCNT0 = 0xFF;
        flags.sound_enable = _eemem_read8(&eeSoundEnable);
        alarmLevel = _eemem_read16(&eeAlarmLevel);
        flags.beep = 0;
}

extern unsigned int GetAlarmLevel()
{
        return alarmLevel;
}

extern void SetAlarmLevel(unsigned int lvl)
{
        if (lvl > 9999) {
                lvl = 0;
        }
        alarmLevel = lvl;
        _eemem_write16(&eeAlarmLevel, lvl);
}

extern void IncAlarmLevel()
{
        unsigned int lvl = alarmLevel;

        if (lvl >= 1000) {
                lvl += 1000;
        } else if (lvl >= 100) {
                lvl += 100;
        } else {
                lvl += 25;
        }
        SetAlarmLevel(lvl);
}

extern char IsAlarm()
{
        return flags.alarm;
}

extern void SetCanselAlarm(char on)
{
        flags.user_cancel_alarm = on;
}

extern char IsCanselAlarm()
{
        return flags.user_cancel_alarm;
}

extern char IsSoundEnable()
{
        return flags.sound_enable;
}

extern void SetSoundEnable(char on)
{
        flags.sound_enable = on;
        _eemem_write8(&eeSoundEnable, on);
}

extern void AsyncBeep(char on)
{
        flags.beep = on;
}

extern char IsAsyncBeep()
{
        return flags.beep;
}

extern void SetBlackLight(char on)
{
        flags.black_light_enable = on;
}

extern char IsBlackLight()
{
        return flags.black_light_enable;
}

extern void calc_counters(unsigned char second)
{
        static unsigned char error_count = 0;
        unsigned int impulse_count = SensorGetRadiation(SENSOR_INDEX_MAX + 1);

        _wdr();
        if (second > 36) {
                //проверка датчиков
                if (impulse_count == 0) {
                        error_count++;
                } else {
                        error_count = 0;
                }
                // Нет импульсов с датчика, подать сигнал.
                if (error_count > 5) {
                        alarm_sound();
                        alarm_sound();
                }
                second = 0;
        }

}

extern void CheckAlarm()
{
        unsigned int impulse_count = SensorGetRadiation(SENSOR_INDEX_MAX + 1);

        if ((impulse_count > alarmLevel) && (!flags.user_cancel_alarm)
                        && (alarmLevel > 0)) {
                flags.beep = 1;
                _interrupt_enable(INT_TIMER0_OVF);
                led_refresh(2);
                delay_ms(100);
                LcdPwrOn();
                flags.alarm = 1;

        }
        if ((impulse_count < alarmLevel) || (alarmLevel == 0)) {
                if (flags.alarm) {
                        flags.alarm = 0;
                        flags.user_cancel_alarm = 0;
                }
        }
}

/**
 *
 * \param command
 *      - 0 - выкл светодиодов
 *      - 1 - восстановление состояния перед выключением.
 *      - 2 - подсветка вкл
 */
extern void led_refresh(unsigned char command)
{
        if (LcdIsPwrDown()) {
                command = 0;
        }

        if (command == 0) {
                _pin_off(OUT_LED_BLUE);
                _pin_off(OUT_LED_RED);
                return;
        }

        if (command == 2) {
                if (flags.alarm) {
                        flags.blue = 0;
                        flags.red = 1;

                } else {
                        if (flags.black_light_enable) {
                                flags.blue = 1;
                                flags.red = 0;
                        } else {
                                flags.blue = 0;
                                flags.red = 0;
                        }
                }
        }

        if (flags.blue == 0) {
                _pin_off(OUT_LED_BLUE);
        } else {
                _pin_on(OUT_LED_BLUE);
        }

        if (flags.red == 0) {
                _pin_off(OUT_LED_RED);
        } else {
                _pin_on(OUT_LED_RED);
        }
}

//звук предупреждения о батарейке и датчике
extern void alarm_sound()
{
        int i;

        for (i = 100; i > 0; i--) {
                led_refresh(0);
                _pin_on(OUT_BEEPER);
                delay_us(50);
                _pin_off(OUT_BEEPER);
                delay_us(1200);  // тут можно менять частоту звука
        }
}

_isr_timer0_ovf(void)
{
        static unsigned char beep_count = 0;
        TCNT0 = 0xFF;
        if (!flags.beep) {
                _interrupt_disable(INT_TIMER0_OVF);
                //beep_pin = 0;
                _pin_off(OUT_BEEPER);
                led_refresh(1);
        } else {
                if (beep_count < 254) {
                        beep_count++;
                        if (!flags.beep_active) {
                                //beep_pin = 0;
                                _pin_off(OUT_BEEPER);
                                led_refresh(1);
                                flags.beep_active = 1;
                        } else {
                                led_refresh(0);
                                //beep_pin = 1;
                                _pin_on(OUT_BEEPER);
                                flags.beep_active = 0;
                        }
                } else {

                        if (flags.alarm && !flags.user_cancel_alarm) {
                                beep_count = 0;
                        } else {
                                //beep_pin = 0;
                                _pin_off(OUT_BEEPER);
                                led_refresh(1);
                                beep_count = 0;
                                _interrupt_disable(INT_TIMER0_OVF);
                                flags.beep = 0;
                        }

                }
        }
}
