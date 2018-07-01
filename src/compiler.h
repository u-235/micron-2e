/**
 * \file
 * \brief Определения для совместимости компиляторов.
 * \date Создан 23.02.2018
 * \author: Nick Egorrov
 * \copyright GNU Public License 3
 * \details
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
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#define delay_ms _delay_ms
#define delay_us _delay_us

#elif defined __CODEVISIONAVR__
        /* CodeVision */
#ifndef _IO_BITS_DEFINITIONS_
#define _IO_BITS_DEFINITIONS_
#endif

#include <io.h>
#include <delay.h>

#define F_CPU _MCU_CLOCK_FREQUENCY

#elif defined __ICCAVR__
        /* IAR AVR */
#ifndef ENABLE_BIT_DEFINITIONS
#define ENABLE_BIT_DEFINITIONS
#endif
#include <ioavr.h>
#include <intrinsics.h>
#define delay_ms(ms)    __delay_cycles(F_CPU/1000UL*ms)
#define delay_us(us)    __delay_cycles(F_CPU/1000000UL*us)

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
#define _sei    __enable_interrupt
#define _cli    __disable_interrupt
#define _wdr    __watchdog_reset
#define _nop    __no_operation
#define _sleep  __sleep
#endif

        /*************************************************************
         *      Data in program space and EEPROM
         *************************************************************/

#ifdef __GNUC__
        /* GCC */
#define flash __flash

#define eeprom EEMEM
#define _eemem_read8(a) eeprom_read_byte((uint8_t*)a)
#define _eemem_read16(a) eeprom_read_word((uint16_t*)a)
#define _eemem_write8(a, v) eeprom_write_byte((uint8_t*)a, v)
#define _eemem_write16(a, v) eeprom_write_word((uint16_t*)a, v)

#elif defined __CODEVISIONAVR__
        /* CodeVision */
#define sprintf_P sprintf

#define _eemem_read8(a) (*((eeprom char*)a))
#define _eemem_read16(a) (*((eeprom int*)a))
#define _eemem_write8(a, v) {*((eeprom char*)a)=v;}
#define _eemem_write16(a, v) {*((eeprom int*)a)=v;}

#elif defined __ICCAVR__
        /* IAR AVR */
#include <pgmspace.h>
#define flash __flash
#define eeprom __eeprom

#define _eemem_read8(a) (*((eeprom char*)a))
#define _eemem_read16(a) (*((eeprom int*)a))
#define _eemem_write8(a, v) {*((eeprom char*)a)=v;}
#define _eemem_write16(a, v) {*((eeprom int*)a)=v;}

#endif

        /*************************************************************
         *      Interrupts
         *************************************************************/

#ifdef __GNUC__
        /* GCC */
#include <avr/interrupt.h>
#define MAKE_ISR(v, n) ISR(v)

#elif defined __CODEVISIONAVR__
        /* CodeVision */
#define MAKE_ISR(v, n) \
        interrupt [v]  void __vector_ ## n(void)

#define ADC_vect ADC_INT
#define INT1_vect EXT_INT1
#define TIMER0_OVF_vect TIM0_OVF
#define TIMER2_OVF_vect TIM2_OVF

#elif defined __ICCAVR__
        /* IAR AVR */
#define MAKE_ISR(v, n) _MAKE_ISR(vector = v, n)
#define _MAKE_ISR(v, n) \
        _Pragma(#v)\
        __interrupt void __vector_ ## n(void)

#endif

#define _isr_adc(nu)        MAKE_ISR(ADC_vect, adc)
#define _isr_ext1(nu)       MAKE_ISR(INT1_vect, int0)
#define _isr_timer0_ovf(nu) MAKE_ISR(TIMER0_OVF_vect, tm0ovf)
#define _isr_timer2_ovf(nu) MAKE_ISR(TIMER2_OVF_vect, tm2ovf)

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

#define _interrupt_enable(i) _set_bit(i)
#define _interrupt_disable(i) _set_bit(i)

#define INT_EXT0        GICR, INT0
#define INT_EXT1        GICR, INT1
#define INT_TIMER0_OVF  TIMSK, TOIE0
#define INT_TIMER2_OVF  TIMSK, TOIE2

#ifdef _cplusplus
}
#endif

#endif /* SRC_COMPILLER_H_ */
