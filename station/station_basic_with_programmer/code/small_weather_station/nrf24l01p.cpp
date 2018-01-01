#define F_CPU 1000000

#include <util/delay.h>
#include "nrf24l01p.h"

#define READ_REGISTER_CMD                0x00
#define WRITE_REGISTER_CMD               0x20
#define READ_RX_PAYLOAD_WIDTH_CMD        0x60
#define READ_RX_PAYLOAD_CMD              0x61
#define WRITE_TX_PAYLOAD_CMD             0xA0
#define FLUSH_TX_FIFO_CMD                0xE1
#define FLUSH_RX_FIFO_CMD                0xE2
#define NOP_CMD                          0xFF

#define REG_ADDRESS_CONFIG               0x00
#define REG_ADDRESS_EN_AA                0x01
#define REG_ADDRESS_EN_RXADDR            0x02
#define REG_ADDRESS_SETUP_AW             0x03
#define REG_ADDRESS_SETUP_RETR           0x04
#define REG_ADDRESS_RF_CH                0x05
#define REG_ADDRESS_RF_SETUP             0x06
#define REG_ADDRESS_STATUS               0x07
#define REG_ADDRESS_RXADDR_PIPE_0        0x0A
#define REG_ADDRESS_RXADDR_PIPE_1        0x0B
#define REG_ADDRESS_RXADDR_PIPE_2        0x0C
#define REG_ADDRESS_RXADDR_PIPE_3        0x0D
#define REG_ADDRESS_RXADDR_PIPE_4        0x0E
#define REG_ADDRESS_RXADDR_PIPE_5        0x0F
#define REG_ADDRESS_TXADDR               0x10
#define REG_ADDRESS_RX_PW_P0             0x11
#define REG_ADDRESS_RX_PW_P1             0x12
#define REG_ADDRESS_RX_PW_P2             0x13
#define REG_ADDRESS_RX_PW_P3             0x14
#define REG_ADDRESS_RX_PW_P4             0x15
#define REG_ADDRESS_RX_PW_P5             0x16
#define REG_ADDRESS_FIFO_STATUS          0x17
#define REG_ADDRESS_DYNPD                0x1C
#define REG_ADDRESS_FEATURE              0x1D

#define _MASK_OUTPUT_POWER               0x06
#define _OUTPUT_POWER_0DBM               0x06
#define _OUTPUT_POWER_MINUS_6DBM         0x04
#define _OUTPUT_POWER_MINUS_12DBM        0x02
#define _OUTPUT_POWER_MINUS_18DBM        0x00

#define _MASK_AIR_DATA_RATE              0x28
#define _AIR_DATA_RATE_250KBPS           0x20
#define _AIR_DATA_RATE_1MBPS             0x00
#define _AIR_DATA_RATE_2MBPS             0x08

#define _MASK_CYCLIC_REDUNDANCY_CHECK    0x0C
#define _CYCLIC_REDUNDANCY_CHECK_OFF     0x00
#define _CYCLIC_REDUNDANCY_CHECK_1BYTE   0x08
#define _CYCLIC_REDUNDANCY_CHECK_2BYTES  0x0C

#define _DISABLE_AUTO_ACKNOWLEDGE_ALL    0x00
#define _DISABLE_AUTO_RETRANSMIT_ALL     0x00

#define _ENABLE_DATA_PIPE_ALL            0x3F
#define _ENABLE_DATA_PIPE_0              0x01
#define _ENABLE_DATA_PIPE_1              0x02
#define _ENABLE_DATA_PIPE_2              0x04
#define _ENABLE_DATA_PIPE_3              0x08
#define _ENABLE_DATA_PIPE_4              0x10
#define _ENABLE_DATA_PIPE_5              0x20

#define _MASK_FREQUENCY_CHANNEL          0x7F
#define _MASK_RX_DATA_READY              0x40
#define _MASK_TX_DATA_READY              0x20
#define _MASK_POWER_MODE                 0x02
#define _MASK_RX_TX_CONTROL              0x01
#define _MASK_ENABLE_DPL                 0x04


