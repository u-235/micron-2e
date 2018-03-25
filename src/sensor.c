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

#include "compiller.h"
#include "config.h"
#include "power.h"
#include "sensor.h"
#include "alarm.h"

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
static void RunCharge(unsigned int ticks);

/*************************************************************
 *      Variable in RAM
 *************************************************************/

static unsigned char impHits;
static unsigned int impCounter;
static unsigned int impulse_massive[SENSOR_INDEX_MAX + 1];

static unsigned long doseHour = 0;
static unsigned long doseDay = 0;
static unsigned long doseAll = 0;
/* Масштабный коэффициент для вычисления относительных уровней радиации в
 * SensorGetLevel(). Равен самому большому значению в массиве.
 */
static unsigned int scale = 6;

/* Длительность положительнного импульса накачки, рабочее значение. */
static unsigned char pulseDuration;
/* Количество импульсов принудительной накачки, рабочее значение. */
static unsigned int ticksPeriodic;
/* Количество импульсов накачки при сработке датчика, рабочее значение. */
static unsigned int ticksHit;

/*************************************************************
 *      Variable in EEPROM
 *************************************************************/

/* Длительность положительнного импульса накачки, настройка. */
static eeprom unsigned char eePulseDuration = PUMP_PULSE_WIDTH_DEFAULT;
/* Количество импульсов принудительной накачки, настройка. */
static eeprom unsigned int eeTicksPeriodic = PUMP_TICKS_PERIODIK_DEFAULT;
/* Количество импульсов накачки при сработке датчика, настройка. */
static eeprom unsigned int eeTicksHit = PUMP_TICKS_HIT_DEFAULT;

/*************************************************************
 *      Public function
 *************************************************************/

extern void InitSensor()
{
        pulseDuration = _eemem_read8(&eePulseDuration);
        ticksPeriodic = _eemem_read16(&eeTicksPeriodic);
        ticksHit = _eemem_read16(&eeTicksHit);
        CalculatePump(GetVoltage());
        RunCharge(6000);
}

extern unsigned char SensorGetPulseDuration()
{
        return pulseDuration;
}

extern void SensorIncPulseDuration()
{
        unsigned char pulse = pulseDuration + 1;
        if (pulse > PUMP_PULSE_WIDTH_MAX) {
                pulse = PUMP_PULSE_WIDTH_MIN;
        }
        pulseDuration = pulse;
        _eemem_write8(&eePulseDuration, pulse);
}

extern unsigned int SensorGetTicksPeriodik()
{
        return ticksPeriodic;
}

extern void SensorIncTicksPeriodik()
{
        unsigned int ticks = ticksPeriodic + 1;
        if (ticks > PUMP_TICKS_PERIODIK_MAX) {
                ticks = PUMP_TICKS_PERIODIK_MIN;
        }
        ticksPeriodic = ticks;
        _eemem_write16(&eeTicksPeriodic, ticks);
}

extern unsigned int SensorGetTicksHit()
{
        return ticksHit;
}

extern void SensorIncTicksHit()
{
        unsigned int ticks = ticksHit + 1;
        if (ticks > PUMP_TICKS_HIT_MAX) {
                ticks = PUMP_TICKS_HIT_MIN;
        }
        ticksHit = ticks;
        _eemem_write16(&eeTicksHit, ticks);
}

extern unsigned int SensorGetRadiation(unsigned char period)
{
        unsigned int retval = 0, i;

        if (--period == 0) {
                return 0;
        }
        if (period > SENSOR_INDEX_MAX) {
                period = SENSOR_INDEX_MAX;
        }

        for (i = 0; i <= period; i++) {
                retval += impulse_massive[i];
        }
        /* Не люблю магию, но тут 3600 - это количество секунд в часе. */
        return retval / (period + 1) * (3600 / SENSOR_SENSITIVITY);
}

extern unsigned long SensorGetDoseHour()
{
        return doseHour / SENSOR_SENSITIVITY;
}

extern unsigned long SensorGetDoseDay()
{
        return doseDay + doseHour / SENSOR_SENSITIVITY;
}

extern unsigned long SensorGetDoseAll()
{
        return doseAll + doseDay + doseHour / SENSOR_SENSITIVITY;
}

extern void SensorClearDose()
{
        doseHour = 0;
        doseDay = 0;
        doseAll = 0;
}

extern unsigned char SensorGetRelative(unsigned char index)
{
        return (unsigned int) impulse_massive[index] * DOSE_HISTOGRAM_SIZE
                        / scale;
}

extern unsigned char SensorIsHit()
{
        unsigned char retval = impHits;
        impHits = 0;
        return retval;
}

/* Количество импульсов принудительной накачки, вычисленное значение. */
static unsigned int realTicksPeriodic;
/* Количество импульсов накачки при сработке датчика, вычисленное значение. */
static unsigned int realTicksHit;

extern void SensorClockEvent(unsigned char event)
{
        int i;

        if (event & CLOCK_EVENT_MINUTE) {
                CalculatePump(GetVoltage());
        }

        if (event & CLOCK_EVENT_SECOND) {
                scale = 6;
                /* смещение массива фона */
                for (i = SENSOR_INDEX_MAX; i >= 0; i--) {
                        unsigned int old;
                        if (i == 0) {
                                old = impCounter;
                        } else {
                                old = impulse_massive[i - 1];
                        }
                        if (old > scale) {
                                scale = old;
                        }
                        impulse_massive[i] = old;
                }
                impCounter = 0;

                GICR = 0x00;
                /* TODO remove led flash */
                led_refresh(0);
                RunCharge(realTicksPeriodic);
                delay_us(25);  //25
                led_refresh(1);
                GICR = 0xC0;
        }

        if (event & CLOCK_EVENT_HOUR) {
                /* TODO Очень странное место. По идее, */
                // 60сек * 60мин / 36 = 100. так проще для МК
                // вот здесь я ввел дополнительную переменную для почасовой дозы.
                // наверное, таким образом баг и починился :D
                doseDay += doseHour / 100;
                doseHour = 0;
        }

        if (event & CLOCK_EVENT_DAY) {
                doseAll += doseDay;
                doseDay = 0;
        }
}

/*************************************************************
 *      Interrupt handler
 *************************************************************/

_isr_ext1(void)
{
        /* TODO Оно тут надо? */
        _pin_off(OUT_BEEPER);
        RunCharge(realTicksHit);
        impHits = 1;
        impCounter++;
        doseHour++;
        _wdr();
        /* TODO Нужно улучшить - отслеживать состояние на входе INT1 */
        delay_us(1);
}

/*************************************************************
 *      Private function
 *************************************************************/

static void CalculatePump(unsigned int voltage)
{
#ifdef POWER_LION
        realTicksHit = (POWER_VOLTAGE_HIGH - SWITCH_DROP_VOLTAGE) * ticksHit
                        / (voltage - SWITCH_DROP_VOLTAGE);
        realTicksPeriodic = (POWER_VOLTAGE_HIGH - SWITCH_DROP_VOLTAGE)
                        * ticksPeriodic / (voltage - SWITCH_DROP_VOLTAGE);
#else // not lithium
        realTicksHit = ticksHit;
        realTicksPeriodic = ticksPeriodic;
#endif
}

static void RunCharge(unsigned int ticks)
{
        unsigned int i, j;

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
