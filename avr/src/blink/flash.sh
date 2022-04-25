avr-gcc \
    -Os \
    -g \
    -std=gnu99 \
    -Wall \
    -mmcu=attiny45 \
    -c \
    blink.c
avr-gcc -mmcu=attiny45 blink.o -o blink.elf
avr-objcopy -j .text -j .data -O ihex blink.elf blink.hex

avrdude -c usbtiny -p attiny45 -U flash:w:blink.hex

rm *.o *.elf *.hex
