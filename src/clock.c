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

/**
 * \file
 * \brief
 * \details
 *
 * \date created on: 24.02.2018
 * \author: nick
 */

#include "compiller.h"
#include "config.h"
#include HEADER_IO
#include HEADER_DELAY
#include "sensor.h"

static unsigned char event = 0;
static unsigned char second_tick = 0;
static unsigned char minute_tick = 0;
static unsigned char hour_tick = 0;
static unsigned int days = 0;

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

extern char ClockSetSeconds(unsigned char s)
{
        second_tick = s;
        if (second_tick > 59) {
                second_tick = 0;
                return -1;
        }
        return 0;
}

extern char ClockIncSeconds()
{
        return ClockSetSeconds(second_tick + 1);
}

extern unsigned char ClockGetMimutes()
{
        return minute_tick;
}

extern char ClockSetMinutes(unsigned char m)
{
        minute_tick = m;
        if (minute_tick > 59) {
                minute_tick = 0;
                return -1;
        }
        return 0;
}

extern char ClockIncMinutes()
{
        return ClockSetMinutes(minute_tick + 1);
}

extern unsigned char ClockGetHours()
{
        return hour_tick;
}
extern char ClockSetHours(unsigned char h)
{
        hour_tick = h;
        if (hour_tick > 23) {
                hour_tick = 0;
                return -1;
        }
        return 0;
}

extern char ClockIncHours()
{
        return ClockSetHours(hour_tick + 1);
}

extern unsigned int ClockGetDays()
{
        return days;
}

extern void ClockClearDays()
{
        days = 0;
}

/**
 * Timer overflow interrupt handler
 */
_isr_timer2_ovf(void)
{
        event |= CLOCK_EVENT_SECOND;

        if (ClockIncSeconds() != 0) {
                event |= CLOCK_EVENT_MINUTE;
                if (ClockIncMinutes() != 0) {
                        event |= CLOCK_EVENT_HOUR;
                        if (ClockIncHours() != 0) {
                                event |= CLOCK_EVENT_DAY;
                                days++;
                        }
                }
        }
}
