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

/**********************************************
 ****************PCD8544 Driver*****************
 ***********************************************

 for original NOKIA 3310 & alternative "chinese" version of display

 48x84 dots, 6x14 symbols

 **********************************************/

#include "n3310lcd.h"
#include "../compiller.h"
#include "../config.h"

#include HEADER_IO
#include HEADER_DELAY
#include <stdio.h>


#ifndef SOFT_SPI
unsigned char SPCR_val = 0x50;	// режим
unsigned char SPSR_val = 0x01;// удвоение частоты
#endif

//***********************************************************
//Настройки контроллера дисплея и переменные для работы с ним
//***********************************************************

//#pragma used+

char addressing = 0;//направление адресации: 0 - горизонтальная, 1- вертикальная
//bit instuct_set = 0;			//набор инструкций: 0 - стандартный, 1 - расширенный - в текущей версии не используется

#ifdef LCD_NOKIA3310_CHINA
bit x_mirror = 0;		//зеркалирование по X: 0 - выкл., 1 - вкл.
bit y_mirror = 0;//зеркалирование по Y: 0 - выкл., 1 - вкл.
bit SPI_invert = 0;//порядок битов в SPI: 0 - MSB first, 1 - LSB first
#endif

//unsigned char set_y;			//адрес по У, 0..5 - в текущей версии не используется
//unsigned char set_x;                 	//адрес по Х, 0..83 - в текущей версии не используется
//unsigned char temp_control = 3;  	//температурный коэффициент, 0..3
//unsigned char bias = 3;              //смещение, 0..7
unsigned char Vop = 55;  //рабочее напрядение LCD, 0..127 (определяет контрастность) //80
unsigned char disp_config = 2;	//режим дисплея: 0 - blank, 1 - all on, 2 - normal, 3 - inverse

#ifdef LCD_NOKIA3310_CHINA
unsigned char shift = 5;		//0..3F - сдвиг экрана вверх, в точках
#endif

#define PIXEL_OFF	0		//режимы отображения пикселя - используются в графических функциях
#define PIXEL_ON	1
#define PIXEL_XOR	2

#define LCD_X_RES               84	//разрешение экрана
#define LCD_Y_RES               48
#define LCD_CACHSIZE          LCD_X_RES*LCD_Y_RES/8

#define Cntr_X_RES              102    	//разрешение контроллера - предполагаемое - но работает))
#define Cntr_Y_RES              64
#define Cntr_buf_size           Cntr_X_RES*Cntr_Y_RES/8

unsigned char LcdCache[LCD_CACHSIZE];  //Cache buffer in SRAM 84*48 bits or 504 bytes
unsigned int LcdCacheIdx;              	//Cache index

#define LCD_CMD         0
#define LCD_DATA        1

//***************************************************
//****************Прототипы функций******************
//***************************************************

//Sends data to display controller
static void LcdSend(unsigned char data, unsigned char cmd);