nRF24L01P::nRF24L01P(void)
{
  /* Give the nrf time to startup */
  _delay_ms(100);

  my_spi.spi_init();
  
  /* Assign -1 value to the variables
  since the status is unknown at this moment */
  pwr_up  = -1;
  prim_rx = -1;

  flush_TX();
  flush_RX();
}

void nRF24L01P::writeData(uint8_t *data, uint8_t data_size)
{
	if(data_size < 0 || data_size > 32)
	{
		//Invalid data size
	}
	
	PORTB &= ~(1<<CE);
	
	PORTB &= ~(1<<SS);

	my_spi.spi_send_char(WRITE_TX_PAYLOAD_CMD);

	for(int i = 0; i < data_size; i++)
	{
		my_spi.spi_send_char(*data++);
	}
	PORTB |= 1<<SS;
	
	setTransmitMode();
	
	PORTB |= 1<<CE;

	_delay_us(15);

	PORTB &= ~(1<<CE);

	uint16_t cnt = 0;
	while(!( getStatusRegister() & _MASK_TX_DATA_READY) )
	{
		//Wait for the transfer to complete
		_delay_us(1);
		cnt++;
		if(cnt >= 1337) break;
	}
	flush_TX();
}



int8_t nRF24L01P::readData(uint8_t *data, uint8_t data_size)
{
	if(data_size < 0 || data_size > 32)
	{
		//Invalid data size
		return -1;
	}

	//Read the length of the payload
	PORTB &= ~(1<<SS);

	my_spi.spi_send_char(READ_RX_PAYLOAD_WIDTH_CMD);

	int8_t rxPayloadWidth = my_spi.spi_send_char(NOP_CMD);

	PORTB |= 1<<SS;

	if(rxPayloadWidth < 0 || rxPayloadWidth > 32)
	{
		return -1;
	}
	else if(rxPayloadWidth == 0)
	{
		return 0;
	}
	else
	{
		//Read the payload
		PORTB &= ~(1<<SS);

		my_spi.spi_send_char(READ_RX_PAYLOAD_CMD);

		for(int i = 0; i < data_size; i++)
		{
			*data++ = my_spi.spi_send_char(NOP_CMD);
		}
		PORTB |= 1<<SS;

		flush_RX();
		
		_delay_us(150);
		
		return rxPayloadWidth;
	}
}

