/**
 * \file
 * \brief Реализация графического интерфейса.
 * \details
 *
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#include <stdio.h>
#include "app.h"
#include "compiler.h"
#include "config.h"
#include "clock.h"
#include "display/n3310lcd.h"
#include "screens.h"
#include "sensor.h"
#include "power.h"
#include "user.h"

/*************************************************************
 *      Private function prototype
 *************************************************************/

/**
 * Обработка кнопок в режиме показа меню.
 * \param key Код состояния кнопки, см. USER_KEY_xx в файле user.h
 */
static void HandleKeyMenu(unsigned char key);

/**
 * Отображение основного окна.
 */
static void DrawWindow();

/**
 * Отображение меню.
 */
static void DrawMenu();

/**
 * Отображение сообщения при включении.
 */
static void DrawIntro();

/**
 * Отображение сообщения при выключении.
 */
static void DrawBay();

/**
 * Отображение тревожного сообщения высокого уровня радиации.
 */
static void DrawAlertDose();

/**
 * Отображение тревожного сообщения при разряде батарейки.
 */
static void DrawAlertPower();

/*************************************************************
 *      Variable in RAM
 *************************************************************/

/* Текстовый буфер для вывода на LCD */
static char buf[15];
/* Выбранный пункт меню */
static unsigned char menuSelected;
/* Стадия показа главного экрана. */
static unsigned char windowStep;
static unsigned char show;
static char invalidate;

/*************************************************************
 *      Public function
 *************************************************************/

/*
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. clock.h
 */
extern void ScreenClockEvent(unsigned char event)
{
        if ((event & CLOCK_EVENT_SECOND) == 0) {
                return;
        }

        if (++windowStep > 9) {
                windowStep = 0;
        }
}

/*
 * Изменяет (если это возможно) отображаемый экран. Для отмены этого используйте
 * ScreenHide(). Эти функции только меняют внутреннее состояние модуля, что бы
 * изменения появились вызовите ScreenDraw().
 * \param view Требуемый экран, может быть одним из #SCREEN_VIEW_ALERT_DOSE,
 * #SCREEN_VIEW_ALERT_POWER, #SCREEN_VIEW_BAY, #SCREEN_VIEW_INTRO,
 * #SCREEN_VIEW_MAIN, #SCREEN_VIEW_MENU.
 */
extern void ScreenShow(unsigned char view)
{
        if (view == SCREEN_VIEW_ALERT_POWER && show == SCREEN_VIEW_ALERT_DOSE) {
                return;
        }

        show = view;
        return;
}

/*
 * Отменяет (если это возможно) результат ScreenShow(). Эти функции только
 * меняют внутреннее состояние модуля, что бы изменения появились вызовите
 * ScreenDraw().
 * \param view Отменяемый экран, может быть одним из #SCREEN_VIEW_ALERT_DOSE,
 * #SCREEN_VIEW_ALERT_POWER, #SCREEN_VIEW_BAY, #SCREEN_VIEW_INTRO,
 * #SCREEN_VIEW_MAIN, #SCREEN_VIEW_MENU.
 */
extern void ScreenHide(unsigned char view)
{
        if (show == view) {
                show = SCREEN_VIEW_MAIN;
        }
        return;
}

/*
 * Перерисовка экрана.
 */
extern void ScreenDraw()
{
        static unsigned char old = 0;

        if (AppGetMode() == APP_MODE_OFF) {
                return;
        }

        if (old != show) {
                invalidate = 1;
        }

        switch (show) {
        case SCREEN_VIEW_INTRO:
                DrawIntro();
                break;
        case SCREEN_VIEW_BAY:
                DrawBay();
                break;
        case SCREEN_VIEW_MENU:
                DrawMenu();
                break;
        case SCREEN_VIEW_ALERT_DOSE:
                DrawAlertDose();
                break;
        case SCREEN_VIEW_ALERT_POWER:
                DrawAlertPower();
                break;
        case SCREEN_VIEW_MAIN:
        default:
                DrawWindow();
        }
}

/*
 * Обработка кнопок текущим экраном.
 * \param key Код состояния кнопки, см. USER_KEY_xx в файле user.h
 * \see ScreenShow()
 */
extern void ScreenHandleKey(unsigned char key)
{

        switch (show) {
        case SCREEN_VIEW_MENU:
                HandleKeyMenu(key);
                break;
        case SCREEN_VIEW_MAIN:
                if (key == USER_KEY_MENU_UP) {
                        menuSelected = 1;
                        ScreenShow(SCREEN_VIEW_MENU);
                }
                break;
        default:
                if (key == USER_KEY_OK_UP) {
                        ScreenHide(show);
                }
        }
        return;
}

/*************************************************************
 *      Private function
 *************************************************************/

static void HandleKeyMenu(unsigned char key)
{
        if (key == USER_KEY_OK_UP) {
                menuSelected++;
                if (menuSelected > 9) {
                        menuSelected = 0;
                        ScreenHide(SCREEN_VIEW_MENU);
                        return;
                }
        } else if (key != USER_KEY_PLUS_UP) {
                return;
        }

        switch (menuSelected) {
        case 1:
                SensorIncAlarmLevel();
                break;
        case 2:
                AppIncSaveTime();
                break;
        case 3:
                UserSetSoundEnable(!UserIsSoundEnable());
                break;
        case 4:
                SensorClearDose();
                ClockClearDays();
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
        default:
                return;
        }

        invalidate = 1;
        return;
}

