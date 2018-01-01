/* 
 *  Small Weather Station
 * 
 * Simply power the Small Weather Station with 3V3 - 12V and it begins
 * sending data.
 * 
 * To change settings press the button and the LED should turn ON.
 * The station is now in receiving mode.
 * 
 * Type the following commands in the serial monitor to change the settings:
 * |----------------------------------------------|
 * |      COMMAND                  EXAMPLE STRING |
 * |----------------------------------------------|
 * |t=[time in seconds]s         | t=100s         | --> sets timer to 100 seconds
 * |ta=[tx address]              | ta=AAAAB       | --> sets transmit address to 0x4141414142*
 * |r[pipe number]=[rx address]  | r0=JJJJK       | --> sets receive address of pipe 0 to 0x4A4A4A4A4B*
 * |fre=[frequency]              | fre=2450       | --> sets frequency to 2450MHz**
 * |----------------------------------------------|
 * 
 * Note*: don't forget to change the addresses of this program also!
 * Note**: don't forget to change the frequency of this program also!
 * 
 * The settings are safed even when resetting (power down) the station.
 * 
 * To reset the settings to default configuration hold the button down 
 * until the LED begings toggeling.
 * 
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//When you connect an external DS18B20 temperature sensor to
//the PCB, change the transfer size to 10
#define TRANSFER_SIZE 8
//#define TRANSFER_SIZE 10

#define DATA_PIPE_0 0
#define DATA_PIPE_1 1
#define DATA_PIPE_2 2
#define DATA_PIPE_3 3
#define DATA_PIPE_4 4
#define DATA_PIPE_5 5
#define tx_address 0xE7E7E7E7E7
#define rx_address 0xE7E7E7E7E7

RF24 my_nrf(9,10);

uint8_t rx_data[TRANSFER_SIZE];
char tx_data[TRANSFER_SIZE];

void setup(void)
{
  Serial.begin(115200);
  printf_begin();

  //Standard nRF24L01+ Settings for communication with the Small Weather Station
  //Don't forget to change these when changing the settings of the station!
  my_nrf.begin();
  my_nrf.setPALevel(RF24_PA_MIN);
  my_nrf.setDataRate(RF24_250KBPS);
  my_nrf.setCRCLength(RF24_CRC_8);
  my_nrf.setPayloadSize(TRANSFER_SIZE);
  my_nrf.setChannel(2);
  my_nrf.setAutoAck(false);
  my_nrf.openReadingPipe(DATA_PIPE_0, rx_address);
  my_nrf.openWritingPipe(tx_address);
  my_nrf.startListening();
 
  //Print the nRF24L01+ settings
  my_nrf.printDetails();
  Serial.print("Transfer size    = ");
  Serial.print(TRANSFER_SIZE);
  Serial.println(" bytes\n");
   
  Serial.println("*********** NOW LISTENING FOR REQUESTS FROM THE SMALL WEATHER STATION ***********");
}
void loop(void)
{
  if(my_nrf.available() ) 
  {
    my_nrf.read(rx_data, sizeof(rx_data));

    //Check if the received data is an error indicator, change of settings or
    //represents values from the sensors
    if(rx_data[0] == 't' && rx_data[1] == '=')
    {
      Serial.print("\nReceived data back from the Small Weather Station!\nThe timer is set to: ");

      for(int i = 2; i < sizeof(rx_data); i++)
      {     
        char c = rx_data[i];
        if(c == 's')
        {
          break;
        }
        else
        {
          Serial.print(c);
        }
      }
      Serial.println(" seconds\n");
    }
    else if(rx_data[0] == 'r' && rx_data[2] == '=')
    {
      Serial.print("\nReceived data back from the Small Weather Station!\nThe receive address of pipe ");
      
      uint8_t pipe = rx_data[1] - '0';
      
      Serial.print(pipe);
      
      Serial.print(" is set to: 0x");
      
      if(pipe <= 2)
      {
        for(int i = 3; i < sizeof(rx_data); i++)
        {
          Serial.print(rx_data[i], HEX);
        }
      }
      else 
      {
        Serial.print(rx_data[3], HEX);
      }
      Serial.println();
    }
    else if(rx_data[1] == 'e' && rx_data[2] == 'r' && rx_data[3] == 'r' && rx_data[4] == 'o' && rx_data[5] == 'r') 
    {
      Serial.print("\nReceived data back from the Small Weather Station!\nReceived message: ");
      
      for(int i = 0; i < sizeof(rx_data); i++) 
      {
        Serial.print((char)rx_data[i]);
      }
    }

    //************ Print and calculate the data values from the Small Weather Station ************//
    else
    {
        //*** Print the raw data ***//
        for(int i = 0; i < sizeof(rx_data); i++)
        {
          Serial.print(rx_data[i]);
          Serial.print(' ');
        }
        
      //****************** Calculate the temperature from the DS18B20 ******************//
        //The most precise 12-bit resolution is used    
        const float RESOLUTION = 0.0625;
          
        uint16_t raw_temperature = (rx_data[1] << 8) + rx_data[0];
        
        float Celcius = (float) raw_temperature * RESOLUTION;
        
        Serial.print("\nDS18B20 Temperature : ");
        Serial.println(Celcius);
        
    
      //************* Calculate the humidity and temperature from the DHT11 ************//
        float dht_humidity = rx_data[2] + (rx_data[3] * 0.01);
    
        float dht_temperature = rx_data[4] + (rx_data[5] * 0.01);
    
        Serial.print("DHT11 Humidity      : ");
        Serial.println(dht_humidity);
        Serial.print("DHT11 Temperature   : ");
        Serial.println(dht_temperature);
        
    
      //******************** Calculate the value from the 10-bit ADC ********************//    
        //Since a 10-bit ADC is used, the highest value is 1023
        uint16_t analogValue = (rx_data[7] << 8) + rx_data[6];
    
        Serial.print("Analog value        : ");
        Serial.println(analogValue);
      
      // When connected, calculate the temperature from the externally connected DS18B20 //
        if(TRANSFER_SIZE == 10)
        {
          uint16_t raw_external_temperature = (rx_data[9] << 8) + rx_data[8];
          
          Celcius = (float) raw_external_temperature * RESOLUTION;
          
          Serial.print("Waterproof DS18B20  : ");
          Serial.println(Celcius);
        }
        Serial.println();
      }
  }

  //Send the user input to the Small Weather Station
  //Make sure the Small Weather Station is in receiving mode! (onboard led is ON)
  if(Serial.available() )
  {
    String s = Serial.readString();
    
    my_nrf.stopListening();
    
    //Add the user input to the buffer
    for(int i = 0; i < sizeof(tx_data); i++)
    {
      tx_data[i] = s[i];
    }

    if(s.length() < sizeof(tx_data) )
    {
      //Add padding to make the data 8 or 10 bytes
      for(int i = s.length(); i < sizeof(tx_data); i++) 
      {
        tx_data[i] = '0';
      }
    }
    
    if(!my_nrf.write(tx_data, sizeof(tx_data)) ) 
    {
      Serial.println("Something went wrong, try again!");
    }
    else
    {
      Serial.print("Sended: '");
      for(int i = 0; i < sizeof(tx_data); i++)
      {
        Serial.print(tx_data[i]);
      }
      Serial.println("' to the Small Weather Station\nWaiting for response...");
    }
    my_nrf.startListening(); 
  }
}
