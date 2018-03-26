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
#include "clock.h"
#include "sensor.h"
#include "alarm.h"
#include "compiler.h"
#include "display/n3310lcd.h"
#include "screens.h"
#include "power.h"

static void InitHard();

static char menu_select = 1;
static unsigned char second = 0;
static unsigned char chrg_tick = 0;

static struct _mFlags {
        unsigned char poweroff_bit :1;
        unsigned char imp :1;
        unsigned char impulse_recive :1;
} flags;

static unsigned char key_press = 0;
static unsigned int display_time = 0;

//********************************************************************

void main(void)
{
        InitHard();
        InitPower();
        InitAlarm();
        InitClock();

        //beep_pin = 0;
        _pin_off(OUT_BEEPER);
        SetMenuActive(0);
        led_refresh(2);
        LcdInit();
        delay_ms(100);
        led_refresh(2);
        delay_ms(100);
        LcdPwrOn();
        delay_ms(100);

        DrawIntro();

        if (CheckPower() != 0) {
                alarm_sound();
                delay_ms(50);
                alarm_sound();  //звук предупреждения о батарейке - 3 пика
                delay_ms(50);
                alarm_sound();
        }

        _cli();
        InitSensor();
        _sei();
        //разрешаем прерывания

        LcdClear();
///////////////////////////////////////////////////////////
        while (1) {
                unsigned char event = ClockGetEvent();
                if (event) {
                        second++;
                        chrg_tick++;
                        if (chrg_tick > 10) {
                                chrg_tick = 0;
                        }
                        GICR = 0x00;
                        led_refresh(0);
                        SensorClockEvent(event);
                        delay_us(25);  //25
                        led_refresh(1);
                        GICR = 0xC0;
                }

                        if (flags.poweroff_bit) {
                                TIMSK = 0x40;
                                AsyncBeep(0);
                                _pin_off(OUT_BEEPER);
                                DrawBay();
                                delay_ms(4000);
                                poweroff();
                                // TODO bad
                                while (flags.poweroff_bit) {
                                        _sleep();
                                }
                        }

                if (IsNeedUpdate()) {
                        SetNeedUpdate(0);
                        calc_counters(1); // TODO IT'S NOT WORK
                        _wdr();

                        CheckAlarm();
                        if ((_is_pin_clean(IN_KEY_PLUS)) && IsMenuActive()) {
                                menu_modification_check(menu_select);
                                display_time = 0;
                                delay_ms(300);
                        }

                        led_refresh(2);

                        if (!LcdIsPwrDown()) {
                                LcdClear();
                                if (IsMenuActive()) {
                                        menu_select = DrawMenu(menu_select);
                                } else {
                                        main_window_draw(chrg_tick);
                                }
                        }

                }

                if (!IsMenuActive()) {
                        if (!IsAsyncBeep()) {
                                _sleep();
                        }
                        _wdr();
                } else {
                        SetNeedUpdate(1);
                }
        }
}
//********************************************************************

_isr_ext0(void)
{
        char key2_pressed = 0;
        int i;

        SetNeedUpdate(1);
        _pin_off(OUT_PUMP_SWITCH);

        if (!LcdIsPwrDown()) {
                if (IsAlarm() && (!IsCanselAlarm())) {
                        SetCanselAlarm(1);
                        AsyncBeep(0);
                        _pin_off(OUT_BEEPER);
                        led_refresh(1);
                        TIMSK = 0x40;
                } else {
                        if (IsMenuActive()) {
                                menu_select++;
                        }
                }
        }

        UpdateWorkTime(0);
        if (TIMSK != 0x00) LcdPwrOn();
        while ((_is_pin_clean(IN_KEY_INT)) && (!flags.poweroff_bit))  // проверка длительности удержания кнопки
        {
                delay_ms(100);
                _wdr();
                if (_is_pin_clean(IN_KEY_PLUS)) {
                        key2_pressed = 1;
                }
                key_press++;

                if ((key_press % 10) == 0) {
                        for (i = 800; i > 0; i--) {
                                led_refresh(0);
                                //beep_pin = 1;
                                _pin_on(OUT_BEEPER);
                                delay_us(50);
                                //beep_pin = 0;
                                _pin_off(OUT_BEEPER);
                                if (!LcdIsPwrDown()) led_refresh(2);
                                delay_us(300);
                        }
                }

                if (key_press > 50) {  // если держим более 5-ти секунд, то выключить устройство
                        flags.poweroff_bit = 1;
                        GICR = 0x00;
                }
        }
        if ((key_press >= 10) && (key_press <= 50) && key2_pressed) {
                if (IsMenuActive() == 0) {
                        SetMenuActive(1);
                }
        } else {
                if ((key_press >= 10) && (key_press <= 50) && (!key2_pressed)) {
                        if (!LcdIsPwrDown()) {
                                if (IsBlackLight()) {
                                        SetBlackLight(0);
                                } else {
                                        SetBlackLight(1);
                                }
                        }
                }
        }
        key_press = 0;

}

static void InitHard()
{
        WDTCR = 0x1F;
        WDTCR = 0x0F;
        _cli();

        TCCR1A = 0x00;
        TCCR1B = 0x00;
        TCNT1H = 0x00;
        TCNT1L = 0x00;
        ICR1H = 0x00;
        ICR1L = 0x00;
        OCR1AH = 0x00;
        OCR1AL = 0x00;
        OCR1BH = 0x00;
        OCR1BL = 0x00;

        // Даем немного времени для стабилизации работы генератора
        delay_ms(1000);

        GICR = 0xC0;
        MCUCR = 0xB0;
        GIFR = 0xC0;

        //PORTD.3 = 0;   Прерывание без подтяжки
        //DDRD.3 = 0;
        _pin_off(IN_SENSOR);
        _dir_in(IN_SENSOR);

        //PORTD.2 = 1;   Подтяжка на прерывание
        //DDRD.2 = 0;
        _pin_on(IN_KEY_INT);
        _dir_in(IN_KEY_INT);

        //PORTC.2 = 1;   Подтяжка на кнопку "+"
        //DDRC.2 = 0;
        _pin_on(IN_KEY_PLUS);
        _dir_in(IN_KEY_PLUS);

        //PORTC.1 = 0;   Динамик
        //DDRC.1 = 1;
        _pin_off(OUT_BEEPER);
        _dir_out(OUT_BEEPER);

        //PORTD.0 = 0;   Красная подсветка
        //DDRD.0 = 1;
        _pin_off(OUT_LED_RED);
        _dir_out(OUT_LED_RED);

        //PORTD.5 = 0;   Синяя подсветка
        //DDRD.5 = 1;
        _pin_off(OUT_LED_BLUE);
        _dir_out(OUT_LED_BLUE);

        //PORTD.1 = 0;   Ножка на управление трансом.
        //DDRD.1 = 1;
        _pin_off(OUT_PUMP_SWITCH);
        _dir_out(OUT_PUMP_SWITCH);

        TIMSK = 0x40;
}