flash char table[0x01E0] = {
                0x00, 0x00, 0x00, 0x00, 0x00,   // 20 space
                0x00, 0x00, 0x5F, 0x00, 0x00,   // 21 !
                0x00, 0x07, 0x00, 0x07, 0x00,   // 22 "
                0x08, 0x2C, 0x30, 0x2C, 0x08,   // 23 # rad
                0x23, 0x13, 0x08, 0x64, 0x62,     // 24 $ %
                0x23, 0x13, 0x08, 0x64, 0x62,   // 25 %
                0x36, 0x49, 0x55, 0x22, 0x50,   // 26 &
                0x00, 0x05, 0x03, 0x00, 0x00,   // 27 '
                0x00, 0x1C, 0x22, 0x41, 0x00,   // 28 (
                0x00, 0x41, 0x22, 0x1C, 0x00,   // 29 )
                0x14, 0x08, 0x3E, 0x08, 0x14,   // 2a *
                0x08, 0x08, 0x3E, 0x08, 0x08,   // 2b +
                0x00, 0x50, 0x30, 0x00, 0x00,   // 2c ,
                0x08, 0x08, 0x08, 0x08, 0x08,   // 2d -
                0x00, 0x60, 0x60, 0x00, 0x00,   // 2e .
                0x20, 0x10, 0x08, 0x04, 0x02,   // 2f /
                0x3E, 0x51, 0x49, 0x45, 0x3E,   // 30 0
                0x00, 0x42, 0x7F, 0x40, 0x00,   // 31 1
                0x42, 0x61, 0x51, 0x49, 0x46,   // 32 2
                0x21, 0x41, 0x45, 0x4B, 0x31,   // 33 3
                0x18, 0x14, 0x12, 0x7F, 0x10,   // 34 4
                0x27, 0x45, 0x45, 0x45, 0x39,   // 35 5
                0x3C, 0x4A, 0x49, 0x49, 0x30,   // 36 6
                0x01, 0x71, 0x09, 0x05, 0x03,   // 37 7
                0x36, 0x49, 0x49, 0x49, 0x36,   // 38 8
                0x06, 0x49, 0x49, 0x29, 0x1E,   // 39 9
                0x00, 0x36, 0x36, 0x00, 0x00,   // 3a :
                0x00, 0x56, 0x36, 0x00, 0x00,   // 3b ;
                0x08, 0x14, 0x22, 0x41, 0x00,   // 3c <
                0x14, 0x14, 0x14, 0x14, 0x14,   // 3d =
                0x00, 0x41, 0x22, 0x14, 0x08,   // 3e >
                0x02, 0x01, 0x51, 0x09, 0x06,   // 3f ?
                0x32, 0x49, 0x79, 0x41, 0x3E,   // 40 @
                0x7E, 0x11, 0x11, 0x11, 0x7E,   // 41 A
                0x7F, 0x49, 0x49, 0x49, 0x36,   // 42 B
                0x3E, 0x41, 0x41, 0x41, 0x22,   // 43 C
                0x7F, 0x41, 0x41, 0x22, 0x1C,   // 44 D
                0x7F, 0x49, 0x49, 0x49, 0x41,   // 45 E
                0x7F, 0x09, 0x09, 0x09, 0x01,   // 46 F
                0x3E, 0x41, 0x49, 0x49, 0x7A,   // 47 G
                0x7F, 0x08, 0x08, 0x08, 0x7F,   // 48 H
                0x00, 0x41, 0x7F, 0x41, 0x00,   // 49 I
                0x20, 0x40, 0x41, 0x3F, 0x01,   // 4a J
                0x7F, 0x08, 0x14, 0x22, 0x41,   // 4b K
                0x7F, 0x40, 0x40, 0x40, 0x40,   // 4c L
                0x7F, 0x02, 0x0C, 0x02, 0x7F,   // 4d M
                0x7F, 0x04, 0x08, 0x10, 0x7F,   // 4e N
                0x3E, 0x41, 0x41, 0x41, 0x3E,   // 4f O
                0x7F, 0x09, 0x09, 0x09, 0x06,   // 50 P
                0x3E, 0x41, 0x51, 0x21, 0x5E,   // 51 Q
                0x7F, 0x09, 0x19, 0x29, 0x46,   // 52 R
                0x46, 0x49, 0x49, 0x49, 0x31,   // 53 S
                0x01, 0x01, 0x7F, 0x01, 0x01,   // 54 T
                0x3F, 0x40, 0x40, 0x40, 0x3F,   // 55 U
                0x1F, 0x20, 0x40, 0x20, 0x1F,   // 56 V
                0x3F, 0x40, 0x38, 0x40, 0x3F,   // 57 W
                0x63, 0x14, 0x08, 0x14, 0x63,   // 58 X
                0x07, 0x08, 0x70, 0x08, 0x07,   // 59 Y
                0x61, 0x51, 0x49, 0x45, 0x43,   // 5a Z
                0x00, 0x7F, 0x41, 0x41, 0x00,   // 5b [
                0x02, 0x04, 0x08, 0x10, 0x20,   // 5c Yen Currency Sign
                0x00, 0x41, 0x41, 0x7F, 0x00,   // 5d ]
                0x7E, 0x7F, 0x7F, 0x7E, 0x00,   // 5e ^ *batt
                0x40, 0x40, 0x40, 0x40, 0x40,   // 5f _
                0x00, 0x01, 0x02, 0x04, 0x00,   // 60 `
                0x20, 0x54, 0x54, 0x54, 0x78,   // 61 a
                0x7F, 0x48, 0x44, 0x44, 0x38,   // 62 b
                0x38, 0x44, 0x44, 0x44, 0x20,   // 63 c
                0x38, 0x44, 0x44, 0x48, 0x7F,   // 64 d
                0x38, 0x54, 0x54, 0x54, 0x18,   // 65 e
                0x08, 0x7E, 0x09, 0x01, 0x02,   // 66 f
                0x0C, 0x52, 0x52, 0x52, 0x3E,   // 67 g
                0x7F, 0x08, 0x04, 0x04, 0x78,   // 68 h
                0x00, 0x44, 0x7D, 0x40, 0x00,   // 69 i
                0x20, 0x40, 0x44, 0x3D, 0x00,   // 6a j
                0x7F, 0x10, 0x28, 0x44, 0x00,   // 6b k
                0x00, 0x41, 0x7F, 0x40, 0x00,   // 6c l
                0x7C, 0x04, 0x18, 0x04, 0x78,   // 6d m
                0x7C, 0x08, 0x04, 0x04, 0x78,   // 6e n
                0x38, 0x44, 0x44, 0x44, 0x38,   // 6f o
                0x7C, 0x14, 0x14, 0x14, 0x08,   // 70 p
                0x08, 0x14, 0x14, 0x18, 0x7C,   // 71 q
                0x7C, 0x08, 0x04, 0x04, 0x08,   // 72 r
                0x08, 0x54, 0x54, 0x54, 0x20,   // 73 s
                0x04, 0x3F, 0x44, 0x40, 0x20,   // 74 t
                0x3C, 0x40, 0x40, 0x20, 0x7C,   // 75 u
                0x1C, 0x20, 0x40, 0x20, 0x1C,   // 76 v
                0x3C, 0x40, 0x30, 0x40, 0x3C,   // 77 w
                0x44, 0x28, 0x10, 0x28, 0x44,   // 78 x
                0x0C, 0x50, 0x50, 0x50, 0x3C,   // 79 y
                0x44, 0x64, 0x54, 0x4C, 0x44,   // 7a z
                0x00, 0x08, 0x36, 0x41, 0x00,   // 7b <
                0x00, 0x00, 0x7F, 0x00, 0x00,   // 7c |
                0x00, 0x41, 0x36, 0x08, 0x00,   // 7d >
                0x10, 0x08, 0x08, 0x10, 0x08,   // 7e Right Arrow ->
                0x78, 0x46, 0x41, 0x46, 0x78
// 7f Left Arrow <-
                };

