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
 * \date created on: 23.02.2018
 * \author: nick
 */

#ifndef SRC_COMPILLER_H_
#define SRC_COMPILLER_H_

#ifdef _cplusplus
extern "C" {
#endif

/*************************************************************
 *      Header for MCU and utils
 *************************************************************/

#ifdef __GNUC__
        /* GCC */
#define HEADER_IO <avr/io.h>
#define HEADER_DELAY <util/delay.h>
#define delay_ms _delay_ms
#define delay_us _delay_us

#elif defined __CODEVISIONAVR__
        /* CodeVision */
#ifndef _IO_BITS_DEFINITIONS_
#define _IO_BITS_DEFINITIONS_
#endif
#define HEADER_IO <io.h>
#define HEADER_DELAY <delay.h>

#elif defined __ICCAVR__
        /* IAR AVR */
#ifndef ENABLE_BIT_DEFINITIONS
#define ENABLE_BIT_DEFINITIONS
#endif
#define HEADER_IO <ioavr.h>
#define HEADER_DELAY <delay.h>

#warning not shur about headers
#else
        /* Неизвестный компилятор */
#error unknown compiller
#endif

/*************************************************************
 *      Inline assembler
 *************************************************************/

#ifdef __GNUC__
        /* GCC */
#define _sei(nu) asm("sei")
#define _cli(nu) asm("cli")
#define _wdr(nu) asm("wdr")
#define _nop(nu) asm("nop")
#define _sleep(nu) asm("sleep")

#elif defined __CODEVISIONAVR__
        /* CodeVision */
#define _sei(nu) #asm("sei")
#define _cli(nu) #asm("cli")
#define _wdr(nu) #asm("wdr")
#define _nop(nu) #asm("nop")
#define _sleep(nu) #asm("sleep")

#elif defined __ICCAVR__
        /* IAR AVR */
#define _sei(nu) asm("sei")
#define _cli(nu) asm("cli")
#define _wdr(nu) asm("wdr")
#define _nop(nu) asm("nop")
#define _sleep(nu) asm("sleep")
#endif

/*************************************************************
 *      Data in programm space and EEPROM
 *************************************************************/

#ifdef __GNUC__
        /* GCC */
#include <avr/pgmspace.h>
#define flash const PROGMEM //__flash

#define format sprintf_P

#include <avr/eeprom.h>
#define eeprom EEMEM

#elif defined __CODEVISIONAVR__
#define PSTR(s) ((flash char *)(s))

#define format sprintf

#elif defined __ICCAVR__
        /* IAR AVR */
#define PSTR(s) ((flash char *)(s))

#define format sprintf

#endif

/*************************************************************
 *      Interrupts
 *************************************************************/

#ifdef __GNUC__
        /* GCC */
#include <avr/interrupt.h>
#define _isr_adc(nu)        ISR(ADC_vect)
#define _isr_ext0(nu)       ISR(INT0_vect)
#define _isr_ext1(nu)       ISR(INT1_vect)
#define _isr_timer0_ovf(nu) ISR(TIMER0_OVF_vect)
#define _isr_timer2_ovf(nu) ISR(TIMER2_OVF_vect)

#elif defined __CODEVISIONAVR__
        /* CodeVision */
#define _isr_adc(nu)        interrupt [ADC_INT]  void voidadc_isr(void)
#define _isr_ext0(nu)       interrupt [EXT_INT0] void ext_int0_isr(void)
#define _isr_ext1(nu)       interrupt [EXT_INT1] void ext_int1_isr(void)
#define _isr_timer0_ovf(nu) interrupt [TIM0_OVF] void timer0_ovf_isr(void)
#define _isr_timer2_ovf(nu) interrupt [TIM2_OVF] void timer2_comp_ovf(void)

#elif defined __ICCAVR__
        /* IAR AVR */

#define _isr_adc(nu)        __interrupt void voidadc_isr(void)
#pragma vector=ADC_vect
_isr_adc(void);
#define _isr_ext0(nu)       __interrupt void ext_int0_isr(void)
#pragma vector=INT0_vect
_isr_ext0(void);
#define _isr_ext1(nu)       __interrupt void ext_int1_isr(void)
#pragma vector=INT1_vect
_isr_ext1(void);
#define _isr_timer0_ovf(nu) __interrupt void timer0_ovf_isr(void)
#pragma vector=TIMER0_OVF_vect
_isr_timer0_ovf(void);
#define _isr_timer2_ovf(nu) __interrupt void timer2_comp_ovf(void)
#pragma vector=TIMER2_OVF_vect
_isr_timer2_ovf(void);

#warning not sure about interrupts
#else
        /* Неизвестный компилятор */
#error unknown compiller
#endif

        /* some useful defines */
#define _set_mask(dst, mask) dst|=(mask)
#define _clear_mask(dst, mask) dst&=~(mask)
#define _set_bit(dst, bit) _set_mask(dst, 1<<bit)
#define _clear_bit(dst, bit) _clear_mask(dst, 1<<bit)

#define __dir_out(port, pin) _set_bit(DDR##port, pin)
#define __dir_in(port, pin) _clear_bit(DDR##port, pin)
#define _dir_out(x) __dir_out(x)
#define _dir_in(x) __dir_out(x)

#define __pin_on(port, pin) _set_bit(PORT##port, pin)
#define __pin_off(port, pin) _clear_bit(PORT##port, pin)
#define _pin_on(x) __pin_on(x)
#define _pin_off(x) __pin_off(x)

#define __is_pin_clean(port, pin) ((PIN##port & (1<<pin)) == 0)
#define __is_pin_set(port, pin) ((PIN##port & (1<<pin)) != 0)
#define _is_pin_clean(x) __is_pin_clean(x)
#define _is_pin_set(x) __is_pin_set(x)

#ifdef _cplusplus
        extern "C" {
#endif

#endif /* SRC_COMPILLER_H_ */
