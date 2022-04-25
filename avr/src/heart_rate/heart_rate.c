#include <avr/io.h>
#include <util/delay.h>


static inline void initADC(void) {
    ADCSRA |= (1 << ADEN);
}


int main(void) {
    uint16_t adcValue;
    ADCSRA |= (1 << ADSC);
    loop_until_bit_is_clean(ADCSRA, ADSC);
    adcValue = ADC;
    
}
