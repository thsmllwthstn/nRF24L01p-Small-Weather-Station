#define F_CPU 1000000

#include <util/delay.h>
#include "adc.h"

adc::adc(void)
{
	//Contructor
}

void adc::initADC(void)
{
	/* Set voltage reference selection to AVcc */
	ADMUX |= 1<<REFS0;

	/* Set the pin as Single Ended Input  */
	ADMUX |= ADC_PIN;

	/* Use analog channel 0 (PINC0) */
	//ADMUX &= ~(1<<MUX0 | 1<<MUX1 | 1<<MUX2);

	/* Define prescaler:
	 * PRESCALER = CLK_FREQUENCY / INPUT_CLK_FREQUENCY_ADC
	 * 1MHz / (200kHz | 50kHz) = 5 - 20
	 * So we set the prescaler to 8 */
	ADCSRA |= (1<<ADPS0 | 1<<ADPS1);

	/* Enable the ADC */
	ADCSRA |= 1<<ADEN;
}

uint8_t adc::analogRead(uint8_t *adc_value)
{
	//Start analog to digital conversion
	ADCSRA |= 1<<ADSC;

	//Set counter prescaler to clk/1
	TCCR0B |= 1<<CS00;
	TCCR0B &= ~(1<<CS01 | 1<<CS02);

	//Start counting from 0...
	TCNT0 = 0;

	//Wait for the conversion to be done (13~25 clock cycles)
	while(ADCSRA & (1<<ADSC))
	{
		//if(TCNT0 >= 45) return 0;
	}

	//Read the 8-bit registers
	adc_value[0] = ADCL;
	adc_value[1] = ADCH;

	return 1;
}