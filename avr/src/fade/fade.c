/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Simple AVR demonstration.  Controls a LED that can be directly
 * connected from OC1/OC1A to GND.  The brightness of the LED is
 * controlled with the PWM.  After each period of the PWM, the PWM
 * value is either incremented or decremented, that's all.
 *
 * $Id: group__demo__project.html,v 1.1.1.25 2022/01/29 09:21:54 joerg_wunsch Exp $
 */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define TIMER1_TOP 255 // 8-bit PWM

enum { UP, DOWN };

ISR(TIMER1_OVF_vect) {       /* Note [2] */ 
    static uint16_t pwm;    /* Note [3] */
    static uint8_t direction;
    switch(direction)      /* Note [4] */
    {
        case UP:
            if (++pwm == TIMER1_TOP)
                direction = DOWN;
            break;
        case DOWN:
            if (--pwm == 0)
                direction = UP;
            break;
    }
    OCR1A = pwm;          /* Note [5] */
}


void ioinit(void) {          /* Note [6] */ 
    /* Timer 1 is 10-bit PWM (8-bit PWM on some ATtinys). */
    TCCR1 = _BV(PWM1A) | _BV(COM1A1);
    /*
     * Start timer 1.
     *
     * NB: TCCR1A and TCCR1B could actually be the same register, so
     * take care to not clobber it.
     */
    TCCR1 |= _BV(CS12);
    OCR1A = 0; // Initial PWM is 0
    DDRB = _BV(PB1); // Output LED is on PB1
    TIMSK |= _BV(TOIE1); // Timer Overflow Interrupt Enable
    sei();
}


int main(void) {
    ioinit();
    /* loop forever, the interrupts are doing the rest */
    for (;;)            /* Note [7] */
        sleep_mode();
    return(0);
}
