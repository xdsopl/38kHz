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
	// wait 200ms for voltage to raise
	_delay_ms(25);

	// hit the turbo switch, 1MHz->8MHz
	CLKPR = (1 << CLKPCE); CLKPR = 0;

	// toggle OC0A, CTC mode
	TCCR0A = (1<<COM0A0) | (1<<WGM01);

	// fsck/1
	TCCR0B = (1<<CS00);

	// ~38kHz at 8MHz
	OCR0A = 105;

	// enable carrier and TX pin as output
	DDRD |= (1<<6) | (1<<1);

	// ~2400hz baud rate at 8MHz
	UBRR0 = 207;

	// set to 8 data bits, 1 stop bit
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

	uint8_t addr = 42, left = 'A', right = 'z';
	while (1) {
		// allow AGC to settle by sending burst
		PORTD &= ~(1<<1);
		_delay_ms(9);

		// give receiver some time to switch
		PORTD |= (1<<1);
		_delay_ms(1);

		// enable UART tx
		UCSR0B = (1<<TXEN0);
		uint8_t crc = 0;
		put_byte(addr);
		crc = crc8_update(crc, addr);
		put_byte(left);
		crc = crc8_update(crc, left);
		put_byte(right);
		crc = crc8_update(crc, right);
		put_byte(crc);
		left = left < 'Z' ? left+1 : 'A';
		right = 'a' < right ? right-1 : 'z';
		// disable UART tx when done
		UCSR0B = 0;
		for (uint8_t i = 5+(15&rand()); i; --i)
			_delay_ms(10.0);
	}
}


