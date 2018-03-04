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

#include "target.h"

#define _reset_up(nu)   _pin_on(LCD_RST)
#define _reset_down(nu) _pin_off(LCD_RST)

#define _chip_enable(nu)        _pin_off(LCD_SS)
#define _chip_disable(nu)       _pin_on(LCD_SS)

#define _mode_data(nu)  _pin_on(LCD_DC)
#define _mode_cmd(nu)   _pin_off(LCD_DC)

#define _clock_up(nu)   _pin_on(LCD_CLK)
#define _clock_down(nu) _pin_off(LCD_CLK)

#define _data_up(nu)   _pin_on(LCD_MOSI)
#define _data_down(nu) _pin_off(LCD_MOSI)

static void HardInit()
{
        _dir_out(LCD_RST);
        _dir_out(LCD_DC);
        _dir_out(LCD_MOSI);
        _dir_out(LCD_CLK);
        _dir_out(LCD_SS);
#ifndef SOFT_SPI
        SPCR = 0x50;  // режим
        SPSR = 0x01;// удвоение частоты
#endif
}

static void HardOn(){
        _pin_on(LCD_PWR);
}

static void HardOff(){
        _pin_off(LCD_PWR);
        _pin_off(LCD_DC);
        _pin_off(LCD_RST);
        _pin_off(LCD_MOSI);
        _pin_off(LCD_CLK);
}

/**
 * Send byte to display.
 * @param data byte to transmit.
 */
#ifndef SOFT_SPI
/* Use hardware SPI */
static void LcdSend(unsigned char data) {
        SPDR = data;   //Send data to display controller
        while ( (SPSR & 0x80) != 0x80 ) {
                //Wait until Tx register empty
        }
}

#elif !defined LCD_ROTATE
/* Use bitbang */
static void LcdSend(unsigned char data) {
        unsigned char i, mask = 1<<8;

        for (i = 0; i < 8; i++) {
                if ((data & mask) != 0) {
                        _data_up();
                }
                else {
                        _data_down();
                }
                mask = mask >> 1;
                _clock_up();
                _clock_down();
        }
}

#else
/* Use bitbang reverse */
static void LcdSend(unsigned char data)
{
        unsigned char i, mask = 1;

        for (i = 0; i < 8; i++) {
                if ((data & mask) != 0) {
                        _data_up();
                } else {
                        _data_down();
                }
                mask = mask << 1;
                _clock_up();
                _clock_down();
        }
}

#endif

#endif /* DISPLAY_CHIP_H_ */
