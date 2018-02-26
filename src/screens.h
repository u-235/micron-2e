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

#ifndef SRC_SCREENS_H_
#define SRC_SCREENS_H_

#ifdef _cplusplus
extern "C" {
#endif

/**
 *
 * \return 0 in menu not active
 */
extern char IsMenuActive();

/**
 *
 * \param act 0 for not active menu
 */
extern void SetMenuActive(char act);

extern char IsNeedUpdate();

extern void SetNeedUpdate(char act);

unsigned char DrawMenu(unsigned char menu_select);

void main_window_draw(unsigned char step);

void menu_modification_check(char selected);

extern void DrawIntro();

extern void DrawBay();

#ifdef _cplusplus
extern "C" {
#endif

#endif /* SRC_SCREENS_H_ */
