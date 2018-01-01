# Small Weather Station with nRF24L01+

For any nRF24L01+ project!

Vcc: 3.3V - 15V


### Description
Start a nRF24L01+ sensor network or expand your existing projects with this Small Weather Station!
This comes with easy to use example codes for Arduino and Raspberry Pi!

This station measures and sends:

 -Temperature
 
 -Humidity
 
 -Lightintensity

To achieve this the station uses three different sensors:

 -DS18B20: Digital temperature sensor
 
 -DHT11: Basic temperature and humidity sensor
 
 -LDR: Measures lightintensity



### What do you need?
You will need a Small Weather Station and your own nRF24L01+ receiver.
You can buy the Small Weather Station in the link below:

--ebay link

Or you can build your own station, you can find the schematics and source code in the link below:
https://github.com/thsmllwthstn/nRF24L01p-Small-Weather-Station/tree/master/station/station_basic

In the link below you will find an easy example code written for Arduino and Python 3 for receiving data from the station:
https://github.com/thsmllwthstn/nRF24L01p-Small-Weather-Station/tree/master/receiver

Simply copy and paste this code and you are ready to retreive data from the station!



### How does it work?
Power the Vin pin with 3V3 - 15V.

As soon as the onboard LED blinks 2 times in a row the station starts sending the data.

Use the example code to receive the data.

It sends the data every 5 seconds.

You can change the stations settings by pressing the button. The LED will go ON. The station is now in receiving mode.
There are 4 settings you can change:

 -Timer (rate at which the station sends the data in seconds)
 
 -Tranceive address
 
 -Receive address
 
 -Frequency

The example code explains in detail how to change these settings.


### Add a waterproof digital temperature sensor!
The station comes with 3 additional pins for connecting a waterproof DS18B20 temperature sensor.
Solder the Vcc, data and ground pins from the temperature sensor to the 3V3, data and GND pin from the station.
Once connected the station detects the presence of the added DS18B20 sensor.
The station will now send the standard readings with the added DS18B20 sensor!



### Programmable Small Weather Station?
Do you want full access to all the settings or use the station to control any other 3V3 circuit?
This is possible with a USBASP programmer and the programmable small weather station: 

--ebay link

Or you can build your own programmable station, you can find the schematics and source code in the link below:
https://github.com/thsmllwthstn/nRF24L01p-Small-Weather-Station/tree/master/station/station_basic_with_programmer
