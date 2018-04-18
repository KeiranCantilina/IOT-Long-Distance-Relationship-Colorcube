# IOT-Long-Distance-Relationship-Colorcube
Design for a pair of networked RGB LED widgets that synchronously change color when one of them is squeezed.

I know there are similar things out there, but I'm intentionally forging forward without looking at other implementations in order to force myself to properly learn this stuff.

The current design uses a Wildfire v4 (an Arduino-flavor board made by WickedDevice). This device is not super well-suited to this project,
but it has an integrated ESP8266 wifi chip and a microSD thingy and also I have like 5 of them somehow, so that's what I'm using.

HARDWARE DESIGN
---------------------
The devices each consist of a plastic 3D printed base (containing the microcontroller board) with an RGB led protruding from the base. The LED is covered by a silicon cube-shaped diffuser, which also contains a bend sensor to detect squeezing. Power is provided by 5V USB. A small Li-ion battery might be included.

SOFTWARE DESIGN
---------------------
The software side of things should be pretty simple. The Wildfire collects sensor data and randomly generates 3 values (for RGB) when a squeeze is detected. These values are first used to change the color of the originating cube, then the values are concatenated with an ID string and published via MQTT to a feed on io.adafruit.com. The device is also subscribed to the same feed. When data is recieved through the subscription, the device parses the data and checks the ID string. If the data originated from the other cube, the cube uses the RGB values to change its own color.
