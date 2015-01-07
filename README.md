# FonaFromUbidots
Arduino pulls (RGB) data (JSON) from Ubidots and drives NeoPixel accordingly. Goes in sleep mode afterwards. 
This is not a beautiful code, but it works. Due to the harsh memory restrictions of the ATMEL (1KB) it is quite hard to parse JSON in a nicer way, but i am sure there are better solutions possible.
Approach can easily be adopted to other sources (because it is just a standard http-get call) or other variables.