flash char table_rus[0x0140] = {
                0x7E, 0x11, 0x11, 0x11, 0x7E,  // C0 А
                0x7F, 0x49, 0x49, 0x49, 0x31,  // C1 Б
                0x7F, 0x49, 0x49, 0x49, 0x36,  // C2 В
                0x7F, 0x01, 0x01, 0x01, 0x03,  // C3 Г
                0x60, 0x3E, 0x21, 0x21, 0x7F,  // C4 Д
                0x7F, 0x49, 0x49, 0x49, 0x41,  // C5 Е
                0x77, 0x08, 0x7F, 0x08, 0x77,  // C6 Ж
                0x22, 0x41, 0x49, 0x49, 0x36,  // C7 З
                0x7F, 0x10, 0x08, 0x04, 0x7F,  // C8 И
                0x7F, 0x10, 0x09, 0x04, 0x7F,  // C9 И
                0x7F, 0x08, 0x14, 0x22, 0x41,  // CA К
                0x40, 0x3E, 0x01, 0x01, 0x7F,  // CB Л
                0x7F, 0x02, 0x0C, 0x02, 0x7F,  // CC М
                0x7F, 0x08, 0x08, 0x08, 0x7F,  // CD Н
                0x3E, 0x41, 0x41, 0x41, 0x3E,  // CE О
                0x7F, 0x01, 0x01, 0x01, 0x7F,  // CF П
                0x7F, 0x09, 0x09, 0x09, 0x06,  // D0 Р
                0x3E, 0x41, 0x41, 0x41, 0x22,  // D1 С
                0x01, 0x01, 0x7F, 0x01, 0x01,  // D2 Т
                0x27, 0x48, 0x48, 0x48, 0x3F,  // D3 У
                0x1E, 0x21, 0x7F, 0x21, 0x1E,  // D4 Ф
                0x63, 0x14, 0x08, 0x14, 0x63,  // D5 Х
                0x3F, 0x20, 0x20, 0x3F, 0x60,  // D6 Ц
                0x07, 0x08, 0x08, 0x08, 0x7F,  // D7 Ч
                0x7F, 0x40, 0x7F, 0x40, 0x7F,  // D8 Ш
                0x3F, 0x20, 0x3F, 0x20, 0x7F,  // D9 Щ
                0x01, 0x7F, 0x48, 0x48, 0x30,  // DA Ъ
                0x7F, 0x48, 0x30, 0x00, 0x7F,  // DB Ы
                0x00, 0x7F, 0x48, 0x48, 0x30,  // DC Ь
                0x22, 0x41, 0x49, 0x49, 0x3E,  // DD Э
                0x7F, 0x08, 0x3E, 0x41, 0x3E,  // DE Ю
                0x46, 0x29, 0x19, 0x09, 0x7F,  // DF Я
                0x20, 0x54, 0x54, 0x54, 0x78,  // E0 а
                0x3C, 0x4A, 0x4A, 0x4A, 0x30,  // E1 б
                0x7C, 0x54, 0x54, 0x28, 0x00,  // E2 в
                0x7C, 0x04, 0x04, 0x04, 0x04,  // E3 г
                0x60, 0x38, 0x24, 0x24, 0x7C,  // E4 д
                0x38, 0x54, 0x54, 0x54, 0x18,  // E5 е
                0x6C, 0x10, 0x7C, 0x10, 0x6C,  // E6 ж
                0x00, 0x44, 0x54, 0x54, 0x28,  // E7 з
                0x7C, 0x20, 0x10, 0x08, 0x7C,  // E8 и
                0x7C, 0x21, 0x12, 0x09, 0x7C,  // E9 й
                0x7C, 0x10, 0x28, 0x44, 0x00,  // EA к
                0x40, 0x38, 0x04, 0x04, 0x7C,  // EB л
                0x7C, 0x08, 0x10, 0x08, 0x7C,  // EC м
                0x7C, 0x10, 0x10, 0x10, 0x7C,  // ED н
                0x38, 0x44, 0x44, 0x44, 0x38,  // EE о
                0x7C, 0x04, 0x04, 0x04, 0x7C,  // EF п
                0x7C, 0x14, 0x14, 0x14, 0x08,  // F0 р
                0x38, 0x44, 0x44, 0x44, 0x00,  // F1 с
                0x04, 0x04, 0x7C, 0x04, 0x04,  // F2 т
                0x0C, 0x50, 0x50, 0x50, 0x3C,  // F3 у
                0x08, 0x14, 0x7C, 0x14, 0x08,  // F4 ф
                0x44, 0x28, 0x10, 0x28, 0x44,  // F5 х
                0x3C, 0x20, 0x20, 0x3C, 0x60,  // F6 ц
                0x0C, 0x10, 0x10, 0x10, 0x7C,  // F7 ч
                0x7C, 0x40, 0x7C, 0x40, 0x7C,  // F8 ш
                0x3C, 0x20, 0x3C, 0x20, 0x7C,  // F9 щ
                0x04, 0x7C, 0x50, 0x50, 0x20,  // FA ъ
                0x7C, 0x50, 0x20, 0x00, 0x7C,  // FB ы
                0x00, 0x7C, 0x50, 0x50, 0x20,  // FC ь
                0x28, 0x44, 0x54, 0x54, 0x38,  // FD э
                0x7C, 0x10, 0x38, 0x44, 0x38,  // FE ю
                0x48, 0x54, 0x34, 0x14, 0x7C
// FF я
                };