static void DrawMenu()
{
        if (invalidate == 0) {
                return;
        }
        invalidate = 0;

        format(buf, PSTR("     МЕНЮ     "));
        LcdStringInv(buf, 0, 0);

        if (menuSelected < 5) {
                if (SensorGetAlarmLevel() == 0) {
                        format(buf, PSTR("Тревога  откл."));
                } else {
                        format(buf, PSTR("Тревога%4uмкР"),
                                        SensorGetAlarmLevel());
                }

                if (menuSelected == 1) {
                        LcdStringInv(buf, 0, 2);
                } else {
                        LcdString(buf, 0, 2);
                }

                if (AppGetSaveTime() == 0) {
                        format(buf, PSTR("Сон      откл."));
                } else {
                        format(buf, PSTR("Сон    %4uсек"), AppGetSaveTime());
                }
                if (menuSelected == 2) {
                        LcdStringInv(buf, 0, 3);
                } else {
                        LcdString(buf, 0, 3);
                }

                if (UserIsSoundEnable() == 0) {
                        format(buf, PSTR("Звук     откл."));
                } else {
                        format(buf, PSTR("Звук      вкл."));
                }

                if (menuSelected == 3) {
                        LcdStringInv(buf, 0, 4);
                } else {
                        LcdString(buf, 0, 4);
                }

                format(buf, PSTR("Сброс  дозы   "));
                if (menuSelected == 4) {
                        LcdStringInv(buf, 0, 5);
                } else {
                        LcdString(buf, 0, 5);
                }

        } else if ((menuSelected > 4) && (menuSelected < 7)) {
                /* 2 страница меню. */
                format(buf, PSTR("Часы     %2u:  "), ClockGetHours());
                if (menuSelected == 5) {
                        LcdStringInv(buf, 0, 2);
                } else {
                        LcdString(buf, 0, 2);
                }

                format(buf, PSTR("Минуты     :%2u"), ClockGetMimutes());
                if (menuSelected == 6) {
                        LcdStringInv(buf, 0, 3);
                } else {
                        LcdString(buf, 0, 3);
                }
        } else {
                /* 3 страница меню. */
                format(buf, PSTR("Накачка %3uимп"), SensorGetTicksPeriodik());
                if (menuSelected == 7) {
                        LcdStringInv(buf, 0, 2);
                } else {
                        LcdString(buf, 0, 2);
                }

                format(buf, PSTR("При имп.%3uимп"), SensorGetTicksHit());
                if (menuSelected == 8) {
                        LcdStringInv(buf, 0, 3);
                } else {
                        LcdString(buf, 0, 3);
                }

                format(buf, PSTR("Импульс %3uмкс"), SensorGetPulseDuration());
                if (menuSelected == 9) {
                        LcdStringInv(buf, 0, 4);
                } else {
                        LcdString(buf, 0, 4);
                }
        }

        LcdUpdate();
}

static void DrawWindow()
{
        unsigned char i, j, old = 0;

        if (old != windowStep) {
                invalidate = 1;
        }
        if (invalidate == 0) {
                return;
        }
        invalidate = 0;

        format(buf, PSTR("%5u"), SensorGetRadiation(SENSOR_INDEX_MAX + 1));
        LcdStringBold(buf, 0, 1);
        format(buf, PSTR("мкР"));
        LcdString(buf, 11, 1);
        format(buf, PSTR(" ч "));
        LcdString(buf, 11, 2);
        LcdLine(66, 16, 83, 16, 1);

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
                        ClockGetSeconds());
        LcdString(buf, 0, 0);

        if (windowStep > 5) {
                format(buf, PSTR("^ %03u$"), PowerCharge());
        } else {
                format(buf, PSTR("^%01u.%02uv"), PowerVoltage() / 10,
                                PowerVoltage() % 10);
        }
        LcdString(buf, 8, 0);

        if (windowStep <= 3) {
                format(buf, PSTR("За час%5luмкР"), SensorGetDoseHour());
        } else if (windowStep > 3 && windowStep <= 6) {
                format(buf, PSTR("Сут.%7luмкР"), SensorGetDoseDay());
        } else {
                format(buf, PSTR("%3uД%7luмкР"), ClockGetDays(),
                                SensorGetDoseAll());
        }
        LcdString(buf, 0, 5);

        LcdUpdate();
}

static void DrawIntro()
{
        if (invalidate == 0) {
                return;
        }
        invalidate = 0;

        LcdClear();
        format(buf, PSTR("   Микрон-2е  "));
        LcdString(buf, 0, 1);
        format(buf, PSTR("Загрузка"));
        LcdString(buf, 0, 4);
        LcdUpdate();
}

static void DrawBay()
{
        if (invalidate == 0) {
                return;
        }
        invalidate = 0;

        LcdClear();
        format(buf, PSTR("  Выключение  "));
        LcdString(buf, 0, 2);
        LcdUpdate();
}

static void DrawAlertDose()
{
        if (invalidate == 0) {
                return;
        }
        invalidate = 0;

        format(buf, PSTR("   ОПАСНОЕ    "));
        LcdStringInv(buf, 0, 2);
        format(buf, PSTR("  ИЗЛУЧЕНИЕ!  "));
        LcdStringInv(buf, 0, 3);
        LcdUpdate();
}

static void DrawAlertPower()
{
        if (invalidate == 0) {
                return;
        }
        invalidate = 0;

        format(buf, PSTR(" НИЗКИЙ ЗАРЯД!"));
        LcdString(buf, 0, 4);
        LcdUpdate();
}
