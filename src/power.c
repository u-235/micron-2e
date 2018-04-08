/**
 * \file
 * \brief Реализация управления питанием.
 * \details
 *
 * \anchor adc_internal
 * \b Измерение \b напряжения \b питания \b МК
 * \n Наиболее интересной и неочевидной частью этого модуля является
 * измерение напряжения аккумулятора при использовании схемы с питанием
 * непосредственно от литиевого аккумулятора. При такой конфигурации за опорное
 * берётся напряжение питания МК \f$V_{cc}\f$, а измеряется напряжение
 * \f$V_{bg}\f$ на первичного источника опорного напряжения. В справочнике это
 * напряжение называется "bandgap reference" и номинально оно равно 1.3 вольта
 * для ATmega8 (но в справочнике от 2003 года указана цифра 1.23 вольта). Если
 * максимальное разрешение АЦП обозначить как \f$ADC_{max}\f$, то в общем случае
 * результат измерений \f$ADC\f$ можно выразить через формулу
 * \f[
 * ADC = \frac{V_{bg} \times ADC_{max}}{V_{cc}}
 * \f]
 *
 * Сложность в том, что напряжение первичного ИОН зависит от питающего
 * напряжения \f$V_{cc}\f$ (хотя и слабо) и от температуры. В данном устройстве
 * нет датчика температуры, поэтому будем считать что
 * \f$V_{bg} = V_{bg0} + k \times V_{cc}\f$ , где \f$V_{bg0}\f$ - это начальное
 * значение опорного напряжения и \f$ k \f$ - это зависимость опорного
 * напряжения от питающего. В таком случае предыдущая формула результата
 * измерений становится
 * \f[
 * ADC
 *     = \frac{(V_{bg0} + k \times V_{cc} ) \times ADC_{max}}{V_{cc}}
 *     = \frac{V_{bg0} \times ADC_{max}}{V_{cc}} + k \times ADC_{max}
 * \f]
 *
 * И наконец, напряжение питания можно вычислить по следующей формуле.
 * \f[
 * V_{cc}=\frac{V_{bg0} \times ADC_{max}}{ADC - k \times ADC_{max}}
 * \f]
 *
 * Поскольку в устройстве напряжение выражается в сотых долях вольта, то
 * получаем
 * \f[
 * V_{cc}=\frac{V_{bg0} \times ADC_{max} \times 100}{ADC - k \times ADC_{max}}
 * \f]
 *
 * \date Создан 24.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#include "compiler.h"
#include "config.h"
#include "alarm.h"
#include "clock.h"
#include "display/n3310lcd.h"
#include "power.h"
#include "screens.h"
#include "sensor.h"

#define VOLTAGE_MEASURE_CYCLE   16U

/**
 * Максимальное значение АЦП. Используются старшие 8 бит результата, поэтому
 * максимальное значение равно 256. Казалось бы должно быть 255, но в
 * справочнике на МК настаивают именно на 256.
 */
#define ADC_MAX                 256U

/**
 * Вольтаж первичного источника опорного напряжения.
 */
#ifndef ADC_BANDGAP_REFERENCE
#define ADC_BANDGAP_REFERENCE   1.2845F
#endif

/**
 * Коэффициент зависимости первичного ИОН от напряжения питания.
 */
#ifndef ADC_BANDGAP_VS_VCC
#define ADC_BANDGAP_VS_VCC      0.0025F
#endif

/**
 * Вольтаж внутреннего источника опорного напряжения АЦП.
 */
#ifndef ADC_INTERNAL_REFERENCE
#define ADC_INTERNAL_REFERENCE  2.56F
#endif

/**
 * В схеме с батарейкой её напряжение измеряется через делитель 2:1
 * Если показания не соответствуют реальным, можно изменить эту константу или
 * #ADC_INTERNAL_REFERENCE.
 */
#ifndef MEASURE_CIRCUIT_SCALE
#define MEASURE_CIRCUIT_SCALE   2.0F
#endif

/*************************************************************
 *      Private function prototype
 *************************************************************/

/*
 * Запуск измерения напряжения питания.
 */
static void RunMeasure();

/*************************************************************
 *      Variable in RAM
 *************************************************************/

/* Текущий режим работы устройства. См. PowerSetMode(). */
static unsigned char powerMode;
/* Оставшийся заряд источника питания. */
static unsigned char btrPercent = 0;
/* Напряжение источника питания, в сотых долях вольта. */
static unsigned int btrVoltage = 0;
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
eeprom static unsigned char eeSaveDelay = POWER_SAVE_DELAY_DEFAULT
                / POWER_SAVE_DELAY_STEP;

/*************************************************************
 *      Public function
 *************************************************************/

/*
 * Инициализация модуля. Для включения устройства нужно вызвать
 * PowerSetMode().
 */
extern void PowerInit()
{
        saveDelay = _eemem_read8(&eeSaveDelay);
        RunMeasure();
}

/*
 * Переключение режима работы устройства.
 * \param mode Один из режимов #POWER_MODE_ON, #POWER_MODE_SAVE или
 * #POWER_MODE_OFF.
 */
