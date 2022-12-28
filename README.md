# DCController

This code is meant to run on an [ESP32](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-DEVKITC-32E/12091810?utm_adgroup=RF%20%26%20RFID&utm_source=google&utm_medium=cpc&utm_campaign=Dynamic%20Search_EN_Product&utm_term=&utm_content=RF%20%26%20RFID&gclid=CjwKCAiA8bqOBhANEiwA-sIlN_6lPVy-qwUohb6T_cuS2tMjNfuP78e8ND6qHMTm7K2qIdEMqKqn-RoCtbgQAvD_BwE) to turn on/off a dust collector when it receives commands from a central MQTT server. 
In order to avoid dealing with high voltage/current, I purchased a spare remote control for my dust collector and wired up the 
digital pins to "press" the remote control buttons to turn on/off the dust collector.

In addition to turning on/off the dust collector, this code also has a couple of other functions:

1. It is connected to a simple flip switch on digital pin 16. When this digital pin changes state, it broadcasts a MQTT signal to a central gate coordiator. 
I use this to signal to the central gate coordinator that all automatically controlled blast gates should be closed so I can open some manual blast gates.
2. It can also be hooked up to the receiver of an [Adafruit Keyfob 4-Button RF Remote Control](https://www.adafruit.com/product/1095?gclid=CjwKCAiA8bqOBhANEiwA-sIlNwkPvGRZcULmMqJVtMudUFIADKRpczrymkDQEP6tlOBwfcPijjPUdhoCV2UQAvD_BwE) and broadcasts an MQTT signal when a particular remote control switch is pressed (I haven't need to use this functionality yet).

This project is part of an overall set of projects mean to automate dust collection in my woodworking workshop.
