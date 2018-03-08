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
 * \brief
 * \details  Эта часть драйвера предоставляет функции, специфичные для
 *  конкретного микроконтроллера.
 *
 * \date created on: 03.03.2018
 * \author: Nick Egorrov
 */

#ifndef DISPLAY_CHIP_H_
#define DISPLAY_CHIP_H_

#ifndef LCD_MONO_INSIDE_
#error This file most be included only in lcd-mono.c
#endif

#include "target.h"

#define _reset_up(nu)   _pin_on(LCD_RST)
#define _reset_down(nu) _pin_off(LCD_RST)

#define _chip_enable(nu)        _pin_off(LCD_SS)
#define _chip_disable(nu)       _pin_on(LCD_SS)

#define _mode_data  ModeData
#define _mode_cmd   ModeCMD

#define _clock_up(nu)   _pin_on(LCD_CLK)
#define _clock_down(nu) _pin_off(LCD_CLK)

#define _data_up(nu)   _pin_on(LCD_MOSI)
#define _data_down(nu) _pin_off(LCD_MOSI)

/**
 *  0 for LSB transmit first
 */
static unsigned char order;

static void ModeData()
{
#ifdef LCD_ROTATE
        order = 0;
#else
        order = 1;
#endif
        _pin_on(LCD_DC);
}

static void ModeCMD()
{
        order = 1;
        _pin_off(LCD_DC);
}

static void HardInit()
{
        _dir_out(LCD_RST);
        _dir_out(LCD_DC);
        _dir_out(LCD_MOSI);
        _dir_out(LCD_CLK);
        _dir_out(LCD_SS);
#ifndef SOFT_SPI
        SPCR = (1 << MSTR) | (1 << DORD);
        SPSR = 1 << SPI2X;
#endif
}

static void HardOn()
{
        _pin_on(LCD_PWR);
}

static void HardOff()
{
        _pin_off(LCD_PWR);
        _pin_off(LCD_DC);
        _pin_off(LCD_RST);
        _pin_off(LCD_MOSI);
        _pin_off(LCD_CLK);
}

/**
 * Send byte to display.
 * \param data byte to transmit.
 */
#ifndef SOFT_SPI
/* Use hardware SPI */
static void LcdSend(unsigned char data) {
        /*
         * DORD=0 MSB transmit first
         * DORD=1 LSB first
         */
        if (order == 0) {
                SPCR |= (1 << DORD);
        } else {
                SPCR &= ~(1 << DORD);
        }
        /* Send data to display controller */
        SPDR = data;
        while ( (SPSR & (1 << SPIF) == 0 ) {
                                /* Waiting until  a serial transfer is't complete */
                        }
                }

#else
/* Use bitbang reverse */
static void LcdSend(unsigned char data)
{
        unsigned char i, mask = 1;

        if (order == 0) {
                mask = 1 << 0;
        } else {
                mask = 1 << 7;
        }

        for (i = 0; i < 8; i++) {
                if ((data & mask) != 0) {
                        _data_up();
                } else {
                        _data_down();
                }

                if (order == 0) {
                        mask <<= 1;
                } else {
                        mask >>= 1;
                }

                _clock_up();
                _clock_down();
        }
}

#endif

#endif /* DISPLAY_CHIP_H_ */
