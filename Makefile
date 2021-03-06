
CFLAGS = -DDEBUG=1 -std=c99 -W -Wall -Os -ffreestanding -fwhole-program -mmcu=atmega328p -DF_CPU=8000000UL
AFLAGS = -c avrispmkII -P usb -p m328p

.PHONY: all
all: sender.hex receiver.hex

.PHONY: sender
sender: sender.flash

.PHONY: receiver
receiver: receiver.flash

%.hex: %.bin
	avr-objcopy -O ihex -I binary -R .eeprom $< $@

%.bin: %.elf
	avr-objcopy -O binary -R .eeprom $< $@
	du -b $@

%.elf: %.c Makefile
	avr-gcc $(CFLAGS) $< -o $@

%.s: %.c
	avr-gcc $(CFLAGS) -S $< -o $@

.PHONY: %.flash
%.flash: %.hex
	avrdude $(AFLAGS) -U flash:w:$<

.PHONY: erase
erase:
	avrdude $(AFLAGS) -e

.PHONY: clean
clean:
	rm -f *.elf *.hex *.s