void LcdSend(unsigned char data, unsigned char cmd)  //Sends data to display controller
{
#ifdef SOFT_SPI
        unsigned char i, mask = 128;
#endif

        //LCD_CPORT.LCD_CPIN = 0;      //Enable display controller (active low)
        _pin_off(LCD_SS);
        if (cmd) {
                //LCD_DC_PORT.LCD_DC_PIN = 1;      //выбираем команда или данные
                _pin_on(LCD_DC);
        } else {
                //LCD_DC_PORT.LCD_DC_PIN = 0;
                _pin_off(LCD_DC);
        }
#ifdef SOFT_SPI
        if (cmd == LCD_DATA) {
#ifdef reverse_display
                mask = 1;
                for (i = 0; i < 8; i++) {
                        if ((data & mask) != 0) {
                                //LCD_MOSI_PORT.LCD_MOSI_PIN = 1;
                                _pin_on(LCD_MOSI);
                        } else {
                                //LCD_MOSI_PORT.LCD_MOSI_PIN = 0;
                                _pin_off(LCD_MOSI);
                        }
                        mask = mask << 1;
                        //LCD_CLK_PORT.LCD_CLK_PIN = 1;
                        //LCD_CLK_PORT.LCD_CLK_PIN = 0;
                        _pin_on(LCD_CLK);
                        _pin_off(LCD_CLK);
                }
#else
                for (i = 0; i < 8; i++) {
                        if ((data&mask)!=0) {
                                //LCD_MOSI_PORT.LCD_MOSI_PIN = 1;
                                _pin_on(LCD_MOSI);
                        }
                        else {
                                //LCD_MOSI_PORT.LCD_MOSI_PIN = 0;
                                _pin_off(LCD_MOSI);
                        }
                        mask = mask >>1;
                        //LCD_CLK_PORT.LCD_CLK_PIN = 1;
                        //LCD_CLK_PORT.LCD_CLK_PIN = 0;
                        _pin_on(LCD_CLK);
                        _pin_off(LCD_CLK);
                }
#endif
        } else {
                for (i = 0; i < 8; i++) {
                        if ((data & mask) != 0) {
                                //LCD_MOSI_PORT.LCD_MOSI_PIN = 1;
                                _pin_on(LCD_MOSI);
                        } else {
                                //LCD_MOSI_PORT.LCD_MOSI_PIN = 0;
                                _pin_off(LCD_MOSI);
                        }
                        mask = mask >> 1;
                        //LCD_CLK_PORT.LCD_CLK_PIN = 1;
                        //LCD_CLK_PORT.LCD_CLK_PIN = 0;
                        _pin_on(LCD_CLK);
                        _pin_off(LCD_CLK);
                }
        }
#endif

#ifndef SOFT_SPI
        SPDR = data;   //Send data to display controller
        while ( (SPSR & 0x80) != 0x80 ) {
                //Wait until Tx register empty
        }
#endif

        //LCD_CPORT.LCD_CPIN = 1;             //Disable display controller
        _pin_on(LCD_SS);
}

