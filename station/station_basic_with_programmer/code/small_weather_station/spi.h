#include <avr/io.h>

#define SS	 PINB2
#define MOSI PINB3
#define MISO PINB4
#define SCK  PINB5
#define CE   PINB6


class spi {

	public: 
		/* Initialize SPI-protocol */
		void spi_init(void);

		/* Transmit bytes trough SPI
		 * @*data: bytes that needs to be send
		 * @count: size of data thats needs to be send  */
		void spi_transmit(char *data, int count);

		/* Used in previous function
		 * Loads data in SPI buffer
		 * @data: byte that needs to be send
		 * @return: (optional) byte from slave  */
		char spi_send_char(char data); 
};
