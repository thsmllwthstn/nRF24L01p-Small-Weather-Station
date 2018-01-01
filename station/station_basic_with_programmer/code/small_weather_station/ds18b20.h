#include <avr/io.h>
#include "OneWire.h"


/* Get the temperature from the DS18B20
 * @temp: the 2-bytes that represent the temperature
 * @return: status of temperature (1 for succes, 0 otherwise)  */
uint8_t getTemperature(uint8_t *temp, uint8_t pin);