void LcdUpdate(void)   //Copies the LCD cache into the device RAM
{
        int i;
#ifdef LCD_NOKIA3310_CHINA
        char j;
#endif

        LcdSend(0x80, LCD_CMD);  //команды установки указателя памяти дисплея на 0,0
        LcdSend(0x40, LCD_CMD);

#ifdef LCD_NOKIA3310_CHINA                    		//если китайский дисплей - грузим пустую строку
        for (j = Cntr_X_RES; j>0; j--) {
                LcdSend(0, LCD_DATA);
        }
#endif
#ifdef reverse_display
        for (i = LCD_CACHSIZE - 1; i >= 0; i--)		//грузим данные
#else
                        for (i = 0; i < LCD_CACHSIZE; i++)//грузим данные
#endif
                        {
                LcdSend(LcdCache[i], LCD_DATA);
#ifdef LCD_NOKIA3310_CHINA				//если дисплей китайский - догружаем каждую строку до размера его буфера
                if (++j == LCD_X_RES)
                {
                        for (j = (Cntr_X_RES-LCD_X_RES); j>0; j--) {
                                LcdSend(0, LCD_DATA);
                        }
                        j=0;
                }
#endif
        }
}

void LcdClear(void)    //Clears the display
{
        int i = LCD_CACHSIZE;

        while (i) {
                LcdCache[--i] = 0;
        }
        LcdUpdate();
}

void LcdInit(void)	//инициализация SPI и дисплея
{
        //LCD_RST_PORT.LCD_RST_PIN = 1;    //настроили порты ввода/вывода
        _pin_on(LCD_RST);
        //LCD_RST_DDR.LCD_RST_PIN = LCD_DC_DDR.LCD_DC_PIN = LCD_CDDR.LCD_CPIN = LCD_MOSI_DDR.LCD_MOSI_PIN = LCD_CLK_DDR.LCD_CLK_PIN = 1;
        _dir_out(LCD_RST);
        _dir_out(LCD_DC);
        _dir_out(LCD_MOSI);
        _dir_out(LCD_CLK);
        //LCD_CLK_PORT.LCD_CLK_PIN = 0;
        _pin_off(LCD_CLK);
        //LCD_SS_DDR.LCD_SS_PIN = 1;
        _dir_out(LCD_SS);
        //LCD_SS_PORT.LCD_SS_PIN = 0;
        _pin_off(LCD_SS);
        delay_ms(1);
#ifndef SOFT_SPI
        SPCR = SPCR_val;
        SPSR = SPSR_val;
#endif
        //LCD_RST_PORT.LCD_RST_PIN = 0;    //дернули ресет
        _pin_off(LCD_RST);
        delay_ms(20);
        //LCD_RST_PORT.LCD_RST_PIN = 1;
        _pin_on(LCD_RST);
        //Enable SPI port: No interrupt, MSBit first, Master mode, CPOL->0, CPHA->0, Clk/4

        //LCD_CPORT.LCD_CPIN = 1;//Disable LCD controller
        _pin_on(LCD_SS);

        /*        LcdSend(0x21, LCD_CMD);
         LcdSend(0xC5, LCD_CMD);
         LcdSend(0x06, LCD_CMD);
         LcdSend(0x20, LCD_CMD); //<--------
         LcdSend(0x20, LCD_CMD);
         LcdSend(0x0C, LCD_CMD);
         */
        LcdSend(0b00100001, LCD_CMD);  //LCD Extended Commands
        LcdSend(0b00000111, LCD_CMD);  //Set Temp coefficent 3

#ifdef LCD_NOKIA3310_CHINA
        LcdSend( 0b00001000|SPI_invert<<3, LCD_CMD );  //порядок битов в SPI
#endif

        LcdSend(0b00010011, LCD_CMD); 			//LCD bias mode 1:48 (3)

#ifdef LCD_NOKIA3310_CHINA
        LcdSend( 0b01000000+shift, LCD_CMD );  //первая строка выше экрана, отображаем со второй
#endif

        LcdSend(0b10000000 + Vop, LCD_CMD); 		//Set LCD Vop (Contrast)

#ifdef LCD_NOKIA3310_CHINA
        LcdSend( 0x20|x_mirror<<4|y_mirror<<3|power_down<<2, LCD_CMD );  //LCD Standard Commands
#endif

        LcdSend(0x20 | 1 << 2 | addressing << 1, LCD_CMD);  //XXXX			//LCD Standard Commands
        LcdSend(0b00001000 | ((disp_config << 1 | disp_config) & 0b00000101),
        LCD_CMD);  //LCD mode

        LcdClear();
}

