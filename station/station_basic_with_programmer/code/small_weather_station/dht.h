#include <avr/io.h>

//#define DHT11_PIN     PIND2
#define dht_pin     2

class dht {

	public:
		/* Constructor */
		dht(void);

		/* Initialize the humidity sensor */
		void initDHT(void);

		/* Send start signal */
		void request(void);

		/* Listen for response after the start signal
		 * @return: state of response (1 for succes, 0 for failure) */
		uint8_t response(void);

		/* If a response happend, read the incomming data 
		 * @return: data byte (2 humidity bytes and 2 temperature bytes) */
		int read_bytes(void);
};