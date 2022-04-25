avr-gcc \
    -Os \
    -g \
    -std=gnu99 \
    -Wall \
    -mmcu=attiny45 \
    -c \
    heart_rate.c
avr-gcc -mmcu=attiny45 heart_rate.o -o heart_rate.elf
avr-objcopy -j .text -j .data -O ihex heart_rate.elf heart_rate.hex

avrdude -c usbtiny -p attiny45 -U flash:w:heart_rate.hex

rm *.o *.elf *.hex