void LcdContrast(unsigned char contrast)  //contrast -> Contrast value from 0x00 to 0x7F
{
        if (contrast > 0x7F) {
                return;
        }
        LcdSend(0x21, LCD_CMD);            //LCD Extended Commands
        LcdSend(0x80 | contrast, LCD_CMD);    //Set LCD Vop (Contrast)
        LcdSend(0x20, LCD_CMD);  //LCD Standard Commands,Horizontal addressing mode
}

void LcdMode(unsigned char mode)  //режим дисплея: 0 - blank, 1 - all on, 2 - normal, 3 - inverse
{
        if (mode > 3) {
                return;
        }
        LcdSend(0b00001000 | ((mode << 1 | mode) & 0b00000101), LCD_CMD);  //LCD mode
}

void Lcd_off(void) 				//выкл дисплея
{
#ifdef LCD_NOKIA3310_CHINA
        LcdSend( 0x20|x_mirror<<4|y_mirror<<3|1<<2|addressing<<1, LCD_CMD );  //LCD Standard Commands
#else
        LcdSend(0x20 | 1 << 2 | addressing << 1, LCD_CMD);
#endif
}

void Lcd_on(void) 				//вкл дисплея
{
#ifdef LCD_NOKIA3310_CHINA
        LcdSend( 0x20|x_mirror<<4|y_mirror<<3|0<<2|addressing<<1, LCD_CMD );  //LCD Standard Commands
#else
        LcdSend(0x20 | 0 << 2 | addressing << 1, LCD_CMD);
#endif
}

void LcdImage(const unsigned char *imageData)	//вывод изображения
{
        unsigned int i;

        LcdSend(0x80, LCD_CMD);		//ставим указатель на 0,0
        LcdSend(0x40, LCD_CMD);
        for (i = 0; i < LCD_CACHSIZE; i++) {
                LcdCache[i] = imageData[i];		//грузим данные
        }
}

void LcdPixel(unsigned char x, unsigned char y, unsigned char mode)  //Displays a pixel at given absolute (x, y) location, mode -> Off, On or Xor
{
        int index;
        unsigned char offset, data;

        if (x > LCD_X_RES) {
                return;  //если передали в функцию муть - выходим
        }
        if (y > LCD_Y_RES) {
                return;
        }

        index = (((int) (y) / 8) * 84) + x;  //считаем номер байта в массиве памяти дисплея
        offset = y - ((y / 8) * 8);       //считаем номер бита в этом байте

        data = LcdCache[index];      //берем байт по найденному индексу

        if (mode == PIXEL_OFF) {
                data &= (~(0x01 << offset));  //редактируем бит в этом байте
        } else if (mode == PIXEL_ON) {
                data |= (0x01 << offset);
        } else if (mode == PIXEL_XOR) {
                data ^= (0x01 << offset);
        }

        LcdCache[index] = data;		//загружаем байт назад
}

void LcdLine(int x1, int y1, int x2, int y2, unsigned char mode)  //Draws a line between two points on the display - по Брезенхейму
{
        signed int dy = 0;
        signed int dx = 0;
        signed int stepx = 0;
        signed int stepy = 0;
        signed int fraction = 0;

        if (x1 > LCD_X_RES || x2 > LCD_X_RES || y1 > LCD_Y_RES || y2 > LCD_Y_RES) {
                return;
        }

        dy = y2 - y1;
        dx = x2 - x1;
        if (dy < 0) {
                dy = -dy;
                stepy = -1;
        } else {
                stepy = 1;
        }
        if (dx < 0) {
                dx = -dx;
                stepx = -1;
        } else {
                stepx = 1;
        }
        dy <<= 1;
        dx <<= 1;
        LcdPixel(x1, y1, mode);
        if (dx > dy) {
                fraction = dy - (dx >> 1);
                while (x1 != x2) {
                        if (fraction >= 0) {
                                y1 += stepy;
                                fraction -= dx;
                        }
                        x1 += stepx;
                        fraction += dy;
                        LcdPixel(x1, y1, mode);
                }
        } else {
                fraction = dx - (dy >> 1);
                while (y1 != y2) {
                        if (fraction >= 0) {
                                x1 += stepx;
                                fraction -= dy;
                        }
                        y1 += stepy;
                        fraction += dx;
                        LcdPixel(x1, y1, mode);
                }
        }
}

