#include "spi.h"

void spi::spi_init(void)
{
	/* Set MOSI, SCK, SlaveSelect and CE as Output */
	DDRB |= (1<<MOSI | 1<<SCK | 1<<SS | 1<<CE);
	
	/* Enable SPI, Set as Master */
	SPCR |= (1<<SPE | 1<<MSTR);

	/* Set Slave Select high */
	PORTB |= 1<<SS;
}

void spi::spi_transmit(char *data, int count)
{
	/* Slave Select low */
	PORTB &= ~(1<<SS);

	for(int i = 0; i < count; i++)
	{
		spi_send_char(data[i]);
	}

	/* Slave Select high */
	PORTB |= 1<<SS;
}

char spi::spi_send_char(char data)
{
	/* Load data into the buffer */
	SPDR = data;
	
	/* Wait until transmission complete */
	while(!(SPSR & (1<<SPIF) ));

	return(SPDR);
}