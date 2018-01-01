#include <avr/io.h>
#include "ds18b20.h"

uint8_t getTemperature(uint8_t *temp, uint8_t pin)
{
	//Start the conversation with a reset puls
	if(!onewire_reset(pin) )
	{
		return 0;
	}

	//Skip the ROM-command
	onewire_write(SKIP_ROM, pin);
	
	//Start analog to digital conversion
	onewire_write(START_CONVERSION, pin);

	/* Wait for the conversion to be done  */

	/*           MAX Waiting time         *
	*  ---------------------------------  *
	*    9-bit conversion  -   93.75ms    *
	*    10-bit conversion -   187.5ms    *
	*    11-bit conversion -   375.0ms    *
	*    12-bit conversion -   750.0ms    */
	while(!(onewire_wait_for_conversion(pin) ) )
	{
		//Wait for conversion
	}

	//Start conversation again with reset puls
	onewire_reset(pin);
		
	//Skip the ROM-command
	onewire_write(SKIP_ROM, pin);

	//Start reading the entire scratchpath which contains of 9 bytes
	onewire_write(READ_SCRATCHPAD, pin);

	//Read the first 2 bytes and store them in data
	for(uint8_t i = 0; i < 2; i++)
	{
		temp[i] = onewire_read(pin);
	}

	//Reset the DS18B20 since it sends 9 bytes of data,
	//and we only need the first 2 bytes
	onewire_reset(pin);

	return 1;
}