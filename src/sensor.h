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

#ifndef SRC_SENSOR_H_
#define SRC_SENSOR_H_

#include "clock.h"

#ifdef _cplusplus
extern "C" {
#endif

#define SENSOR_INDEX_MAX 35

extern void InitSensor();

/**
 * Обновление данных датчика в зависимости от времени.
 * \param event Набор флагов CLOCK_EVENT_xx см. "clock.h"
 */
extern void SensorClockEvent(unsigned char event);

/**
 *
 * \return Ненулевое значение если была сработка датчика.
 */
extern unsigned char SensorIsHit();

/**
 * Возвращает относительный уровень радиации.
 * \param index Индекс значения уровня. Диаппазон от 0 до #SENSOR_INDEX_MAX.
 * \return Относительный уровень радиации в диаппазоне от 0 до
 *       #DOSE_HISTOGRAM_SIZE
 */
extern unsigned char SensorGetRelative(unsigned char index);

/*************************************************************
 *      Dose section
 *************************************************************/

/**
 * Возвращает уровень радиации за указанный период в мкР/ч
 * @param period Период в секундах, от 1 до SENSOR_INDEX_MAX + 1
 * @return Уровень радиации в мкР/ч
 */
extern unsigned int SensorGetRadiation(unsigned char period);

extern unsigned long SensorGetDoseHour();

extern unsigned long SensorGetDoseDay();

extern unsigned long SensorGetDoseAll();

extern void SensorClearDose();

/*************************************************************
 *      Setup section
 *************************************************************/

extern unsigned char SensorGetPulseDuration();

extern void SensorIncPulseDuration();

extern unsigned int SensorGetTicksPeriodik();

extern void SensorIncTicksPeriodik();

extern unsigned int SensorGetTicksHit();

extern void SensorIncTicksHit();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_SENSOR_H_ */