void LcdCircle(char x, char y, char radius, unsigned char mode)  //рисуем круг по координатам с радиусом - по Брезенхейму
{
        signed char xc = 0;
        signed char yc = 0;
        signed char p = 0;

        if (x > LCD_X_RES || y > LCD_Y_RES) {
                return;
        }

        yc = radius;
        p = 3 - (radius << 1);
        while (xc <= yc) {
                LcdPixel(x + xc, y + yc, mode);
                LcdPixel(x + xc, y - yc, mode);
                LcdPixel(x - xc, y + yc, mode);
                LcdPixel(x - xc, y - yc, mode);
                LcdPixel(x + yc, y + xc, mode);
                LcdPixel(x + yc, y - xc, mode);
                LcdPixel(x - yc, y + xc, mode);
                LcdPixel(x - yc, y - xc, mode);
                if (p < 0) p += (xc++ << 2) + 6;
                else p += ((xc++ - yc--) << 2) + 10;
        }
}

void Batt(int x1, int y1)	//рисуем батарейку
{
        LcdLine(x1, y1, (x1 + 5), y1, 1);  //up
        LcdLine(x1 + 1, (y1 + 9), (x1 + 1 + 25), (y1 + 9), 1);  //down
        LcdLine(x1 + 1, y1, x1 + 1, (y1 + 9), 1);  //left
        LcdLine((x1 + 1 + 25), y1, (x1 + 1 + 25), (y1 + 9), 1);  //right
        LcdLine(x1, y1 + 3, x1, y1 + 6, 1);  // пимпочка
}

void LcdBar(int x1, int y1, int x2, int y2, unsigned char persent)  //рисуем прогресс-бар
{
        unsigned char line;
        if (persent > 100) {
                return;
        }
        LcdLine(x1 + 2, y2, x2 - 2, y2, 1);  //down
        LcdLine(x2 - 2, y1, x2 - 2, y2, 1);  //right
        LcdLine(x1 + 2, y1, x1 + 2, y2, 1);  //left
        LcdLine(x1 + 2, y1, x2 - 2, y1, 1);  //up

        LcdLine(x2 - 1, y1 + 1, x2 - 1, y2 - 1, 1);  //right
        LcdLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, 1);  //left

        LcdLine(x2, y1 + 2, x2, y2 - 2, 1);  //right
        LcdLine(x1, y1 + 2, x1, y2 - 2, 1);  //left

        line = persent * (x2 - x1 - 7) / 100 - 1;
        LcdLine(x1 + 4, y1 + 2, x2 - 4, y2 - 2, 0);
        LcdLine(x1 + 4, y1 + 2, x1 + 4 + line, y2 - 2, 1);
}

void LcdBarLine(unsigned char line, unsigned char persent)  //рисуем прошресс-бар
{
        LcdBar(0, (line - 1) * 7 + 1, 83, (line - 1) * 7 + 5, persent);
}

void LcdSetTextPos(unsigned char x, unsigned char y)  //Sets cursor location to xy location. Range: 1,1 .. 14,6
{
        if (x <= 14 && y <= 6) {
                LcdCacheIdx = ((int) (y) - 1) * 84 + ((int) (x) - 1) * 6;
        }
}

const char * GetGlif(unsigned char ch)
{
        if (ch > 0x7f) {
                return &table_rus[(ch * 5 - 0x3C0)];
        } else {
                return &table[(ch * 5 - 0xA0)];
        }
}

  //Displays a character at current cursor location and increment cursor location
void LcdChr(char ch, char inv)
{
        unsigned char i, row;
        const char *glif;

        glif=GetGlif(ch);

        for (i = 0; i < 5; i++) {
                row = *glif;
                if (inv) {
                        row = ~row;
                }
                LcdCache[LcdCacheIdx++] = row;
        }

        if (inv) {
                row = 0xff;
        } else {
                row = 0;
        }
        //добавляем пробел между символами
        LcdCache[LcdCacheIdx++] = row;
}

unsigned char StretchRow(unsigned char row){
        unsigned char retval;

        retval = (row & 0x01) * 3;
        retval |= (row & 0x02) * 6;
        retval |= (row & 0x04) * 12;
        retval |= (row & 0x08) * 24;
        return retval;
}

