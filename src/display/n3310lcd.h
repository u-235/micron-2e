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

#ifndef N3310LCD_H_INCLUDED
#define N3310LCD_H_INCLUDED


void LcdUpdate(void);   	//Copies the LCD cache into the device RAM
void LcdClear(void);    				//Clears the display
void LcdClear_massive(void);    //Clears the display
void LcdInit(void);				//Настройка SPI и дисплея
void LcdContrast(unsigned char contrast);  //contrast -> Contrast value from 0x00 to 0x7F
void LcdMode(unsigned char mode);  //режимы дисплея: 0 - blank, 1 - all on, 2 - normal, 3 - inverse
void LcdPwrMode(void);			//инвертирует состояние вкл/выкл дисплея
void LcdPwrOff(void);						//выкл дисплея
void LcdPwrOn(void);						//вкл дисплея
char LcdIsPwrDown();//power-down control: 0 - chip is active, 1 - chip is in PD-mode
void LcdImage (const unsigned char *imageData);  //вывод изображения
void LcdPixel(unsigned char x, unsigned char y, unsigned char mode);  //Displays a pixel at given absolute (x, y) location, mode -> Off, On or Xor
void LcdLine(int x1, int y1, int x2, int y2, unsigned char mode);  //Draws a line between two points on the display
void LcdCircle(char x, char y, char radius, unsigned char mode);  //рисуем круг с координатами центра и радиусом
void LcdSetTextPos(unsigned char x, unsigned char y);  //Sets cursor location to xy location. Range: 1,1 .. 14,6
void LcdChr(char ch, char inv);//Displays a character at current cursor location and increment cursor location
void LcdChrBold(char ch, char inv);//Печатает символ на текущем месте, большой и жирный)
void LcdChrBig(char ch, char inv);		//Печатает символ на текущем месте, большой

void LcdString(char *msg, unsigned char x, unsigned char y);//Displays a string at current cursor location
void LcdStringInv(char *msg, unsigned char x, unsigned char y);//Displays a string at current cursor location
void LcdStringBold(char *msg, unsigned char x, unsigned char y);//Печатает большую и жирную строку
void LcdStringBig(char *msg, unsigned char x, unsigned char y);//Печатает большую строку

//void LcdBar(int x1, int y1, int x2, int y2, unsigned char persent);           // рисует прогресс-бар и заполняет его на "процент"
//void LcdBarLine(unsigned char line, unsigned char persent);                   // рисуем прошресс-бар в указанной строке

#endif // N3310LCD_H_INCLUDED
