/**
 * \file
 * \brief Реализация интерфейса датчика
 * \date Создан 24.02.2018
 * \author: Nick Egorrov
 * \copyright GNU Public License 3
 * \details
 */

#include "compiler.h"
#include "config.h"
#include "sensor.h"
#include "power.h"

#define SCALE_MIN 3U
#define PUMP_TICKS_INIT 6000UL

/*************************************************************
 *      Private function prototype
 *************************************************************/

/**
 *
 * \param voltage
 */
static void CalculatePump(unsigned int voltage);

/**
 * Run impulse sequence for charge a capacitor of sensor power supply.
 * \param ticks
 */
static void RunCharge(unsigned char ticks);

/*************************************************************
 *      Variable in RAM
 *************************************************************/

/* Флаг о срабатывании датчика, см. SensorIsHit() и _isr_ext1() */
static unsigned char impHits;
/* Массив зарегистрированных импульсов. Нулевой элемент используется как
 * счётчик импульсов за текущую секунду. */
static unsigned int impMassive[SENSOR_INDEX_MAX + 2];

/* Накопленная доза за час, в импульсах датчика. */
static unsigned long doseHour = 0;
/* Накопленная доза за день, в импульсах датчика. */
static unsigned long doseDay = 0;
/* Общая накопленная доза, в мкР. */
static unsigned long doseAll = 0;
/* Масштабный коэффициент для вычисления относительных уровней радиации в
 * SensorGetLevel(). Равен самому большому значению в массиве. */
static unsigned int scale = SCALE_MIN;

/* Длительность положительнного импульса накачки, рабочее значение. */
static unsigned char pulseDuration;
/* Количество импульсов принудительной накачки, рабочее значение. */
static unsigned char ticksPeriodic;
/* Количество импульсов накачки при сработке датчика, рабочее значение. */
static unsigned char ticksHit;

/*************************************************************
 *      Variable in EEPROM
 *************************************************************/

/* Длительность положительнного импульса накачки, настройка. */
static eeprom unsigned char eePulseDuration = PUMP_PULSE_WIDTH_DEFAULT;
/* Количество импульсов принудительной накачки, настройка. */
static eeprom unsigned char eeTicksPeriodic = PUMP_TICKS_PERIODIK_DEFAULT;
/* Количество импульсов накачки при сработке датчика, настройка. */
static eeprom unsigned char eeTicksHit = PUMP_TICKS_HIT_DEFAULT;

/*************************************************************
 *      Public function
 *************************************************************/

/*
 * Инициализация датчика. Параметры настройки извлекаются из EEPROM и
 * запускается начальный цикл накачки.
 */
extern void InitSensor()
{
        unsigned char i;

        _pin_off(IN_SENSOR);
        _pin_off(OUT_PUMP_SWITCH);
        _dir_in(IN_SENSOR);
        _dir_out(OUT_PUMP_SWITCH);
        pulseDuration = _eemem_read8(&eePulseDuration);
        ticksPeriodic = _eemem_read8(&eeTicksPeriodic);
        ticksHit = _eemem_read8(&eeTicksHit);
        CalculatePump(GetVoltage());
        for (i = PUMP_TICKS_INIT / 255; i > 0; i--) {
                RunCharge(255);
        }
        RunCharge(PUMP_TICKS_INIT % 255);
        _interrupt_enable(INT_EXT1);
}

/*
 * Возвращает длительность импульса накачки преобразователя; задаётся в
 * количестве циклов в RunCharge(), точное время неизвестно.
 * \return Длительность импульса накачки преобразователя.
 */
extern unsigned char SensorGetPulseDuration()
{
        return pulseDuration;
}

/*
 * Увеличивает длительность импульса накачки преобразователя. При достижении
 * #PUMP_PULSE_WIDTH_MAX сбрасывается в #PUMP_PULSE_WIDTH_MIN.
 * \see SensorGetPulseDuration()
 */
extern void SensorIncPulseDuration()
{
        unsigned char pulse = pulseDuration + 1;
        if (pulse > PUMP_PULSE_WIDTH_MAX) {
                pulse = PUMP_PULSE_WIDTH_MIN;
        }
        pulseDuration = pulse;
        _eemem_write8(&eePulseDuration, pulse);
}

/*
 * Возвращает количество импульсов накачки при запуске от таймера.
 * \return Количество импульсов накачки.
 */
extern unsigned int SensorGetTicksPeriodik()
{
        return ticksPeriodic;
}

/*
 * Увеличивает количество импульсов накачки преобразователя при запуске от
 * таймера. При достижении #PUMP_TICKS_PERIODIK_MAX сбрасывается в
 * #PUMP_TICKS_PERIODIK_MIN.
 * \see SensorGetTicksPeriodik()
 */
extern void SensorIncTicksPeriodik()
{
        unsigned char ticks = ticksPeriodic + 1;
        if (ticks > PUMP_TICKS_PERIODIK_MAX) {
                ticks = PUMP_TICKS_PERIODIK_MIN;
        }
        ticksPeriodic = ticks;
        _eemem_write8(&eeTicksPeriodic, ticks);
}

/*
 * Возвращает количество импульсов накачки при сработке датчика.
 * \return Количество импульсов накачки.
 */
extern unsigned int SensorGetTicksHit()
{
        return ticksHit;
}

/*
 * Увеличивает количество импульсов накачки преобразователя при сработке
 * датчика. При достижении #PUMP_TICKS_HIT_MAX сбрасывается в
 * #PUMP_TICKS_HIT_MIN.
 * \see SensorGetTicksHit()
 */