void LcdChrBold(char ch, char inv)  //Displays a bold character at current cursor location and increment cursor location
{
        unsigned char i, a = 0, b = 0, row = 0;
        const char *glif;

        glif=GetGlif(ch);

        for (i = 0; i < 5; i++) {
                row = *glif;
                if (inv) {
                        row = ~row;
                }

                b = StretchRow(row );      //"растягиваем" столбец на два байта
                row >>= 4;
                a = StretchRow(row );

                LcdCache[LcdCacheIdx] = b;  //копируем байты в экранный буфер
                LcdCache[LcdCacheIdx + 1] = b;  //дублируем для получения жирного шрифта
                LcdCache[LcdCacheIdx + 84] = a;
                LcdCache[LcdCacheIdx + 85] = a;
                LcdCacheIdx = LcdCacheIdx + 2;
        }


        if (inv) {
                row = 0xff;
        } else {
                row = 0;
        }
        //для пробела между символами
        LcdCache[LcdCacheIdx++] = row;
        LcdCache[LcdCacheIdx++] = row;
}

void LcdChrBig(char ch, char inv)  //Displays a character at current cursor location and increment cursor location
{
        unsigned char i;
        unsigned char a = 0, b = 0, row = 0;
        const char *glif;

        glif=GetGlif(ch);

        for (i = 0; i < 5; i++) {
                row = *glif;
                if (inv) {
                        row = ~row;
                }

                b = StretchRow(row );      //"растягиваем" столбец на два байта
                row >>= 4;
                a = StretchRow(row );

                LcdCache[LcdCacheIdx] = b;
                LcdCache[LcdCacheIdx + 84] = a;
                LcdCacheIdx = LcdCacheIdx + 1;
        }

        if (inv) {
                row = 0xff;
        } else {
                row = 0;
        }
        LcdCache[LcdCacheIdx++] = row;
}

void LcdString(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 14 || y > 6) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 15 - x; i++) {
                if (msg[i]) {
                        LcdChr(msg[i], 0);
                }
        }
}

void LcdStringInv(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 14 || y > 6) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 15 - x; i++) {
                if (msg[i]) {
                        LcdChr(msg[i], 1);
                }
        }
}

void LcdStringBold(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 13 || y > 5) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 14 - x; i++) {
                if (msg[i]) {
                        LcdChrBold(msg[i], 0);
                }
        }
}

void LcdStringBoldInv(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 13 || y > 5) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 14 - x; i++) {
                if (msg[i]) {
                        LcdChrBold(msg[i], 1);
                }
        }
}

void LcdStringBig(char *msg, unsigned char x, unsigned char y)  //Displays a string at current cursor location
{
        unsigned char i;

        if (x > 14 || y > 5) return;
        LcdSetTextPos(x, y);
        for (i = 0; i < 15 - x; i++) {
                if (msg[i]) {
                        LcdChrBig(msg[i], 0);
                }
        }
}

//power-down control: 0 - chip is active, 1 - chip is in PD-mode
static char power_down = 0;

char LcdIsPwrDown()
{
        return power_down;
}

void LcdPwrOff(void)
{
        if (!power_down) {
                power_down = 1;
                //power_of_display = 0;
                _pin_off(LCD_PWR);
                //power_of_display_ddr=0;

                //LCD_DC_PORT.LCD_DC_PIN=0;
                _pin_off(LCD_DC);

                //LCD_CPORT.LCD_CPIN=0;

                //LCD_RST_PORT.LCD_RST_PIN=0;
                //LCD_RST_DDR.LCD_RST_PIN=0;
                _pin_off(LCD_RST);

                //LCD_MOSI_PORT.LCD_MOSI_PIN=0;
                //LCD_MOSI_DDR.LCD_MOSI_PIN=0;
                _pin_off(LCD_MOSI);

                //LCD_CLK_PORT.LCD_CLK_PIN=0;
                //LCD_CLK_DDR.LCD_CLK_PIN=0;
                _pin_off(LCD_CLK);
        }
}

void LcdPwrOn(void)
{
        if (power_down) {
                //power_of_display = 1;
                //power_of_display_ddr = 1;
                _dir_out(LCD_PWR);
                _pin_on(LCD_PWR);
                power_down = 0;
                LcdInit();
                LcdUpdate();
        }

}

void LcdPwrMode(void)                   //инвертирует состояние вкл/выкл дисплея
{
        power_down = (~power_down) & 0x01;
#ifdef LCD_NOKIA3310_CHINA
        LcdSend( 0x20|x_mirror<<4|y_mirror<<3|power_down<<2|addressing<<1, LCD_CMD );  //LCD Standard Commands
#else
//LcdSend( 0x20|0<<2|addressing<<1, LCD_CMD );
        LcdSend(0x20 | power_down << 2 | addressing << 1, LCD_CMD);  //LCD Standard Commands
//LcdSend( 0x20|power_down<<3, LCD_CMD );
#endif
}

//#pragma used-
