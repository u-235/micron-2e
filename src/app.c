/**
 * \file
 * \brief
 * \details
 *
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#include "app.h"
#include "compiler.h"
#include "config.h"
#include "clock.h"
#include "display/n3310lcd.h"
#include "power.h"
#include "screens.h"
#include "sensor.h"
#include "user.h"

/*************************************************************
 *      Private function prototype
 *************************************************************/

static void AppInit();
static void AppClockEvent(unsigned char event);
static void AppStartSaveTimer();

/*************************************************************
 *      Variable in RAM
 *************************************************************/

/* Текущий режим работы устройства. См. AppSetMode(). */
static unsigned char appMode;
/* Время задержки перехода в режим энергосбережения. */
static unsigned char saveDelay;
/* Счётчик перехода в режим энергосбережения. */
static unsigned int saveTimer;

/*************************************************************
 *      Variable in EEPROM
 *************************************************************/

/* Время задержки перехода в режим энергосбережения, значение, сохраняемое
 * в EEPROM.
 */
eeprom static unsigned char eeSaveDelay = APP_SAVE_DELAY_DEFAULT
                / APP_SAVE_DELAY_STEP;

/*************************************************************
 *      Entry point
 *************************************************************/

void main(void)
{
        WDTCR = 0x1F;
        WDTCR = 0x0F;
        _sei();
        PowerInit();
        ClockInit();
        AppInit();
        AppSetMode(APP_MODE_ON);
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
                AppClockEvent(event);
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
                        AppStartSaveTimer();
                        ScreenHandleKey(key);
                }

                if (key == USER_KEY_POWER_DOWN) {
                        ScreenShow(SCREEN_VIEW_BAY);
                }

                ScreenDraw();

                if (key == USER_KEY_POWER_DOWN) {
                        delay_ms(2000);
                        AppSetMode(APP_MODE_OFF);
                }
                _sleep();
        }
}

/*************************************************************
 *      Public function
 *************************************************************/

/**
 * Переключение режима работы устройства.
 * \param mode Один из режимов #APP_MODE_ON, #APP_MODE_SAVE или
 * #APP_MODE_OFF.
 */
extern void AppSetMode(unsigned char mode)
{
        appMode = mode;

        switch (mode) {
        case APP_MODE_ON:
                SensorInit();
                UserInit();
                LcdInit();
                break;
        case APP_MODE_SAVE:
                //LcdPwrOff();
                break;
        case APP_MODE_OFF:
        default:
                _interrupt_disable(INT_EXT1);
                LcdPwrOff();
                //UserAsyncBeep(0);
                //SetMenuActive(0);
                //UserLight(2);
                GICR = 0x40;
                MCUCR = 0xA0;
                DDRB = 0x00;
                DDRC = 0x00;
                DDRD = 0x00;
                PORTB = 0x00;
                PORTC = 0x00;
                PORTD = 0x04;
                TCCR0 = 0x00;
        }
}

/**
 * Получение режима работы устройства.
 * \return Один из режимов #APP_MODE_ON, #APP_MODE_SAVE или #APP_MODE_OFF.
 */
extern unsigned char AppGetMode()
{
        return appMode;
}

/*
 * Получение времени задержки перехода в режим пониженного потребления питания.
 * \return время задержки в секундах.
 */
extern unsigned int AppGetSaveTime()
{
        return (unsigned int) saveDelay * APP_SAVE_DELAY_STEP;
}

/*
 * Установка времени задержки перехода в режим пониженного потребления питания.
 * \param tm время задержки в секундах. Если \arg tm больше APP_SAVE_DELAY_MAX
 *      то записывается ноль.
 */
extern void AppSetSaveTime(unsigned int tm)
{
        unsigned char t = tm / APP_SAVE_DELAY_STEP;
        if (t > APP_SAVE_DELAY_MAX / APP_SAVE_DELAY_STEP) {
                t = 0;
        }
        saveDelay = t;
        _eemem_write8(&eeSaveDelay, saveDelay);
}

/*
 * Увеличение времени задержки перехода в режим пониженного потребления питания.
 * Для значения задержки до 60 секунд увеличение происходит на 10, далее на 60
 * секунд.Если задержка больше #APP_SAVE_DELAY_MAX, то записывается ноль.
 */
extern void AppIncSaveTime()
{
        unsigned char st = saveDelay;
        if (st >= 60 / APP_SAVE_DELAY_STEP) {
                st += 60 / APP_SAVE_DELAY_STEP;
        } else {
                st += 10 / APP_SAVE_DELAY_STEP;
        }
        AppSetSaveTime(st);
}

/*************************************************************
 *      Private function
 *************************************************************/

/*
 * Инициализация модуля.
 */
static void AppInit()
{
        saveDelay = _eemem_read8(&eeSaveDelay);
}

/*
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
static void AppClockEvent(unsigned char event)
{
        if (AppGetMode() == APP_MODE_OFF || (event & CLOCK_EVENT_SECOND) == 0) {
                return;
        }
        if (saveTimer != 0) {
                saveTimer--;
                if (saveTimer == 0) {
                        AppSetMode(APP_MODE_SAVE);
                }
        }
}

/*
 * Перезапуск счетчика задержки перехода в режим пониженного потребления
 * питания.
 */
static void AppStartSaveTimer()
{
        saveTimer = saveDelay * APP_SAVE_DELAY_STEP;
}