extern void PowerSetMode(unsigned char mode)
{
        powerMode = mode;

        switch (mode) {
        case POWER_MODE_ON:
                SensorInit();
                AlarmInit();
                LcdInit();
                break;
        case POWER_MODE_SAVE:
                LcdPwrOff();
                break;
        case POWER_MODE_OFF:
        default:
                _interrupt_disable(INT_EXT1);
                LcdPwrOff();
                AsyncBeep(0);
                SetMenuActive(0);
                led_refresh(2);
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

/*
 * Получение режима работы устройства.
 * \return Один из режимов #POWER_MODE_ON, #POWER_MODE_SAVE или #POWER_MODE_OFF.
 */
extern unsigned char PowerGetMode()
{
        return powerMode;
}

/*
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
extern void PowerClockEvent(unsigned char event)
{
        if (powerMode == POWER_MODE_OFF || (event & CLOCK_EVENT_SECOND) == 0) {
                return;
        }
        RunMeasure();
        if (saveTimer != 0) {
                saveTimer--;
                if (saveTimer == 0) {
                        PowerSetMode(POWER_MODE_SAVE);
                }
        }
}

/*
 * Перезапуск счетчика задержки перехода в режим пониженного потребления
 * питания.
 */
void PowerStartSaveTimer()
{
        saveTimer = saveDelay * POWER_SAVE_DELAY_STEP;
}

/*
 * Получение времени задержки перехода в режим пониженного потребления питания.
 * \return время задержки в секундах.
 */
extern unsigned int PowerGetSaveTime()
{
        return (unsigned int) saveDelay * POWER_SAVE_DELAY_STEP;
}

/*
 * Установка времени задержки перехода в режим пониженного потребления питания.
 * \param tm время задержки в секундах. Если \arg tm больше POWER_SAVE_DELAY_MAX
 *      то записывается ноль.
 */
extern void PowerSetSaveTime(unsigned int tm)
{
        unsigned char t = tm / POWER_SAVE_DELAY_STEP;
        if (t > POWER_SAVE_DELAY_MAX / POWER_SAVE_DELAY_STEP) {
                t = 0;
        }
        saveDelay = t;
        _eemem_write8(&eeSaveDelay, saveDelay);
}

/*
 * Увеличение времени задержки перехода в режим пониженного потребления питания.
 * Для значения задержки до 60 секунд увеличение происходит на 10, далее на 60
 * секунд.Если задержка больше #POWER_SAVE_DELAY_MAX, то записывается ноль.
 */
extern void PowerIncSaveTime()
{
        unsigned char st = saveDelay;
        if (st >= 60 / POWER_SAVE_DELAY_STEP) {
                st += 60 / POWER_SAVE_DELAY_STEP;
        } else {
                st += 10 / POWER_SAVE_DELAY_STEP;
        }
        PowerSetSaveTime(st);
}

/*
 * Проверка состояния источника питания.
 * \return 0 если уровень питания в норме.
 */
extern char PowerCheck()
{
        if (PowerCharge() < POWER_LEVEL_ALARM) {
                return ~0;
        }
        return 0;
}

/**
 * Возвращает напряжение источника питания.
 * \return Напряжение питания в сотых долях вольта.
 */
extern unsigned int PowerVoltage()
{
        return btrVoltage;
}

/**
 * Возвращает заряд источника питания.
 * \return Оставшийся заряд в процентах.
 */
extern unsigned char PowerCharge()
{
        return btrPercent;
}

/*************************************************************
 *      Interrupt handler
 *************************************************************/
static unsigned int measureVoltage;
static unsigned char measureCycle;

_isr_adc(void)
{
        measureVoltage += ADCH;
        if (--measureCycle != 0) {
                return;
        }

        ADCSRA = 0x00;
        ACSR = 0x00;
        ADMUX = 0x00;
#ifdef POWER_LION
        btrVoltage = measureVoltage
                        - (unsigned int) (ADC_BANDGAP_VS_VCC * ADC_MAX
                                        * VOLTAGE_MEASURE_CYCLE);
        btrVoltage /= VOLTAGE_MEASURE_CYCLE;
        btrVoltage = ((unsigned int) (100 * ADC_BANDGAP_REFERENCE) * ADC_MAX)
                        / btrVoltage;
#else
        btrVoltage = measureVoltage
        * ((unsigned int) (100 * ADC_INTERNAL_REFERENCE
                                        * MEASURE_CIRCUIT_SCALE)
                        / VOLTAGE_MEASURE_CYCLE )/ ADC_MAX;
#endif
        if (btrVoltage > POWER_VOLTAGE_HIGH) {
                btrVoltage = POWER_VOLTAGE_HIGH;
        }
        if (btrVoltage < POWER_VOLTAGE_LOW) {
                btrVoltage = POWER_VOLTAGE_LOW;
        }
        btrPercent = (btrVoltage - POWER_VOLTAGE_LOW) * 100
                        / (POWER_VOLTAGE_HIGH - POWER_VOLTAGE_LOW);
}

/*************************************************************
 *      Private function
 *************************************************************/

static void RunMeasure()
{
        measureVoltage = 0;
        measureCycle = VOLTAGE_MEASURE_CYCLE;

        /*
         * В версии с радикота измерение происходило в режиме сна для
         * уменьшения помех. Однако на мой взгляд в этом нет необходимости,
         * так как младшие два разряда отбрасываются и используется серия
         * измерений.
         *
         * Общие настройки: разрешаем прерывания, измерение в цикле,
         * предделитель на 64 (частота АЦП 125kHz при шине 8mHz). Используются
         * только старшие 8 бит результата.
         */
        ADCSRA = (1 << ADEN) | (1 << ADFR) | (1 << ADIE) | (1 << ADPS2)
                        | (1 << ADPS1);
#ifdef POWER_LION
        /*
         * В качестве опорного используется напряжение питания.
         * Измерять на ADC14, т.е. вн. ИОН
         */
        ADMUX = (1 << REFS0) | (1 << ADLAR) | (1 << MUX3) | (1 << MUX2)
                        | (1 << MUX1);
#else
        /*
         * В качестве опорного напряжения используется внутренний источник 2.56
         * вольт. Измерять на ADC7 (это невозможно в DIP корпусе)
         */
        ADMUX = (1 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (1 << MUX2)
        | (1 << MUX1) | (1 << MUX0);
#endif
        /* Запуск АЦП */
        ADCSRA |= (1 << ADSC);
}
