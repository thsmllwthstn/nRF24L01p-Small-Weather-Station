#include <avr/io.h>
#include "spi.h"

#define OUTPUT_POWER_0DBM                  0
#define OUTPUT_POWER_MINUS_6DBM           -6
#define OUTPUT_POWER_MINUS_12DBM         -12
#define OUTPUT_POWER_MINUS_18DBM         -18

#define AIR_DATA_RATE_250KBPS            250
#define AIR_DATA_RATE_1MBPS             1000
#define AIR_DATA_RATE_2MBPS             2000

#define CYCLIC_REDUNDANCY_CHECK_OFF        0
#define CYCLIC_REDUNDANCY_CHECK_1BYTE      1
#define CYCLIC_REDUNDANCY_CHECK_2BYTES     2

#define DATA_PIPE_ALL                     -1
#define DATA_PIPE_0                        0
#define DATA_PIPE_1                        1
#define DATA_PIPE_2                        2
#define DATA_PIPE_3                        3
#define DATA_PIPE_4                        4
#define DATA_PIPE_5                        5

#define MINIMUM_FREQUENCY_CHANNEL       2400
#define MAXIMUM_FREQUENCY_CHANNEL       2525

class nRF24L01P 
{
  private:
    int8_t pwr_up, prim_rx;

	spi my_spi;
    
  public:
    nRF24L01P(void);
	
    /* Write data to the TX Payload 
     * @*data: the data that needs to be send
     * @data_size: size of the data thats been send (max 32 bytes) */
    void writeData(uint8_t *data, uint8_t data_size);

    /* Read data from the RX Payload 
     * @*data: the received data is placed in this variable
     * @data_size: size of the data (max 32 bytes) 
     * @return: size of the data (max 32, -1 when something is wrong)*/
    int8_t readData(uint8_t *data, uint8_t data_size);

    /* Listen to incomming data 
     * @return: if data is available */
    bool readable(void);

    /* Set nrf in RX mode (max 130us transition time)*/
    void setReceiveMode(void);

    /* Set nrf in TX mode (max 130us transition time) */
    void setTransmitMode(void);

    /* Enter Standbymode I (max 1,5ms transition time) */
    void powerUp(void);

    /* Enter low current power down mode */
    void powerDown(void);

    /* When dynamic payload length hasnt been enabled the
     * transfer size needs te be defined
     * @pipe: pipenumber (0 through 5)
     * @transfer_size: size of transfered data (max 32 bytes) */
    void setTransferSize(uint8_t pipe, uint8_t transfer_size);

    /* Enable Dynamic payload lengths on all data pipes */
    void enableDynamicPayload(void);

    /* Gets frequency thats being send on
     * @return: height of frequency in MHz (2400 trough 2525) */
    uint16_t getFrequencyChannel(void);

    /* Sets frequency thats being send on
     * @frequency: height of frequency in MHz (2400 trough 2525) */
    void setFrequencyChannel(uint16_t frequency);
     
    /* Enables receive addresses on data pipes (pipe 0 and 1 are standard open)
     * @pipe: pipenumber that needs to be opened (0 trough 5), -1 for all pipes */
    void enableDataPipe(int8_t pipe);

    /* Gets receive address from given pipe
     * @pipe: pipenumber (0-5)
     * @return: 40-bit receive address */
	 //Warning: function not used
	 //Todo: somehow cant add up 8-bit addresses to 64-bit address
    unsigned long long getRXAddress(uint8_t pipe);
    
    /* Sets receive address for specific pipe number
     * The standard addresses can be found in the datasheet
     * @pipe: pipenumber which address is set
     * @address: 40-bit receive address */
    void setRXAddress(uint8_t pipe, unsigned long long address); 

	/* Sets receive address for specific pipe number
     * The standard addresses can be found in the datasheet
     * @pipe: pipenumber which address is set
     * @address: 5-byte array receive address */
    void setRXAddress(uint8_t pipe, uint8_t *address); 

    /* Gets transmit address
     * @return: 40-bit transmit address */
	 //Warning: function not used
	 //Todo: somehow cant add up 8-bit addresses to 64-bit address
    unsigned long long getTXAddress(void);
    
    /* Sets Transmit address
     * @address: 40-bit transmit address */
    void setTXAddress(unsigned long long address);

    /* Sets Transmit address
     * @address: 5-byte array transmit address */
    void setTXAddress(uint8_t *address);
    
    /* Disable auto acknowledgment on all pipes */
    void disableAutoAcknowledgment(void);

	/* Disable Auto Retransmission on all pipes */
	void disableAutoRetransmit(void);

    /* Get the length of Cyclic Redundacny Check
     * @return: length of cyclic redundancy check in bytes (0, 1 or 2) */
    uint8_t getCyclicRedundancyCheck(void);

    /* Set Cyclic Redundancy Check (always set when Auto Acknowledgment is enabled)
     * @crc_length: number of bytes for CRC (0, 1 or 2) */
    void setCyclicRedundancyCheck(uint8_t crc_length);

    /* Get RF air data reate
     * @return: data rate in kbps (250, 1000 or 2000) */
    int getAirDataRate(void);

    /* Set RF air data rate
     * @rate: speed of datarate in kbps (250, 1000 or 2000) */
    void setAirDataRate(uint16_t rate);

    /* Get RF outputpower in TX mode
     * @return: outputpower in dBM (0, -6, -12 or -18) */
    int getOutputPower(void);

    /* Set RF outputpower in TX mode
     * @power: hight of power in dBM (0, -6, -12 or -18 */
    void setOutputPower(int8_t power);
    
	/* Flush receive buffer and receive flag */
	void flush_RX(void);

	/* Flush transmit buffer and transmit flag */
	void flush_TX(void);

    /* Read an addressable register
     * @regAddress: address of the register
     * @return:     data of the register */
    uint8_t getRegister(uint8_t regAddress);

    /* Set an addressable register
     * @regAddress: address of the register
     * @regData:    the new register */
    void setRegister(uint8_t regAddress, uint8_t regData);
         
    /* Read that Status register
     * @return: status register */
    uint8_t getStatusRegister(void);
};