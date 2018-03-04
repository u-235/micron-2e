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
 * \file pcd8544.h
 * \brief
 * \details
 *
 * \date created on: 04.03.2018
 * \author: Nick Egorrov
 */

#ifndef DISPLAY_PCD8544_H_
#define DISPLAY_PCD8544_H_

#ifdef _cplusplus
extern "C" {
#endif


#define CMD_FUNC        (1<<5)
#define MASK_FUNC       (1<<2 | 1<< 1 | 1<<0)
#define FUNC_SET_EXTEND (1<<0)
#define FUNC_SET_BASIC  (0<<0)
#define FUNC_DIR_VERT   (1<<1)
#define FUNC_DIR_HORZ   (0<<1)
#define FUNC_PWR_DOWN   (1<<2)
#define FUNC_PWR_ACTIVE (0<<2)
#define MAKE_FUNC(flags) (CMD_FUNC | ((flags) & MASK_FUNC))

/**
 * Set display configuration.
 */
#define CMD_MODE (1<<3)
#define MASK_MODE (1<<2 | 1<<0)
/**
 * Make configuration using one of mode #LCD_MODE_BLANK, #LCD_MODE_FILLED,
 *  #LCD_MODE_NORMAL, #LCD_MODE_INVERSE.
 */
#define MAKE_MODE(mode)      (CMD_MODE | ((mode)<<1 & 1<<2) | ((mode) & 1<<0))

#define CMD_ADDR_X (1<<7)
#define MASK_ADDR_X (1<<6 | 1<< 5 | 1<<4 | 1<<3 | 1<<2 | 1<< 1 | 1<<0)
#define MAKE_ADDR_X(x) (CMD_ADDR_X |((x) & MASK_ADDR_X))

#define CMD_ADDR_Y (1<<6)
#define MASK_ADDR_Y (1<<2 | 1<< 1 | 1<<0)
#define MAKE_ADDR_Y(y) (CMD_ADDR_Y |((y) & MASK_ADDR_Y))

#define EXT_TEMP (1<<2)
#define MASK_TEMP (1<< 1 | 1<<0)
#define EMAKE_TEMP(t) (EXT_TEMP + ((t) & MASK_TEMP))

#define EXT_BIAS (1<<4)
#define MASK_BIAS (1<<2 | 1<< 1 | 1<<0)
#define EMAKE_BIAS(b) (EXT_BIAS + ((b) & MASK_BIAS))

#define EXT_CONTR (1<<7)
#define MASK_CONTR (1<<6 | 1<< 5 | 1<<4 | 1<<3 | 1<<2 | 1<< 1 | 1<<0)
#define EMAKE_CONTR(c) (EXT_CONTR + ((c) & MASK_CONTR))

#define CHN_SHIFT (1<<6)
#define MASK_SHIFT (1<< 5 | 1<<4 | 1<<3 | 1<<2 | 1<< 1 | 1<<0)
#define CMAKE_SHIFT(s) (CHN_SHIFT + ((s) & MASK_SHIFT))

#define CHN_SPI_MODE (1<<3)
#define MASK_SPI_MODE (1<<2)
#define SPI_MODE_MSB (0<<2)
#define SPI_MODE_LSB (1<<2)
#define CMAKE_SPI_MODE(s) (CHN_SPI_MODE + ((s) & MASK_SPI_MODE))

#ifdef _cplusplus
        extern "C" {
#endif

#endif /* DISPLAY_PCD8544_H_ */
