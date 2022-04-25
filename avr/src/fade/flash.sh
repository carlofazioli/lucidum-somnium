avr-gcc \
    -Os \
    -g \
    -std=gnu99 \
    -Wall \
    -mmcu=attiny45 \
    -c \
    fade.c
avr-gcc -mmcu=attiny45 fade.o -o fade.elf
avr-objcopy -j .text -j .data -O ihex fade.elf fade.hex

avrdude -c usbtiny -p attiny45 -U flash:w:fade.hex

rm *.o *.elf
