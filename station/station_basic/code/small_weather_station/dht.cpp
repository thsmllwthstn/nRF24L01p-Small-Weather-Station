#define F_CPU 1000000

#include <util/delay.h>
#include "dht.h"


dht::dht(void)
{
	//Constructor
}

void dht::initDHT(void)
{   
    _delay_ms(1000);
}

void dht::request(void)
{
	//Set the DHT11_PIN as output
	DDRD |= 1<<dht_pin;

	//Set the output low
	PORTD &= ~(1<<dht_pin);

	//Set pin high after 20ms
	_delay_ms(20);
	PORTD |= (1<<dht_pin);
}

uint8_t dht::response(void)
{
	//Set the DHT11_pin as input
	DDRD &= ~(1<<dht_pin);

	//Set counter prescaler to clk/1
	TCCR0B |= 1<<CS00;

	//Start counting from 0...
	TCNT0 = 0;

	//Wait for the sensor to drive the line low (20us - 40us)
	while(PIND & (1<<dht_pin))
	{
		if(TCNT0 >= 60) return 0;
	}

	//Wait for the sensor to drive the line high (~80us)
	TCNT0 = 0;
	while(!(PIND & (1<<dht_pin)))
	{
		if(TCNT0 >= 100) return 0;
	}

	//Wait for the sensor to drive the line low again (~80us)
	TCNT0 = 0;
	while(PIND & (1<<dht_pin))
	{
		if(TCNT0 >= 100) return 0;
	}

	return 1;
}

int dht::read_bytes(void) 
{
	uint8_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		//Data transmission always starts with a low pulse (~50us)
		TCNT0 = 0;
		while(!(PIND & (1<<dht_pin)))
		{
			if(TCNT0 >= 70) return -1;
		}
		

		/* After 45us, if the pulse is high, the bit is a '1', 
		 * otherwise the bit is a '0' */
		 _delay_us(45);

		(PIND & (1<<dht_pin)) ? data = (data << 1) | (0x01) : data = (data << 1);

		//If the pulse is a '1', wait for the next low pulse to begin (~25us)
		TCNT0 = 0;
		while(PIND & (1<<dht_pin))
		{
			if(TCNT0 >= 45) return -1;
		}
	}
	return data;
}