extern void SensorIncTicksHit()
{
        unsigned char ticks = ticksHit + 1;
        if (ticks > PUMP_TICKS_HIT_MAX) {
                ticks = PUMP_TICKS_HIT_MIN;
        }
        ticksHit = ticks;
        _eemem_write8(&eeTicksHit, ticks);
}

/*
 * Возвращает уровень радиации за указанный период в мкР/ч
 * \param period Период в секундах, от 1 до #SENSOR_INDEX_MAX + 1
 * \return Уровень радиации в мкР/ч.
 */
extern unsigned int SensorGetRadiation(unsigned char period)
{
        unsigned int retval = 0, i;

        if (period == 0) {
                return 0;
        }
        if (period > SENSOR_INDEX_MAX) {
                period = SENSOR_INDEX_MAX;
        }

        for (i = 0; i <= period; i++) {
                retval += impMassive[i];
        }
        /* Не люблю магию, но тут 3600 - это количество секунд в часе. */
        return retval / (period + 1) * (3600 / SENSOR_SENSITIVITY);
}

/*
 * Возвращает измеренную дозу за последний завершившийся час.
 * \return Доза в мкР.
 */
extern unsigned long SensorGetDoseHour()
{
        return doseHour / SENSOR_SENSITIVITY;
}

/*
 * Возвращает измеренную дозу за последние завершившиеся сутки с момента
 * сброса измерений функцией SensorClearDose().
 * \return Доза в мкР.
 */
extern unsigned long SensorGetDoseDay()
{
        return (doseDay + doseHour) / SENSOR_SENSITIVITY;
}

/*
 * Возвращает измеренную дозу за всё время с момента сброса измерений функцией
 * SensorClearDose().
 * \return Доза в мкР.
 */
extern unsigned long SensorGetDoseAll()
{
        return doseAll + (doseDay + doseHour) / SENSOR_SENSITIVITY;
}

/*
 * Сброс измерений дозы.
 */
extern void SensorClearDose()
{
        doseHour = 0;
        doseDay = 0;
        doseAll = 0;
}

/*
 * Возвращает относительный уровень радиации.
 * \param index Индекс значения уровня. Диапазон от 0 до #SENSOR_INDEX_MAX.
 * \return Относительный уровень радиации в диапазоне от 0 до
 *       #DOSE_HISTOGRAM_MAX
 */
extern unsigned char SensorGetRelative(unsigned char index)
{
        return (unsigned int) impMassive[++index] * DOSE_HISTOGRAM_MAX / scale;
}

/*
 * Проверяет, была ли сработка датчика с момента предыдущего вызова функции.
 * \return Ненулевое значение если была сработка датчика.
 */
extern unsigned char SensorIsHit()
{
        unsigned char retval = impHits;
        impHits = 0;
        return retval;
}

/* Количество импульсов принудительной накачки, вычисленное значение. */
static unsigned char realTicksPeriodic;
/* Количество импульсов накачки при сработке датчика, вычисленное значение. */
static unsigned char realTicksHit;

/*
 * Обновление данных датчика в зависимости от времени.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
extern void SensorClockEvent(unsigned char event)
{
        unsigned char i;
        unsigned int old;

        if (event & CLOCK_EVENT_MINUTE) {
                CalculatePump(GetVoltage());
        }

        if (event & CLOCK_EVENT_SECOND) {
                scale = SCALE_MIN;
                /* смещение массива фона */
                for (i = SENSOR_INDEX_MAX; i > 0; i--) {
                        _cli();
                        old = impMassive[i - 1];
                        _sei();
                        if (old > scale) {
                                scale = old;
                        }
                        impMassive[i] = old;
                }
                _cli();
                impMassive[0] = 0;
                _sei();
                RunCharge(realTicksPeriodic);
        }

        if (event & CLOCK_EVENT_HOUR) {
                _cli();
                doseDay += doseHour;
                doseHour = 0;
                _sei();
        }

        if (event & CLOCK_EVENT_DAY) {
                doseAll += doseDay / SENSOR_SENSITIVITY;
                doseDay = 0;
        }
}

/*************************************************************
 *      Interrupt handler
 *************************************************************/

_isr_ext1(void)
{
        RunCharge(realTicksHit);
        impHits = 1;
        impMassive[0]++;
        doseHour++;
        /* TODO Нужно улучшить - отслеживать состояние на входе INT1 */
        delay_us(1);
}

/*************************************************************
 *      Private function
 *************************************************************/

static void CalculatePump(unsigned int voltage)
{
#ifdef POWER_LION
        realTicksHit = (unsigned int) (POWER_VOLTAGE_HIGH - SWITCH_DROP_VOLTAGE)
                        * ticksHit / (voltage - SWITCH_DROP_VOLTAGE);
        realTicksPeriodic = (unsigned int) (POWER_VOLTAGE_HIGH
                        - SWITCH_DROP_VOLTAGE) * ticksPeriodic
                        / (voltage - SWITCH_DROP_VOLTAGE);
#else // not lithium
        realTicksHit = ticksHit;
        realTicksPeriodic = ticksPeriodic;
#endif
}

static void RunCharge(unsigned char ticks)
{
        unsigned char i, j;

        for (i = 0; i < ticks; i++) {
                _pin_on(OUT_PUMP_SWITCH);
                for (j = pulseDuration; j > 0; j--) {
                        _nop();
                }
                _pin_off(OUT_PUMP_SWITCH);
                delay_us(10);
                _wdr();
        }
}
