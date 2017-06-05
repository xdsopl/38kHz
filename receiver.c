/*
38kHz - IR receiver with CRC8 using ATmega328p
Written in 2017 by <Ahmet Inan> <xdsopl@gmail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

void put_byte(uint8_t byte)
{
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = byte;
}

uint8_t get_byte()
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

// X^8+X^5+X^4+1
uint8_t crc8_update(uint8_t crc, uint8_t data)
{
	crc ^= data;
	for (uint8_t i = 8; i; --i) {
		if (crc&1)
			crc = (crc>>1) ^ 140;
		else
			crc >>= 1;
	}
	return crc;
}

__attribute__((noreturn))
void main()
{
	// wait 200ms for voltage to raise
	_delay_ms(25);

	// hit the turbo switch, 1MHz->8MHz
	CLKPR = (1 << CLKPCE); CLKPR = 0;

	// ~2400hz baud rate at 8MHz
	UBRR0 = 207;

	// set to 8 data bits, 1 stop bit
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

	// enable UART tx
	UCSR0B = (1<<TXEN0);

	// enable debug output pin
	DDRB |= 1 << 1;

	while (1) {
		// wait until burst
		for (int i = 0; i < 2500; ++i)
			if (PINB & (1<<0))
				i = 0;
		PORTB |= (1<<1);
		while (!(PINB & (1<<0)));
		PORTB &= ~(1<<1);

		// enable UART rx
		UCSR0B |= (1<<RXEN0);
		uint8_t crc = 0;
		uint8_t addr = get_byte();
		crc = crc8_update(crc, addr);
		uint8_t left = get_byte();
		crc = crc8_update(crc, left);
		uint8_t right = get_byte();
		crc = crc8_update(crc, right);
		crc = crc8_update(crc, get_byte());
		// disable UART rx
		UCSR0B &= ~(1<<RXEN0);
		if (!crc && addr == 42) {
			put_byte(left);
			put_byte(right);
		} else {
			put_byte('-');
		}
		put_byte(' ');
	}
}


