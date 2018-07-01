/**
 * \file
 * \brief
 * \details
 *
 * \date Создан 25.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#include "compiler.h"
#include "config.h"
#include "clock.h"
#include "display/n3310lcd.h"
#include "screens.h"
#include "sensor.h"
#include "user.h"

#define BOUNCE_DOWN     (CLOCK_DIVIDER / 4)
#define BOUNCE_UP       (CLOCK_DIVIDER / 2)
#define DELAY_MENU_KEY  (1 * CLOCK_DIVIDER)
#define DELAY_POWER_KEY (5 * CLOCK_DIVIDER)

/*************************************************************
 *      Private function prototype
 *************************************************************/

static void scanKeys();

/*************************************************************
 *      Variable in RAM
 *************************************************************/

static struct _aFlags {
        unsigned char red :1, blue :1, sound_enable :1, beep :1, beep_active :1,
                        alarm :1, black_light_enable :1, init :1, sleep :1;
} flags;

static unsigned char key;

/*************************************************************
 *      Variable in EEPROM
 *************************************************************/

static eeprom char eeSoundEnable = 1;

/*************************************************************
 *      Public function
 *************************************************************/

extern void UserOn()
{
        _pin_on(IN_KEY_INT);
        _dir_in(IN_KEY_INT);
        _pin_on(IN_KEY_PLUS);
        _dir_in(IN_KEY_PLUS);
        _pin_off(OUT_BEEPER);
        _dir_out(OUT_BEEPER);
        _pin_off(OUT_LED_RED);
        _dir_out(OUT_LED_RED);
        _pin_off(OUT_LED_BLUE);
        _dir_out(OUT_LED_BLUE);
        TCCR0 = 0x05;
        TCNT0 = 0xFF;
        flags.sound_enable = _eemem_read8(&eeSoundEnable);
        flags.beep = 0;
        flags.init = 1;
        flags.sleep = 0;
}

extern void UserSleep()
{
        flags.sleep = 1;
}

/*
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
extern void UserClockEvent(unsigned char event)
{
        if (event == 0) {
                return;
        }

        scanKeys();
}

extern void UserAlert(unsigned char alert)
{

}

extern void UserAlertCansel(unsigned char alert)
{

}

extern unsigned char UserGetKey()
{
        return key;
}

extern char UserIsSoundEnable()
{
        return flags.sound_enable;
}

extern void UserSetSoundEnable(char on)
{
        flags.sound_enable = on;
        _eemem_write8(&eeSoundEnable, on);
}

/**
 *
 * \param command
 *      - 0 - выкл светодиодов
 *      - 1 - восстановление состояния перед выключением.
 *      - 2 - подсветка вкл
 */
extern void UserLight(unsigned char command)
{
        if (flags.sleep == 1) {
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

/*************************************************************
 *      Interrupt handler
 *************************************************************/

_isr_timer0_ovf(void)
{
        TCNT0 = 0xFF;

        if (!flags.beep) {
                _pin_off(OUT_BEEPER);
                _interrupt_disable(INT_TIMER0_OVF);
                return;
        }

        if (!flags.beep_active) {
                _pin_off(OUT_BEEPER);
                flags.beep_active = 1;
        } else {
                _pin_on(OUT_BEEPER);
                flags.beep_active = 0;
        }
}

/*************************************************************
 *      Private function
 *************************************************************/

static void scanKeys()
{
        static char old = 0;
        /* Счётчик для  защиты от дребезга контактов. */
        static unsigned char bounce = 0;
        /* Задержка перед сменой назначения клавиши. */
        static unsigned char delay = 0;

        if (delay != 0) {
                delay--;
        }

        if (bounce != 0) {
                bounce--;
                return;
        }

        switch (old) {
        case USER_KEY_OK_DOWN:
                if (_is_pin_set(IN_KEY_INT)) {
                        key = USER_KEY_OK_UP;
                        old = 0;
                        bounce = BOUNCE_UP;
                } else if (delay == 0) {
                        key = USER_KEY_MENU_DOWN;
                        old = key;
                        delay = DELAY_POWER_KEY;
                }
                break;
        case USER_KEY_MENU_DOWN:
                if (_is_pin_set(IN_KEY_INT)) {
                        key = USER_KEY_MENU_UP;
                        old = 0;
                        bounce = BOUNCE_UP;
                } else if (delay == 0) {
                        key = USER_KEY_POWER_DOWN;
                        old = key;
                }
                break;
        case USER_KEY_POWER_DOWN:
                if (_is_pin_set(IN_KEY_INT)) {
                        key = USER_KEY_POWER_UP;
                        old = 0;
                        bounce = BOUNCE_UP;
                }
                break;
        case USER_KEY_PLUS_DOWN:
                if (_is_pin_set(IN_KEY_PLUS)) {
                        key = USER_KEY_PLUS_UP;
                        old = 0;
                        bounce = BOUNCE_UP;
                }
                break;
        default:
                if (_is_pin_clean(IN_KEY_INT)) {
                        key = USER_KEY_OK_DOWN;
                        old = key;
                        bounce = BOUNCE_DOWN;
                        delay = DELAY_MENU_KEY;
                } else if (_is_pin_clean(IN_KEY_PLUS)) {
                        key = USER_KEY_PLUS_DOWN;
                        bounce = BOUNCE_DOWN;
                        old = key;
                }
        }
}