bool nRF24L01P::readable(void)
{	
	if(prim_rx != 1) setReceiveMode();

	uint8_t status_reg = getStatusRegister();

	if((status_reg & _MASK_RX_DATA_READY) == _MASK_RX_DATA_READY)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void nRF24L01P::setReceiveMode(void)
{
	if(pwr_up != 1) powerUp();
	
	if(prim_rx != 1)
	{
		uint8_t old_reg = getRegister(REG_ADDRESS_CONFIG);

		uint8_t new_reg = old_reg | _MASK_RX_TX_CONTROL;

		setRegister(REG_ADDRESS_CONFIG, new_reg);

		flush_RX();
		
		PORTB |= 1<<CE;

		_delay_us(130);

		prim_rx = 1;
	}
}

void nRF24L01P::setTransmitMode(void)
{
	/* If previously receiving, restart nrf... */
	if (prim_rx == 1) powerDown();
	powerUp();

	if(prim_rx != 0)
	{
		PORTB &= ~(1<<CE);
		
		uint8_t old_reg = getRegister(REG_ADDRESS_CONFIG);
		
		uint8_t new_reg = old_reg & ~(_MASK_RX_TX_CONTROL);

		setRegister(REG_ADDRESS_CONFIG, new_reg);

		_delay_us(130);
		
		prim_rx = 0;
	}
	else
	{
		PORTB &= ~(1<<CE);
		
		_delay_us(130);
	}
}

void nRF24L01P::powerUp(void)
{
	if(pwr_up != 1)
	{
		uint8_t old_reg = getRegister(REG_ADDRESS_CONFIG);

		uint8_t new_reg = old_reg | _MASK_POWER_MODE;
		
		/* Always power up in transmit mode! */
		new_reg &= ~(_MASK_RX_TX_CONTROL);
		
		setRegister(REG_ADDRESS_CONFIG, new_reg);
		
		_delay_us(1500);

		pwr_up = 1;

		prim_rx = 0;
	}
}

void nRF24L01P::powerDown(void)
{
	if(pwr_up != 0)
	{
		PORTB &= ~(1<<CE);

		_delay_us(4);

		uint8_t old_reg = getRegister(REG_ADDRESS_CONFIG);

		uint8_t new_reg = old_reg & ~(_MASK_POWER_MODE);

		setRegister(REG_ADDRESS_CONFIG, new_reg);	

		pwr_up = 0;
	}
}

void nRF24L01P::setTransferSize(uint8_t pipe, uint8_t transfer_size)
{
	if(transfer_size < 0 || transfer_size > 32)
	{
		//No valid transfer size
	}
	else
	{
		switch(pipe)
		{
			case DATA_PIPE_0:
			  setRegister(REG_ADDRESS_RX_PW_P0, transfer_size);
			  break;

			case DATA_PIPE_1:
			  setRegister(REG_ADDRESS_RX_PW_P1, transfer_size);
			  break;

			case DATA_PIPE_2:
			  setRegister(REG_ADDRESS_RX_PW_P2, transfer_size);
			  break;

			case DATA_PIPE_3:
			  setRegister(REG_ADDRESS_RX_PW_P3, transfer_size);
			  break;

			case DATA_PIPE_4:
			  setRegister(REG_ADDRESS_RX_PW_P4, transfer_size);
			  break;

			case DATA_PIPE_5:
			  setRegister(REG_ADDRESS_RX_PW_P5, transfer_size);
			  break;

			default:
			  //Invalid pipe number
			  _delay_ms(1);
		}
	}
}

void nRF24L01P::enableDynamicPayload(void)
{
	//Enable dynamic payload length in 'feature' register
	uint8_t old_reg = getRegister(REG_ADDRESS_FEATURE);

	uint8_t new_reg = old_reg | _MASK_ENABLE_DPL;

	setRegister(REG_ADDRESS_FEATURE, new_reg);
	
	//Enable dynamic payload lengths on all pipes
	old_reg = getRegister(REG_ADDRESS_DYNPD);

	new_reg = old_reg | _ENABLE_DATA_PIPE_ALL;
	
	setRegister(REG_ADDRESS_DYNPD, new_reg);
}

void nRF24L01P::disableAutoAcknowledgment(void)
{
	uint8_t old_reg = getRegister(REG_ADDRESS_EN_AA);

	uint8_t new_reg = old_reg & _DISABLE_AUTO_ACKNOWLEDGE_ALL;

	setRegister(REG_ADDRESS_EN_AA, new_reg);
}

void nRF24L01P::disableAutoRetransmit(void)
{
	uint8_t old_reg = getRegister(REG_ADDRESS_SETUP_RETR);

	uint8_t new_reg = old_reg & _DISABLE_AUTO_RETRANSMIT_ALL;

	setRegister(REG_ADDRESS_SETUP_RETR, new_reg);
}

uint16_t nRF24L01P::getFrequencyChannel(void)
{
	uint8_t reg = getRegister(REG_ADDRESS_RF_CH);

	return (reg + MINIMUM_FREQUENCY_CHANNEL);
}

void nRF24L01P::setFrequencyChannel(uint16_t frequency)
{
	if(frequency < MINIMUM_FREQUENCY_CHANNEL || frequency > MAXIMUM_FREQUENCY_CHANNEL)
	{
		//No valid frequency
	}
	else
	{
		frequency -= MINIMUM_FREQUENCY_CHANNEL;

		uint8_t rf_ch = frequency & _MASK_FREQUENCY_CHANNEL;

		setRegister(REG_ADDRESS_RF_CH, rf_ch);
	}
}

void nRF24L01P::enableDataPipe(int8_t pipe)
{
	uint8_t old_reg = getRegister(REG_ADDRESS_EN_RXADDR);

	uint8_t new_reg = 0;

	switch(pipe)
	{
		case DATA_PIPE_ALL:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_ALL;
		  break;

		case DATA_PIPE_0:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_0;
		  break;

		case DATA_PIPE_1:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_1;
		  break;

		case DATA_PIPE_2:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_2;
		  break;

		case DATA_PIPE_3:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_3;
		  break;

		case DATA_PIPE_4:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_4;
		  break;

		case DATA_PIPE_5:
		  new_reg = old_reg | _ENABLE_DATA_PIPE_5;
		  break;

		default:
		  //Invalid pipe number
		  _delay_ms(1);
	}
	setRegister(REG_ADDRESS_EN_RXADDR, new_reg);
}

unsigned long long nRF24L01P::getRXAddress(uint8_t pipe)
{
  if(pipe < 0 || pipe > 5) 
  {
      //nRF24L01P: Error, invalid pipenumber.
	  return 0;
  }

  else
  {
	  //RX addresses can be 3, 4 or 5 bytes long
	  //For now we only use 5 byte addresses
	  uint8_t width = 5;

	  uint32_t address_arr[width];
	  
	  if(pipe == DATA_PIPE_0 || pipe == DATA_PIPE_1)
	  {
		  PORTB &= ~(1<<SS);

		  (pipe == DATA_PIPE_0) ? my_spi.spi_send_char(READ_REGISTER_CMD | REG_ADDRESS_RXADDR_PIPE_0) : my_spi.spi_send_char(READ_REGISTER_CMD | REG_ADDRESS_RXADDR_PIPE_1);

		  for(int i = 0; i < width; i++)
		  {
			  //Store the address in an array (MSByte First)
			  address_arr[i] = my_spi.spi_send_char(NOP_CMD);
		  }
		  PORTB |= 1<<SS;

		  unsigned long long address;

		  for(int i = 0; i < width; i++)
		  {
			  address += (address_arr[i] << (8 * i) );
		  }

		  return address;
	  }
	  else
	  {
		  //Todo, get addresses of pipe 2-5
		  return 0;
	  }
  }
}

void nRF24L01P::setRXAddress(uint8_t pipe, unsigned long long address)
{
	if(pipe < 0 || pipe > 5) 
	{
	    //nRF24L01P: Error, invalid pipe number.
	}

	else
	{
		if(pipe <= 1 && (address < 0x0000000000 || address > 0xFFFFFFFFFF))
		{
		    //nRF24L01P: Error, invalid address.
		}

		else if(pipe >= 2 && (address < 0x00 || address > 0xFF)) 
		{
		    //nRF24L01P: Error, invalid address.
		}

		else
		{
			if(pipe == DATA_PIPE_0 || pipe == DATA_PIPE_1)
			{
				//RX addresses can be 3, 4 or 5 bytes long
				//For now we only use 5 byte addresses
				uint8_t width = 5;

				//Store the address in an array (LSByte First)
				uint8_t address_arr[width];
				
				for(int i = 0; i < width; i++)
				{
					address_arr[i] = (address >> (i * 8));
				}
				
				PORTB &= ~(1<<SS);
				
				(pipe == DATA_PIPE_0) ? my_spi.spi_send_char(WRITE_REGISTER_CMD | REG_ADDRESS_RXADDR_PIPE_0) : my_spi.spi_send_char(WRITE_REGISTER_CMD | REG_ADDRESS_RXADDR_PIPE_1);
				
				for(int i = 0; i < width; i++)
				{
					my_spi.spi_send_char(address_arr[i]);
				}
				PORTB |= 1<<SS;
			}
			else
			{
				uint8_t reg_address;

				switch(pipe)
				{
					case DATA_PIPE_2:
					  reg_address = REG_ADDRESS_RXADDR_PIPE_2;
					  break;

					case DATA_PIPE_3:
					  reg_address = REG_ADDRESS_RXADDR_PIPE_3;
					  break;

					case DATA_PIPE_4:
					  reg_address = REG_ADDRESS_RXADDR_PIPE_4;
					  break;

					case DATA_PIPE_5:
					  reg_address = REG_ADDRESS_RXADDR_PIPE_5;
					  break;

					default:
					  //nRF24L01P: Error, something went wrong.
					  reg_address = 0;
					  break;
				}

				//Only the last byte can be set
				//The rest of the 4 bytes are equal to address of pipe 1
				uint8_t width = 1;

				//Store the address in an array (LSByte First)
				uint8_t address_arr[width];
				
				for(int i = 0; i < width; i++)
				{
					address_arr[i] = (address >> (i * 8));
				}

				PORTB &= ~(1<<SS);
				
				my_spi.spi_send_char(WRITE_REGISTER_CMD | reg_address);
				
				for(int i = 0; i < width; i++)
				{
					my_spi.spi_send_char(address_arr[i]);
				}
				PORTB |= 1<<SS;
			}
		}
	}
}

void nRF24L01P::setRXAddress(uint8_t pipe, uint8_t *address)
{
	if(pipe < 0 || pipe > 5)
	{
		//nRF24L01P: Error, invalid pipe number.
	}

	else
	{
		if(pipe == DATA_PIPE_0 || pipe == DATA_PIPE_1)
		{
			//RX addresses can be 3, 4 or 5 bytes long
			//For now we only use 5 byte addresses
			uint8_t width = 5;

			uint8_t width_cnt = 0;

			for(int i = 0; i < width; i++)
			{
				if(address[i] < 0x00 || address[i] > 0xFF) width_cnt = 0;
				else width_cnt++;
			}
			
			if(width_cnt != width)
			{
				//nRF24L01P: Error, something went wrong.
			}

			else
			{
				PORTB &= ~(1<<SS);
				
				(pipe == DATA_PIPE_0) ? my_spi.spi_send_char(WRITE_REGISTER_CMD | REG_ADDRESS_RXADDR_PIPE_0) : my_spi.spi_send_char(WRITE_REGISTER_CMD | REG_ADDRESS_RXADDR_PIPE_1);
				
				for(int i = 0; i < width; i++)
				{
					my_spi.spi_send_char(address[i]);
				}
				PORTB |= 1<<SS;
			}
		}
		else
		{
			uint8_t reg_address;
			
			switch(pipe)
			{
				case DATA_PIPE_2:
				reg_address = REG_ADDRESS_RXADDR_PIPE_2;
				break;

				case DATA_PIPE_3:
				reg_address = REG_ADDRESS_RXADDR_PIPE_3;
				break;

				case DATA_PIPE_4:
				reg_address = REG_ADDRESS_RXADDR_PIPE_4;
				break;

				case DATA_PIPE_5:
				reg_address = REG_ADDRESS_RXADDR_PIPE_5;
				break;

				default:
				//nRF24L01P: Error, something went wrong.
				reg_address = 0;
				break;
			}

			if(address[0] < 0x00 || address[0] > 0xFF)
			{
				//nRF24L01P: Error, invalid address.
			}
			
			PORTB &= ~(1<<SS);
			
			my_spi.spi_send_char(WRITE_REGISTER_CMD | reg_address);

			my_spi.spi_send_char(address[0]);

			PORTB |= 1<<SS;
		}
	}
}

unsigned long long nRF24L01P::getTXAddress(void)
{
	//Todo: implementation
	return 0;
}

void nRF24L01P::setTXAddress(unsigned long long address)
{
	if(address < 0x0000000000 || address > 0xFFFFFFFFFF)
	{
	    //nRF24L01P: Error, invalid address.
	}

	else
	{
		//TX Address is always 5 bytes
		uint8_t width = 5;
		
		//Store the address in an array (LSByte First)
		uint8_t address_arr[width];
		
		for(int i = 0; i < width; i++)
		{
			address_arr[i] = (address >> (i * 8));
		}

		PORTB &= ~(1<<SS);

		my_spi.spi_send_char(WRITE_REGISTER_CMD | REG_ADDRESS_TXADDR);

		for(int i = 0; i < width; i++)
		{
			my_spi.spi_send_char(address_arr[i]);
		}
		PORTB |= 1<<SS;
	}
}

void nRF24L01P::setTXAddress(uint8_t *address)
{
	//TX Address is always 5 bytes
	uint8_t width = 5;
		
	uint8_t width_cnt = 0;
		
	for(int i = 0; i < width; i++)
	{
		if(address[i] < 0x00 || address[i] > 0xFF) width = 0;
		else width_cnt++;
	}

	if(width_cnt != width)
	{
		//nRF24L01P: Error, something went wrong.
	}
	else
	{
		PORTB &= ~(1<<SS);

		my_spi.spi_send_char(WRITE_REGISTER_CMD | REG_ADDRESS_TXADDR);

		for(int i = 0; i < width; i++)
		{
			my_spi.spi_send_char(address[i]);
		}
		PORTB |= 1<<SS;
	}
}

uint8_t nRF24L01P::getCyclicRedundancyCheck(void)
{
	uint8_t reg = getRegister(REG_ADDRESS_CONFIG);

	uint8_t crc_length = reg & _MASK_CYCLIC_REDUNDANCY_CHECK;

	switch(crc_length)
	{
		case _CYCLIC_REDUNDANCY_CHECK_OFF:
		return CYCLIC_REDUNDANCY_CHECK_OFF;

		case _CYCLIC_REDUNDANCY_CHECK_1BYTE:
		return CYCLIC_REDUNDANCY_CHECK_1BYTE;

		case _CYCLIC_REDUNDANCY_CHECK_2BYTES:
		return CYCLIC_REDUNDANCY_CHECK_2BYTES;

		default:
		//Something went wrong
		return -1;
	}
}

void nRF24L01P::setCyclicRedundancyCheck(uint8_t crc_length)
{
	uint8_t old_reg = getRegister(REG_ADDRESS_CONFIG);

	old_reg &= _MASK_CYCLIC_REDUNDANCY_CHECK;

	uint8_t new_reg = 0;
	
	//Register 01 has to be zero in order to turn the CRC off
	if(crc_length == CYCLIC_REDUNDANCY_CHECK_OFF)
	{
		uint8_t en_aa_reg = getRegister(REG_ADDRESS_EN_AA);

		if(en_aa_reg == 0x00)
		{
			new_reg = old_reg | _CYCLIC_REDUNDANCY_CHECK_OFF;
		}
		else
		{
			//nRF24L01P: Error, could not set CRC off
		}
	}
	else
	{
		switch(crc_length)
		{
			case CYCLIC_REDUNDANCY_CHECK_1BYTE:
			new_reg = old_reg | _CYCLIC_REDUNDANCY_CHECK_1BYTE;
			break;

			case CYCLIC_REDUNDANCY_CHECK_2BYTES:
			new_reg = old_reg | _CYCLIC_REDUNDANCY_CHECK_2BYTES;
			break;

			default:
			//Something went wrong...
			_delay_ms(1);
		}
	}
	setRegister(REG_ADDRESS_CONFIG, new_reg);
}

int nRF24L01P::getAirDataRate(void)
{
	uint8_t reg = getRegister(REG_ADDRESS_RF_SETUP);

	uint16_t rate = reg & _MASK_AIR_DATA_RATE;

	switch(rate)
	{
		case _AIR_DATA_RATE_250KBPS:
		return AIR_DATA_RATE_250KBPS;

		case _AIR_DATA_RATE_1MBPS:
		return AIR_DATA_RATE_1MBPS;

		case _AIR_DATA_RATE_2MBPS:
		return AIR_DATA_RATE_2MBPS;

		default:
		//Something went wrong...
		return -1;
	}
}

void nRF24L01P::setAirDataRate(uint16_t rate)
{
	uint8_t old_reg = getRegister(REG_ADDRESS_RF_SETUP);

	old_reg &= ~(_MASK_AIR_DATA_RATE);

	uint8_t new_reg = 0;

	switch(rate)
	{
		case AIR_DATA_RATE_250KBPS:
		new_reg = old_reg | _AIR_DATA_RATE_250KBPS;
		break;

		case AIR_DATA_RATE_1MBPS:
		new_reg = old_reg | _AIR_DATA_RATE_1MBPS;
		break;

		case AIR_DATA_RATE_2MBPS:
		new_reg = old_reg | _AIR_DATA_RATE_2MBPS;
		break;

		default:
		//Something went wrong...
		_delay_ms(1);
	}
	setRegister(REG_ADDRESS_RF_SETUP, new_reg);
}

int nRF24L01P::getOutputPower(void)
{
	uint8_t reg = getRegister(REG_ADDRESS_RF_SETUP);

	uint8_t power = reg & _MASK_OUTPUT_POWER;

	switch(power)
	{
		case _OUTPUT_POWER_0DBM:
		return OUTPUT_POWER_0DBM;

		case _OUTPUT_POWER_MINUS_6DBM:
		return OUTPUT_POWER_MINUS_6DBM;

		case _OUTPUT_POWER_MINUS_12DBM:
		return OUTPUT_POWER_MINUS_12DBM;

		case _OUTPUT_POWER_MINUS_18DBM:
		return OUTPUT_POWER_MINUS_18DBM;

		default:
		//Something went wrong...
		return -1;
	}
}

void nRF24L01P::setOutputPower(int8_t power)
{
	uint8_t old_reg = getRegister(REG_ADDRESS_RF_SETUP);

	old_reg &= ~(_MASK_OUTPUT_POWER);

	uint8_t new_reg = 0;

	switch(power)
	{
		case OUTPUT_POWER_0DBM:
		new_reg = old_reg | _OUTPUT_POWER_0DBM;
		break;

		case OUTPUT_POWER_MINUS_6DBM:
		new_reg = old_reg | _OUTPUT_POWER_MINUS_6DBM;
		break;

		case OUTPUT_POWER_MINUS_12DBM:
		new_reg = old_reg | _OUTPUT_POWER_MINUS_12DBM;
		break;

		case OUTPUT_POWER_MINUS_18DBM:
		new_reg = old_reg | _OUTPUT_POWER_MINUS_18DBM;
		break;

		default:
		//Something went wrong...
		_delay_ms(1);
	}
	setRegister(REG_ADDRESS_RF_SETUP, new_reg);
}

void nRF24L01P::flush_TX(void)
{
	//Flush the TX FIFO buffer
	PORTB &= ~(1<<SS);
	
	my_spi.spi_send_char(FLUSH_TX_FIFO_CMD);
	
	PORTB |= 1<<SS;

	//Clear TX status bit
	uint8_t status_reg = getStatusRegister();
	
	uint8_t new_reg = status_reg | _MASK_TX_DATA_READY;
	
	setRegister(REG_ADDRESS_STATUS, new_reg);
}

void nRF24L01P::flush_RX(void)
{
	//Clear RX status bit
	uint8_t status_reg = getStatusRegister();
	
	uint8_t new_reg = status_reg | _MASK_RX_DATA_READY;
	
	setRegister(REG_ADDRESS_STATUS, new_reg);

	//Flush the RX FIFO buffer
	PORTB &= ~(1<<SS);
	
	my_spi.spi_send_char(FLUSH_RX_FIFO_CMD);
	
	PORTB |= 1<<SS;
}

uint8_t nRF24L01P::getRegister(uint8_t regAddress)
{
	PORTB &= ~(1<<SS);

	my_spi.spi_send_char(READ_REGISTER_CMD | regAddress);

	uint8_t reg = my_spi.spi_send_char(NOP_CMD);

	PORTB |= 1<<SS;

	return reg;
}

void nRF24L01P::setRegister(uint8_t regAddress, uint8_t regData)
{
	PORTB &= ~(1<<SS);

	my_spi.spi_send_char(WRITE_REGISTER_CMD | regAddress);

	my_spi.spi_send_char(regData);

	PORTB |= 1<<SS;

	_delay_us(4);
}

uint8_t nRF24L01P::getStatusRegister(void)
{
	PORTB &= ~(1<<SS);
	
	uint8_t reg = my_spi.spi_send_char(NOP_CMD);
	
	PORTB |= 1<<SS;

	_delay_us(4);

	return reg;
}
