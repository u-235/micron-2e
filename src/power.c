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

#include "config.h"
#include "alarm.h"
#include "compiler.h"
#include "display/n3310lcd.h"
#include "screens.h"

/**
 * Run sequence or measure and calculate raw value to hundredth of volts.
 */
static void ReadADC();

static unsigned char btrPercent = 0;
static unsigned int btrVoltage = 0;
static unsigned int sleepTime;  // Через сколько секунд засыпать

static eeprom unsigned int eeSleepTime = 30;  // Через сколько секунд засыпать

extern void InitPower()
{
        sleepTime = _eemem_read16(&eeSleepTime);
}

extern unsigned int GetSleepTime()
{
        return sleepTime;
}

extern void SetSleepTime(unsigned int tm)
{
        if (tm > 600) {
                tm = 0;
        }
        sleepTime = tm;
        _eemem_write16(&eeSleepTime, sleepTime);
}

extern void IncSleepTime()
{
        unsigned int st = sleepTime;
        if (st >= 60) {
                st += 60;
        } else {
                st += 10;
        }
        SetSleepTime(st);
}

extern unsigned int GetVoltage()
{
        return btrVoltage;
}

extern unsigned char GetCharge()
{
        return btrPercent;
}

extern char CheckPower()
{
        ReadADC();
        /* Adjust */
        if (btrVoltage > POWER_VOLTAGE_HIGH) {
                btrVoltage = POWER_VOLTAGE_HIGH;
        }
        if (btrVoltage < POWER_VOLTAGE_LOW) {
                btrVoltage = POWER_VOLTAGE_LOW;
        }

        btrPercent = (btrVoltage - POWER_VOLTAGE_LOW) * 100
                        / (POWER_VOLTAGE_HIGH - POWER_VOLTAGE_LOW);

        if (btrPercent < POWER_LEVEL_ALARM) {
                return -1;
        }
        return 0;
}

void UpdateWorkTime(char flag)
{
        static unsigned int display_time = 0;
        if (flag == 0) {
                display_time = 0;
                return;
        }

        if (sleepTime == 0) {
                return;
        }

        if (display_time < 1000) {
                display_time++;
        }

        if ((display_time > sleepTime) && !IsAlarm()) {
                LcdPwrOff();
        }
}

extern void poweroff()
{
        AsyncBeep(0);
        _interrupt_disable(INT_TIMER0_OVF);
        //beep_pin = 0;
        _pin_off(OUT_BEEPER);
        SetMenuActive(0);
        led_refresh(2);
        LcdPwrOff();

//#pragma optsize-
        WDTCR = 0x1F;
        WDTCR = 0x00;
#ifdef _OPTIMIZE_SIZE_
//#pragma optsize+
#endif

        _sei();
        GICR = 0x40;
        MCUCR = 0xA0;

        DDRB = 0x00;
        DDRC = 0x00;
        DDRD = 0x00;

        PORTB = 0x00;
        PORTC = 0x00;
        PORTD = 0x04;

        TCCR0 = 0x00;
//TCNT0=0xFF;

        ASSR = 0x00;
        TCCR2 = 0x00;
        TCNT2 = 0x00;
        OCR2 = 0x00;

        _interrupt_disable(INT_TIMER0_OVF);
        _interrupt_disable(INT_TIMER2_OVF);

//PORTD.2=1; // �������� �� ����������
//DDRD.2=0;

        _sei();

//#pragma optsize-
        WDTCR = 0x1F;
        WDTCR = 0x0F;
#ifdef _OPTIMIZE_SIZE_
//#pragma optsize+
#endif

        /*
         while (1) {
         _sleep();
         }
         */
}

_isr_adc(void)
{
#ifdef POWER_LION
        /* TODO delete magic */
        btrVoltage = (118 * 255) / (ADCH - 1);  // 1.16 коэфф (для мег8 колебрется от 1.01 до 1.2 можно уточнить вольтметром и формулой ADC_VALUE = V_BG * 255/Vcc)
//как посчитать я так и не понял, но методом научного тыка определил, что 1 = 0.03 В :)
//при 116 показывало 3.74, а было - 3.82, при 118 показывает 3.80, что довольно-таки точно.
#else
                        btrVoltage=((unsigned int)ADCH-1)*2;  // 1023 - 5 вольт
#endif
}

static void ReadADC()
{
        unsigned int tmpv = 0;
        unsigned char ixi = 64, MCUCR_def;

        _sei();
        // Разрешить Все прерывания

#ifdef POWER_LION
        //Включение АЦП
        // можна мерять
        // мерять непрерывно
        // разрешить прерывание
        // предделитель на 64 (частота АЦП 125kHz при шине 8mHz)
        ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1);
        // опора AVCC
        // результат только в старший байт, ADCL не нужен
        // вход ADC14, т.е. вн. ИОН
        ADMUX |= (1 << REFS0) | (1 << ADLAR) | (1 << MUX3) | (1 << MUX2)
                        | (1 << MUX1);
#else
        //ADCSRA = 0b10011111;
        //ADMUX = 0b11100111;
        ADCSRA |= (1 << ADEN) | (1 << ADIF) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
        ADMUX |= (1 << REFS1) | (1 << REFS0) | (1 << ADLAR) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);
#endif

        MCUCR_def = MCUCR;
        MCUCR = 0x90;  // режим низких шумов АЦП
        delay_us(125);
        _interrupt_disable(INT_TIMER0_OVF);
        _interrupt_disable(INT_TIMER2_OVF);
        while (ixi > 0) {
                _sleep();  //  Уложить спать
                _wdr();
                tmpv += btrVoltage;
                ixi--;
        }
        _interrupt_enable(INT_TIMER2_OVF);
        //beep_pin = 0;
        _pin_off(OUT_BEEPER);
        MCUCR = MCUCR_def;  // Возврат режима сна.
        ADCSRA = 0x00;
        ACSR = 0x00;
        ADMUX = 0x00;  // выкл
        btrVoltage = (tmpv >> 6);
}
