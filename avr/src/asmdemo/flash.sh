avr-gcc \
    -Os \
    -g \
    -std=gnu99 \
    -Wall \
    -mmcu=attiny45 \
    -c \
    asmdemo.c
avr-gcc -mmcu=attiny45 asmdemo.o -o asmdemo.elf
avr-objcopy -j .text -j .data -O ihex asmdemo.elf asmdemo.hex

avrdude -c usbtiny -p attiny45 -U flash:w:asmdemo.hex

rm *.o *.elf *.hex
