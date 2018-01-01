#include <avr/io.h>

#define SKIP_ROM         0xCC
#define START_CONVERSION 0x44
#define READ_SCRATCHPAD  0xBE

/* Send a reset pulse */
uint8_t onewire_reset(uint8_t TEMPERATURE_PIN);

/* Writes an 8-bit command
 * @command: 8-bits that needs to be transmitted */ 
void onewire_write(uint8_t command, uint8_t TEMPERATURE_PIN);

/* Read 8-bits from the device
 * @return: 8-bits from the device */
uint8_t onewire_read(uint8_t TEMPERATURE_PIN);

/* Wait for the conversion to be done (DS18B20 only)
 * @return: state of the conversion (0 when ongoing, 1 when done) */
uint8_t onewire_wait_for_conversion(uint8_t TEMPERATURE_PIN);