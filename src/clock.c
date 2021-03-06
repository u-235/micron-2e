/**
 * \file
 * \brief
 * \details
 *
 * \author Nick Egorrov
 * \date created on: 24.02.2018
 * \copyright GNU Public License 3
 */

#include "compiler.h"
#include "config.h"
#include "sensor.h"

/*************************************************************
 *      Private function prototype
 *************************************************************/

/*************************************************************
 *      Variable in RAM
 *************************************************************/

static unsigned char event = 0;
static unsigned char second_tick = 0;
static unsigned char minute_tick = 0;
static unsigned char hour_tick = 0;
static unsigned int days = 0;

/*************************************************************
 *      Public function
 *************************************************************/

extern void ClockOn()
{
        /* Асинхронный режим работы
         * Предделитель = 128
         * 32768 Hz / 128 / 64 = переполнение 4 раза за секунду
         * Прерывания по переполнению
         */
        ASSR = (1 << AS2);
        TCNT2 = 0;
        OCR2 = 64;
        TCCR2 |= (1 << CS22) | (1 << CS20);
        _interrupt_enable(INT_TIMER2_OVF);
}

extern unsigned char ClockGetEvent()
{
        unsigned char retval = event;
        event = 0;
        return retval;
}

extern unsigned char ClockGetSeconds()
{
        return second_tick;
}

extern unsigned char ClockSetSeconds(unsigned char s)
{
        if (s < 60) {
                second_tick = s;
        }
        return second_tick;
}

extern unsigned char ClockIncSeconds()
{
        second_tick++;
        if (second_tick >= 60) {
                second_tick = 0;
        }
        return second_tick;
}

extern unsigned char ClockGetMimutes()
{
        return minute_tick;
}

extern unsigned char ClockSetMinutes(unsigned char m)
{
        if (m < 60) {
                minute_tick = m;
        }
        return minute_tick;
}

extern unsigned char ClockIncMinutes()
{
        minute_tick++;
        if (minute_tick >= 60) {
                minute_tick = 0;
        }
        return minute_tick;
}

extern unsigned char ClockGetHours()
{
        return hour_tick;
}

extern unsigned char ClockSetHours(unsigned char h)
{
        if (h < 24) {
                hour_tick = h;
        }
        return hour_tick;
}

extern unsigned char ClockIncHours()
{
        hour_tick++;
        if (hour_tick >= 24) {
                hour_tick = 0;
        }
        return hour_tick;
}

extern unsigned int ClockGetDays()
{
        return days;
}

extern void ClockClearDays()
{
        days = 0;
}

/*************************************************************
 *      Interrupt handler
 *************************************************************/

/**
 * Timer overflow interrupt handler
 */
_isr_timer2_ovf(void)
{
        static unsigned char counter = 0;

        event |= CLOCK_EVENT_PORTION;

        if (counter++ < CLOCK_DIVIDER) {
                return;
        }
        counter = 0;

        event |= CLOCK_EVENT_SECOND;

        if (ClockIncSeconds() == 0) {
                event |= CLOCK_EVENT_MINUTE;
                if (ClockIncMinutes() == 0) {
                        event |= CLOCK_EVENT_HOUR;
                        if (ClockIncHours() == 0) {
                                event |= CLOCK_EVENT_DAY;
                                days++;
                        }
                }
        }
}

/*************************************************************
 *      Private function
 *************************************************************/
