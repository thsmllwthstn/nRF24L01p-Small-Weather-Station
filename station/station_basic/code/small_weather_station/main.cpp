/*
 * Code for Small Weather Station
 */

#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "nrf24l01p.h"
#include "dht.h"
#include "adc.h"
#include "ds18b20.h"

#define LED      PIND3
#define BUTTON   PIND5
#define RX_MODE  0x00
#define TX_MODE  0x01

#define TEMPERATURE_PIN     PIND4
#define EXT_TEMPERATURE_PIN PIND0

//Prototypes
void send_tx_data(void);

//Make objects from classes
nRF24L01P my_nrf;
adc       my_adc;
dht       my_humidity;

//Variables 
uint8_t TRANSFER_SIZE;
uint8_t TX_DATA[10];
uint8_t RX_DATA[10];
uint16_t one_second_counter = 0;
uint16_t *one_second_counter_ptr;
uint16_t send_tx_timer = 5;
bool debounce = false;
bool rx_error = false;
bool send_response = false;
int mode = TX_MODE;

int main(void)
{
	/******* Check if the external temperature sensor is connected or not *******/
	uint8_t dummy_array[2];
	(getTemperature(dummy_array, EXT_TEMPERATURE_PIN) ) ? TRANSFER_SIZE = 10 : TRANSFER_SIZE = 8;	
	
	my_humidity.initDHT();
	my_adc.initADC();
	
	/******* nRF24L01+ Settings *******/
	my_nrf.setOutputPower(OUTPUT_POWER_MINUS_18DBM);
	my_nrf.setAirDataRate(AIR_DATA_RATE_250KBPS);
	my_nrf.setCyclicRedundancyCheck(CYCLIC_REDUNDANCY_CHECK_1BYTE);
	my_nrf.disableAutoAcknowledgment();
	my_nrf.disableAutoRetransmit();
	my_nrf.enableDataPipe(DATA_PIPE_ALL);
	my_nrf.setTransferSize(DATA_PIPE_0, TRANSFER_SIZE);
	my_nrf.setTransferSize(DATA_PIPE_1, TRANSFER_SIZE);
	my_nrf.setTransferSize(DATA_PIPE_2, TRANSFER_SIZE);
	my_nrf.setTransferSize(DATA_PIPE_3, TRANSFER_SIZE);
	my_nrf.setTransferSize(DATA_PIPE_4, TRANSFER_SIZE);
	my_nrf.setTransferSize(DATA_PIPE_5, TRANSFER_SIZE);

	my_nrf.setReceiveMode();

	/* Enable Clear Timer on Compare mode */
	TCCR1B |= (1 << WGM12);

	/* Enable timer interrupt */
	TIMSK1 |= (1 << OCIE1A);

	/* One Second Calculation
	 * time[s] = frequency / (prescaler * compare) 
	 * 1000000 / (64 * (15625 - 1)) = 1[s]   */
	TCCR1B |= (1 << CS11) | (1 << CS10);
	OCR1A = 0x3D08;

	/* Enable interrupts */
	sei();

	//Set the LED as output
	DDRD |= 1<<LED;

	//Set the button as input
	DDRD &= ~(1<<BUTTON);	

	//Toggle the led to let the user know the PCB is ready to use...
	for(int i = 0; i < 4; i++)
	{
		PORTD ^= 1<<LED;
		_delay_ms(250);
	}

	send_tx_data();

    while(1)
    {
		one_second_counter_ptr = &one_second_counter;
		if(*one_second_counter_ptr >= send_tx_timer && mode == TX_MODE)
		{
			send_tx_data();
			one_second_counter = 0;
		}

		/* If button is pressed, toggle between RX en TX mode */
		if((PIND & 1<<BUTTON) && !debounce)
		{
			debounce = true;
			one_second_counter = 0;
			_delay_ms(5);
		}
		else if(!(PIND & 1<<BUTTON) && debounce && mode == TX_MODE)
		{
			my_nrf.setReceiveMode();
			PORTD |= 1<<LED;
			mode = RX_MODE;
			debounce = false;
			_delay_ms(5);
		}
		else if(!(PIND & 1<<BUTTON) && debounce && mode == RX_MODE)
		{
			my_nrf.setTransmitMode();
			PORTD &= ~(1<<LED);
			mode = TX_MODE;
			debounce = false;
			_delay_ms(5);
		}

		/* When the button is held down for 4 seconds, reset to standard configuration */
		else if( (PIND & 1<<BUTTON) && debounce)
		{
			if(*one_second_counter_ptr >= 4)
			{
				my_nrf.setFrequencyChannel(MINIMUM_FREQUENCY_CHANNEL + 2);
				my_nrf.setRXAddress(DATA_PIPE_0, 0xE7E7E7E7E7);
				my_nrf.setRXAddress(DATA_PIPE_1, 0xC2C2C2C2C2);
				my_nrf.setRXAddress(DATA_PIPE_2, 0xC3);
				my_nrf.setRXAddress(DATA_PIPE_3, 0xC4);
				my_nrf.setRXAddress(DATA_PIPE_4, 0xC5);
				my_nrf.setRXAddress(DATA_PIPE_5, 0xC6);
				my_nrf.setTXAddress(0xE7E7E7E7E7);
				send_tx_timer = 5;

				for(int i = 0; i < 8; i++)
				{
					PORTD ^= 1<<LED;
					_delay_ms(250);
				}
				debounce = false;
				_delay_ms(5);
			}
		}

		/* When in RX mode, listen for incomming data */
		if(my_nrf.readable() && mode == RX_MODE)
		{
			int rx_length = my_nrf.readData(RX_DATA, sizeof(RX_DATA));

			//Check if the user wants to set the timer
			if(RX_DATA[0] == 't' && RX_DATA[1] == '=')
			{
				//Read the data until 's'
				char new_time[TRANSFER_SIZE];
				for(int i = 0; i < rx_length; i++)
				{
					char c = RX_DATA[i + 2];

					if(c == 's')
					{
						if(i == 1) send_tx_timer = RX_DATA[i + 1] - '0';
						else send_tx_timer = atoi(new_time);
						send_response = true;
						break;
					}
					else
					{
						//Check if the received data is valid...
						if (c >= '0' && c <= '9') new_time[i] = RX_DATA[i + 2];
						else
						{
							rx_error = true;
							break;
						}
					}
				}
				if(send_tx_timer < 2 || send_tx_timer > 65535) rx_error = true;
			}

			//Check if the user wants to change the TX-address
			else if(RX_DATA[0] == 't' && RX_DATA[1] == 'a' && RX_DATA[2] == '=')
			{
				uint8_t tx_address[5];

				//LSByte first
				for(int i = 0; i < 5; i++)
				{
					tx_address[4 - i] = RX_DATA[i + 3];
				}
				my_nrf.setTXAddress(tx_address);
			}
      
			//Check if the user wants to change the RX-address
			else if(RX_DATA[0] == 'r' && RX_DATA[2] == '=')
			{
				uint8_t pipe = RX_DATA[1] - '0';

				if(pipe == 0 || pipe == 1)
				{
					uint8_t rx_address[5];

					//LSByte first
					for(int i = 0; i < 5; i++)
					{
						rx_address[4 - i] = RX_DATA[i + 3];
					}
					my_nrf.setRXAddress(pipe, rx_address);
					send_response = true;
				}

				  else if(pipe >= 2 && pipe <= 5)
				  {
					  my_nrf.setRXAddress(pipe, RX_DATA[3]);
					  send_response = true;
				  }

				else
				{
					rx_error = true;
				}
			}
			      
		    //Check if the user wants to change the frequency
			else if(RX_DATA[0] == 'f' && RX_DATA[1] == 'r' && RX_DATA[2] == 'e' && RX_DATA[3] == '=' )
			{
				char new_frequency[4];

				for(uint8_t i = 0; i < sizeof(new_frequency); i++)
				{
					char c = RX_DATA[i + 4];

					if(c >= '0' && c <= '9') new_frequency[i] = c;
					else
					{
						rx_error = true;
						break;
					}
				}

				uint16_t f = atoi(new_frequency);

				if(f >= MINIMUM_FREQUENCY_CHANNEL && f <= MAXIMUM_FREQUENCY_CHANNEL)
				{
					my_nrf.setFrequencyChannel(f);
				}
				else rx_error = true;
			}
					
			else
			{
				rx_error = true;
			}
			my_nrf.setTransmitMode();
	    
			/* Let the user know an error occured, stay in receive mode after */
			if(rx_error)
			{
				uint8_t ERROR_DATA[10] = {'-', 'e', 'r', 'r', 'o', 'r', '!', '-', '-', '-'};
				my_nrf.writeData(ERROR_DATA, TRANSFER_SIZE);
				mode = RX_MODE;
				rx_error = false;
			}
	    
			/* If no error occured, let the user know by echoing the data back
			 * Also, send the data and go in sleep mode after */
			else
			{
				if(send_response)
				{
					my_nrf.writeData(RX_DATA, TRANSFER_SIZE);
					send_tx_data();
					send_response = false;
				}
				one_second_counter = 0;
				mode = TX_MODE;
				PORTD &= ~(1<<LED);
			}
		}
    }		
}

