#define F_CPU 1000000

#include <util/delay.h>
#include <avr/io.h>
#include "OneWire.h"

uint8_t onewire_reset(uint8_t TEMPERATURE_PIN)
{		
	//Set the One Wire pin as an output
	DDRD |= 1<<TEMPERATURE_PIN;

	//Set the output low for 480us - 960us
	PORTD &= ~(1<<TEMPERATURE_PIN);
	_delay_us(720);

	//Set the One wire pin as an input and wait for a response (60us - 240us)
	DDRD &= ~(1<<TEMPERATURE_PIN);
	_delay_us(60);

	//The sensor pulls the line low
	if(PIND & (1<<TEMPERATURE_PIN)) return 0;

	//Set counter prescaler to clk/1
	TCCR0B |= 1<<CS00;

	//Start counting from 0...
	TCNT0 = 0;
	
	//Wait for the sensor to pull the line high
	while(!(PIND & 1<<TEMPERATURE_PIN))
	{
		if(TCNT0 >= 250) return 0;
	}
	return 1;
}

void onewire_write(uint8_t command, uint8_t TEMPERATURE_PIN)
{	
	uint8_t AMOUNT_OF_BITS = 8;

	for(int i = 0; i < AMOUNT_OF_BITS; i++)
	{
		//Write a logic '1'
		if((command & (1<<i)) != 0)
		{
			//Set the one wire pin as an output
			DDRD |= 1<<TEMPERATURE_PIN;

			//Set the output low for ~1us
			PORTD &= ~(1<<TEMPERATURE_PIN);
			_delay_us(1);

			//Set the one wire pin as an input to drive the line high again,
			//the sensor will read this as a '1'
			DDRD &= ~(1<<TEMPERATURE_PIN);
			_delay_us(59);
		}
		
		//Write a logic '0'
		else
		{
			//Set the one wire pin as an output
			DDRD |= 1<<TEMPERATURE_PIN;

			//Set the output low for 15us - 60us,
			//the sensor will read this as a '0'
			PORTD &= ~(1<<TEMPERATURE_PIN);
			_delay_us(59);

			//Set the one wire pin as an input
			DDRD &= ~(1<<TEMPERATURE_PIN);
			_delay_us(1);
		}
	}
}

uint8_t onewire_read(uint8_t TEMPERATURE_PIN)
{	
	uint8_t value = 0;

	uint8_t AMOUNT_OF_BITS = 8;

	for(int i = 0; i < AMOUNT_OF_BITS; i++)
	{
		//Set the one wire pin as an output
		DDRD |= 1<<TEMPERATURE_PIN;

		//Set the output low for ~1us
		PORTD &= ~(1<<TEMPERATURE_PIN);
		_delay_us(1);

		//Set the one wire pin as an input for 1us - 15us
		DDRD &= ~(1<<TEMPERATURE_PIN);
		_delay_us(9);

		//If the line is high, the bit represents a logic '1',
		//if the line is low, the bit represents a logic '0'
		if( (PIND & 1<<TEMPERATURE_PIN))
		{
			value += (1<<i);
		}
		//One timeslot takes around 60us, so we wait for the next one
		_delay_us(50);
	}
	return value;
}

uint8_t onewire_wait_for_conversion(uint8_t TEMPERATURE_PIN)
{
	uint8_t value = 0;


	DDRD |= 1<<TEMPERATURE_PIN;
	PORTD &= ~(1<<TEMPERATURE_PIN);
	_delay_us(1);

	DDRD &= ~(1<<TEMPERATURE_PIN);
	_delay_us(9);

	if( (PIND & (1<<TEMPERATURE_PIN)) != 0)
	{
		value = 1;
	}
	_delay_us(50);

	return value;
}