#include <avr/io.h>
#include <util/delay.h>


int main(void) {
    DDRB |= 0b00000111;
    while(1) {
        for(int i=0; i<3; i++) {
            PORTB = (1<<i);
            _delay_ms(1000);
            PORTB = 0;
            _delay_ms(1000);
        }
    }
    return(0);
}
