/*****************************************************************************
 *
 *  Nokia 3310 display driver
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
 * \brief Доступ к шрифту.
 * \details Для получения растра символа нужно использовать GetGlif(). Это
 *  позволяет легко менять используемый шрифт или делать в нём изменения не
 *  затрагивая драйвер.
 *
 * \date created on: 03.03.2018
 * \author: Nick Egorrov
 */

#ifndef DISPLAY_FONT_8X5_EN_RUS_H_
#define DISPLAY_FONT_8X5_EN_RUS_H_

#ifdef _cplusplus
extern "C" {
#endif

#define _get_row(ptr)  (*(flash const char*)(ptr))

/**
 * Возвращает указатель на растр указанного символа.
 * \param ch Символ, для которого требуется растр.
 * \return Указатель на растр.
 */
extern char * GetGlif(unsigned char ch);

#ifdef _cplusplus
extern "C" {
#endif

#endif /* DISPLAY_FONT_8X5_EN_RUS_H_ */
