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

#ifndef SRC_CLOCK_H_
#define SRC_CLOCK_H_

#ifdef _cplusplus
extern "C" {
#endif

#define CLOCK_EVENT_SECOND 0x01
#define CLOCK_EVENT_MINUTE 0x02
#define CLOCK_EVENT_HOUR   0x04
#define CLOCK_EVENT_DAY    0x08

/**
 *
 * \return Набор флагов CLOCK_EVENT_xx если состояние часов обновилось или ноль.
 */
extern unsigned char ClockGetEvent();

extern unsigned char ClockGetSeconds();

extern char ClockSetSeconds(unsigned char s);

extern char ClockIncSeconds();

extern unsigned char ClockGetMimutes();

extern char ClockSetMinutes(unsigned char m);

extern char ClockIncMinutes();

extern unsigned char ClockGetHours();

extern char ClockSetHours(unsigned char h);

extern char ClockIncHours();

extern unsigned int ClockGetDays();

extern void ClockClearDays();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_CLOCK_H_ */
