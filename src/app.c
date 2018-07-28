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

/**
 * Задержка при включении, секунд.
 */
#define APP_DELAY_ON    7
/**
 * Задержка при выключении, секунд.
 */
#define APP_DELAY_OFF   3

/**
 * Максимальное значение времени задержки перехода в режим энергосбережения.
 * Это значение не может быть больше чем 255*APP_SAVE_DELAY_STEP.
 */
#define APP_SAVE_DELAY_MAX            600U

/**
 * Значение времени задержки перехода в режим энергосбережения по умолчанию.
 */
#define APP_SAVE_DELAY_DEFAULT        30U

/**
 * Шаг отсчёта времени задержки перехода в режим энергосбережения. Эта величина
 * используется для уменьшения размера переменной, отвечающей за отсчёт
 * задержки.
 */
#define APP_SAVE_DELAY_STEP           10U

/*************************************************************
 *      Private function prototype
 *************************************************************/

static void AppOn();
static void AppOff();
static void AppClockEvent(unsigned char event);
static void AppStartSaveTimer(unsigned char key);
static void AppToggle(unsigned char key);

/*************************************************************
 *      Variable in RAM
 *************************************************************/

/* Текущий режим работы устройства. См. AppSetMode(). */
static unsigned char appMode = APP_MODE_OFF;
/* Время задержки перехода в режим энергосбережения. */
static unsigned char saveDelay = 0;
/* Счётчик перехода в режим энергосбережения. */
static unsigned int saveTimer = 0;
/* Счётчик задержки переключения включен-выключен. */
static unsigned char toggleTimer = 0;
static unsigned char toggleMode = APP_MODE_OFF;

/*************************************************************
 *      Variable in EEPROM
 *************************************************************/

/* Время задержки перехода в режим энергосбережения, значение, сохраняемое
 * в EEPROM.
 */
static eeprom unsigned char eeSaveDelay = APP_SAVE_DELAY_DEFAULT
                / APP_SAVE_DELAY_STEP;

/*************************************************************
 *      Entry point
 *************************************************************/

void main(void)
{
        unsigned char event, key;
        struct {
                char sensorError :1;
                char sensorAlarm :1;
                char powerBad :1;
        } flags = {
                0, 0, 0
        };

        /*
         * Подсовываем нажатие кнопки "питание" что бы запустить устройство
         * при подаче напряжения. Именно поэтому опрос кнопок происходит в
         * конце бесконечного цикла после выхода из спячки.
         * Возможно стоит написать проверку, не было ли срабатывания сторожевого
         * таймера.
         */
        key = USER_KEY_POWER_DOWN;

        while (1) {

                /* Обработка событий от кнопок. */
                AppToggle(key);
                AppStartSaveTimer(key);
                ScreenHandleKey(key);

                if (SensorIsHit()){
                        /*TODO Сделать реализацию щелчка при сработке датчика.*/
                }

                /* Проверка превышения максимальной радиации. */
                if (SensorIsAlarm()) {
                        if (flags.sensorAlarm == 0) {
                                flags.sensorAlarm = 1;
                                UserAlert(USER_ALERT_DOSE);
                                ScreenShow(SCREEN_VIEW_ALERT_DOSE);
                        }
                } else {
                        flags.sensorAlarm = 0;
                        ScreenHide(SCREEN_VIEW_ALERT_DOSE);
                }

                /* Проверка работоспособности датчика. */
                if (SensorError()) {
                        if (flags.sensorError == 0) {
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

                /* Обновление экрана и спячка. */
                ScreenDraw();
                _sleep();

                /* Получение и обработка событий программного таймера. */
                event = ClockGetEvent();
                AppClockEvent(event);
                SensorClockEvent(event);
                PowerClockEvent(event);
                UserClockEvent(event);
                ScreenClockEvent(event);

                /* Получение событий от кнопок. */
                key = UserGetKey();
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
                AppOn();
                PowerOn();
                ClockOn();
                SensorOn();
                ScreenOn();
                UserOn();
                LcdOn();
                break;
        case APP_MODE_SAVE:
                PowerSleep();
                ClockSleep();
                SensorSleep();
                ScreenSleep();
                UserSleep();
                LcdSleep();
                break;
        case APP_MODE_OFF:
        default:
                AppOff();
                PowerOff();
                ClockOff();
                SensorOff();
                ScreenOff();
                UserOff();
                LcdOff();
        }
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
static void AppOn()
{
        WDTCR = 0x1F;
        WDTCR = 0x0F;
        _sei();
        saveDelay = _eemem_read8(&eeSaveDelay);
}

static void AppOff()
{
        GICR = 0x40;
        MCUCR = 0xA0;
        TCCR0 = 0x00;
}

/**
 * \brief Обновление внутреннего состояния модуля.
 * \details Обновления происходят каждую секунду. По окончанию счёта таймера
 *  saveTimer() происходит переключение в экономный режим. По окончанию счёта
 *  таймера toggleTimer() происходит смена режима включено/выключено.
 * \param event Набор флагов CLOCK_EVENT_xx см. clock.h
 */
static void AppClockEvent(unsigned char event)
{
        if ((event & CLOCK_EVENT_SECOND) == 0) {
                return;
        }

        if (saveTimer != 0 && appMode != APP_MODE_OFF) {
                saveTimer--;
                if (saveTimer == 0) {
                        AppSetMode(APP_MODE_SAVE);
                }
        }

        if (toggleTimer != 0) {
                toggleTimer--;
                if (toggleTimer == 0) {
                        AppSetMode(toggleMode);
                        ScreenHide(SCREEN_VIEW_INTRO);
                        ScreenHide(SCREEN_VIEW_BAY);
                }
        }
}

/**
 * \brief Подготовка к переключению режима работы устройства.
 * \details Если нажата кнопка #USER_KEY_POWER_DOWN и процесс подготовки ещё
 *  не запущен, то подготовка запускается. Механизм переключения использует
 *  таймер toggleTimer() для задержек.
 *
 *  Если устройство отключено, сразу же устанавливается режим #APP_MODE_ON,
 *  запускается таймер и показывается приветствие.
 *
 *  Если устройство включено, то запускается таймер и показывается экран
 *  выключения. Устройство выключается в AppClockEvent() по окончании работы
 *  таймера.
 *
 * \param key Код нажатой кнопки.
 */
static void AppToggle(unsigned char key)
{
        if (toggleTimer != 0 || key != USER_KEY_POWER_DOWN) {
                return;
        }

        if (appMode == APP_MODE_OFF) {
                AppSetMode(APP_MODE_ON);
                toggleMode = APP_MODE_ON;
                toggleTimer = APP_DELAY_ON;
                ScreenShow(SCREEN_VIEW_INTRO);
        } else {
                toggleMode = APP_MODE_OFF;
                toggleTimer = APP_DELAY_OFF;
                ScreenShow(SCREEN_VIEW_BAY);
        }

}

/*
 * Перезапуск счетчика задержки перехода в режим пониженного потребления
 * питания.
 */
static void AppStartSaveTimer(unsigned char key)
{
        if (key == 0) {
                return;
        }

        saveTimer = saveDelay * APP_SAVE_DELAY_STEP;
}
