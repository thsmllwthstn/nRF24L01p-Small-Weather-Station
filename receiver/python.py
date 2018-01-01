#
# Small Weather Station 
# 
# Simply power the Small Weather Station with 3V3 - 12V and it begins
# sending data.
# 
# To change settings press the button and the LED should turn ON.
# The station is now in receiving mode.
# 
# Send the following commands to change the stations settings:
# |----------------------------------------------|
# |      COMMAND                  EXAMPLE STRING |
# |----------------------------------------------|
# |t=[time in seconds]s         | t=100s         | --> sets timer to 100 seconds
# |ta=[tx address]              | ta=AAAAB       | --> sets transmit address to 0x4141414142*
# |r[pipe number]=[rx address]  | r0=JJJJK       | --> sets receive address of pipe 0 to 0x4A4A4A4A4B*
# |fre=[frequency]              | fre=2450       | --> sets frequency to 2450MHz**
# |----------------------------------------------|
#
# ATTENTION! --> Make sure the message you send is the same as the TRANSFER_SIZE,
#                so when needed, add padding to the message to make it longer.
#
# Note*: don't forget to change the addresses of this program also!
# Note**: don't forget to change the frequency of this program also!
# 
# The settings are safed even when resetting (power down) the station.
# 
# To reset the settings to default configuration hold the button down 
# until the LED begings toggeling.
#

import urllib
import time
import RPi.GPIO as GPIO
from lib_nrf24 import NRF24
import spidev

DATA_PIPE_0 = 0
DATA_PIPE_1 = 1
DATA_PIPE_2 = 2
DATA_PIPE_3 = 3
DATA_PIPE_4 = 4
DATA_PIPE_5 = 5
tx_address = [0xE7, 0xE7, 0xE7, 0xE7, 0xE7]
rx_address = [0xE7, 0xE7, 0xE7, 0xE7, 0xE7]

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
my_nrf = NRF24(GPIO, spidev.SpiDev())
my_nrf.begin(0, 25)

#When you connect an external DS18B20 temperature sensor to
#the PCB, change the transfer size to 10
TRANSFER_SIZE = 8
#TRANSFER_SIZE = 10

#Standard nRF24L01+ Settings for communication with the Small Weather Station
#Don't forget to change these when changing the settings of the station!
my_nrf.setPALevel(NRF24.PA_MIN)
my_nrf.setDataRate(NRF24.BR_250KBPS)
my_nrf.setCRCLength(NRF24.CRC_8)
my_nrf.setPayloadSize(TRANSFER_SIZE)
my_nrf.setChannel(2)
my_nrf.setAutoAck(False)
my_nrf.openWritingPipe(tx_address)
my_nrf.openReadingPipe(DATA_PIPE_0, rx_address)
my_nrf.startListening()

#Print the nRF24L01+ settings
my_nrf.printDetails()
print("Transfer size    = " + str(TRANSFER_SIZE) + " bytes\n")

print("******* NOW LISTENING FOR REQUESTS FROM THE SMALL WEATHER STATION *******\n")

while True:   
    if my_nrf.available():
        rx_data = []
        my_nrf.read(rx_data, TRANSFER_SIZE)

        #*************** The first 2-bytes are from the DS18B20 temperature sensor ***************

        #The most precise 12-bit resolution is used
        RESOLUTION = 0.0625
            
        raw_temperature = (rx_data[1] << 8) + rx_data[0]
            
        Celcius = raw_temperature * RESOLUTION

        print("DS18B20 Temperature: " + str(Celcius) )


        #****************** The next 4-bytes are from the DHT11 humidity sensor ******************

        dht_humidity = rx_data[2] + (rx_data[3] * 0.01)

        dht_temperature = rx_data[4] + (rx_data[5] * 0.01)

        print("DHT11 Humidity     : " + str(dht_humidity) )
        print("DHT11 Temperature  : " + str(dht_temperature) )


        #************* The last 2-bytes represent the analog value from the 1-bit ADC ************

        analogValue = (rx_data[7] << 8) + rx_data[6]

        print("Analog value       : " + str(analogValue) )


        #**** When connected, calculate the temperature from the externally connected DS18B20 ****
        if TRANSFER_SIZE == 10:
            raw_external_temperature = (rx_data[9] << 8) + rx_data[8]

            Celcius = raw_external_temperature * RESOLUTION;

            print("Waterproof DS18B20  : " + str(Celcius) )

        print("")

    
    
