# IOT Long Distance Relationship Colorcube
Design for a pair of networked RGB LED widgets that synchronously change color when one of them is squeezed.

I know there are similar things out there, but I'm intentionally forging forward without looking at other implementations in order to force myself to properly learn this stuff.

~~The current design uses a Wildfire v4 (an Arduino-flavor board made by WickedDevice). This device is not super well-suited to this project,
but it has an integrated ESP8266 wifi chip and a microSD thingy and also I have like 5 of them somehow, so that's what I'm using.~~

The newest version of the design uses an Adafruit Feather HUZZAH ESP8266 board. In contrast to the Wildfire, this board is very well suited to this project and costs less than $20. Instead of a generic RGB LED driven using PWM, the newest hardware revision now uses a "NeoPixel" WS2812 RGB LED breakout board. 

HARDWARE DESIGN (Prototype completed!)
---------------------
The devices each consist of a plastic 3D printed base (containing the microcontroller board) with an RGB led protruding from the base. The LED is covered by a silicon cube-shaped diffuser, which also contains a motion sensor to detect squeezing. Power is provided by 5V USB. A small Li-ion battery might be included.

SOFTWARE DESIGN (Done! See Prototype_ESP8266_V2.ino)
---------------------
The software side of things should be pretty simple. The Wildfire collects sensor data and randomly generates 3 values (for RGB) when a squeeze is detected. These values are published via MQTT to a feed on io.adafruit.com. The device is also subscribed to the same feed, and will collect both values it publishes and values from other cubes. When data is recieved through the subscription, the device parses the data and uses the RGB values to change its own color.

ENCLOSURE DESIGN (Prototype completed! See 3mf files)
---------------------
The enclosure consists of a two-part 3D printed shell with holes for wires (and to provide access to the USB port) and standoffs for boards. There is a sort of brim thingy to hold the silicone cube using a friction fit. This design will probably go through several refinement stages. Holes in the standoffs (for screws) will be drilled by hand because the 3D printer is bad at holes.
