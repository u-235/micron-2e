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

#ifndef SRC_POWER_H_
#define SRC_POWER_H_

#ifdef _cplusplus
extern "C" {
#endif

extern void InitPower();

extern unsigned int GetSleepTime();

extern void SetSleepTime(unsigned int tm);

extern void IncSleepTime();

/**
 * Проверка состояния источника питания.
 * \return 0 если уровень питания в норме.
 */
extern char CheckPower();

void UpdateWorkTime(char flag);

/**
 *
 * \return Battery voltage in hundredth of volts.
 */
extern unsigned int GetVoltage();

/**
 *
 * \return Battery charge in percent.
 */
extern unsigned char GetCharge();

extern void poweroff();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_POWER_H_ */
