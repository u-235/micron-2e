/**
 * \file
 * \brief
 * \details
 *
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#include "compiler.h"
#include "config.h"
#include "clock.h"
#include "power.h"
#include "screens.h"
#include "sensor.h"
#include "user.h"

void main(void)
{
        unsigned char delay;
        WDTCR = 0x1F;
        WDTCR = 0x0F;
        _sei();
        PowerInit();
        ClockInit();
        PowerSetMode(POWER_MODE_ON);
        ScreenShow(SCREEN_VIEW_INTRO);
        ScreenDraw();
        delay_ms(2000);

        while (1) {
                unsigned char event, key;
                struct {
                        char sensorBad :1;
                        char powerBad :1;
                        char sensorError :1;
                } flags = {
                        0, 0, 0
                };

                /* Получение и обработка событий программного таймера. */
                event = ClockGetEvent();
                SensorClockEvent(event);
                PowerClockEvent(event);
                UserClockEvent(event);
                ScreenClockEvent(event);

                /* Проверка превышения максимальной радиации. */
                if (SensorIsAlarm()) {
                        if (flags.sensorBad == 0) {
                                flags.sensorBad = 1;
                                UserAlert(USER_ALERT_DOSE);
                                ScreenShow(SCREEN_VIEW_ALERT_DOSE);
                        }
                } else {
                        flags.sensorBad = 0;
                        ScreenHide(SCREEN_VIEW_ALERT_DOSE);
                }

                /* Проверка работоспособности датчика. */
                if (SensorError()) {
                        if (flags.sensorBad == 0) {
                                flags.sensorError = 1;
                                UserAlert(USER_ALERT_SENSOR);
                        }
                } else {
                        flags.sensorError = 0;
                }

                /* Проверка уровня заряда источника питания. */
                if (PowerCheck()) {
                        if (flags.powerBad == 0) {
                                flags.powerBad = 1;
                                UserAlert(USER_ALERT_POWER);
                                ScreenShow(SCREEN_VIEW_ALERT_POWER);
                        }
                } else {
                        flags.powerBad = 0;
                        ScreenHide(SCREEN_VIEW_ALERT_POWER);
                }

                /* Получение и обработка событий от кнопок. */
                key = UserGetKey();
                if (key != 0) {
                        PowerStartSaveTimer();
                        ScreenHandleKey(key);
                }

                if (key == USER_KEY_POWER_DOWN) {
                        ScreenShow(SCREEN_VIEW_BAY);
                }

                ScreenDraw();

                if (key == USER_KEY_POWER_DOWN) {
                        delay_ms(2000);
                        PowerSetMode(POWER_MODE_OFF);
                }
                _sleep();
        }
}