void send_tx_data(void)
{
	//Variables
	uint8_t data_counter = 0, new_data_counter = 0;

	/**************************** Temperature Sensor ****************************/
	//Read the temperature and store the bytes
	uint8_t temperature_data[2];
	if(getTemperature(temperature_data, TEMPERATURE_PIN) )
	{
		//Add the bytes to the array we're going to send
		for(uint8_t i = 0; i < sizeof(temperature_data); i++)
		{
			TX_DATA[i + data_counter] = temperature_data[i];
			new_data_counter++;
		}
	}
	data_counter = new_data_counter;

	/***************************** Humidity Sensor ******************************/	
	//Read the DHT11 data and store the bytes
	uint8_t DHT_data[4];
		
	my_humidity.request();
	
	if(my_humidity.response() )
	{	
		for(int i = 0; i < 4; i++)
		{
			uint8_t dht_byte = my_humidity.read_bytes();
		
			if(dht_byte != -1)
			{
				DHT_data[i] = dht_byte;
			}
		}

		uint8_t DHT_checksum  = my_humidity.read_bytes();
	
		if ((DHT_data[0] + DHT_data[1] + DHT_data[2] + DHT_data[3]) == DHT_checksum)
		{
			//Add the bytes to the array we're going to send
			for(uint8_t i = 0; i < sizeof(DHT_data); i++)
			{
				TX_DATA[i + data_counter] = DHT_data[i];
				new_data_counter++;
			}
		}	
	}
	data_counter = new_data_counter;

	/******************************* Analog Value *******************************/
	//Read the analog value and store the bytes
	uint8_t analogValue[2];

	if(my_adc.analogRead(analogValue) )
	{
		//Add the bytes to the array we're going to send
		for(int i = 0; i < 2; i++)
		{
			TX_DATA[i + data_counter] = analogValue[i];
			new_data_counter++;
		}
	}
	data_counter = new_data_counter;

	/********************** External Temperature Sensor *************************/
	if(TRANSFER_SIZE == 10)
	{
		//Read the temperature and store the bytes
		uint8_t ext_temperature_data[2];
		if(getTemperature(ext_temperature_data, EXT_TEMPERATURE_PIN) )
		{
			//Add the bytes to the array we're going to send
			for(uint8_t i = 0; i < sizeof(ext_temperature_data); i++)
			{
				TX_DATA[i + data_counter] = ext_temperature_data[i];
				new_data_counter++;
			}
		}
		data_counter = new_data_counter;
	}
	
	//Send to array to the nRF24L01+
	my_nrf.writeData(TX_DATA, TRANSFER_SIZE);

	//Put the nRF24L01+ in sleep mode
	my_nrf.powerDown();
}

ISR (TIMER1_COMPA_vect)
{
	one_second_counter++;
}
