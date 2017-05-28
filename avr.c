/*
38kHz - dummy IR sender with CRC8 using ATmega328p
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
	// toggle OC0A, CTC mode
	TCCR0A = (1<<COM0A0) | (1<<WGM01);

	// fsck/1
	TCCR0B = (1<<CS00);

	// ~38kHz
	OCR0A = 209;

	// enable output
	DDRD |= 1 << 6;

	// 2khz baud rate at 16MHz
	UBRR0H = 1;
	UBRR0L = 243;

	// set to 8 data bits, 1 stop bit
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

	uint8_t addr = 42, left = 1, right = 128;
	while (1) {
		// enable UART tx
		UCSR0B = (1<<TXEN0);
		_delay_ms(5);
		uint8_t crc = 0;
		put_byte(addr);
		crc = crc8_update(crc, addr);
		put_byte(left);
		crc = crc8_update(crc, left);
		put_byte(right);
		crc = crc8_update(crc, right);
		put_byte(crc);
		left = (left<<1) | (left>>7);
		right = (right>>1) | (right<<7);
		// disable UART tx when done
		UCSR0B = 0;
		for (uint8_t i = 5+(15&rand()); i; --i)
			_delay_ms(10.0);
	}
}


