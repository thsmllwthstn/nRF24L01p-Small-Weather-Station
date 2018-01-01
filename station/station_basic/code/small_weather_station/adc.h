#include <avr/io.h>

#define ADC_PIN       PINC0

class adc {

	public:
		/* Constructor */
		adc(void);

		/* Initialize the ADC */
		void initADC(void);

		/* Read analog value 
		 * @adc_value: array for 2-byte analog value
		* @return: check if read was succesfull (1 for succes, 0 for failure) */
		uint8_t analogRead(uint8_t *adc_value);
};