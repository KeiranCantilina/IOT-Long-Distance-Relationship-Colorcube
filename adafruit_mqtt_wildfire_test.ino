/***************************************************
 * Wicked Device Wildfire v4 MQTT-Adafruit.io Example
 * 
 * 
 * This is the Adafruit MQTT Library ESP8266 Example, 
 * modified to work on the Wildfire v4 boards made by WickedDevice
 * (http://shop.wickeddevice.com/) using the WickedDevice ESP8266 AT Client
 * library (found at https://github.com/WickedDevice/ESP8266_AT_Client).
 * 
 * This bit of code serves as an example of how to get the Wildfire v4 to 
 * work with io.adafruit.com using MQTT, since the code shipped with the 
 * boards (supporting the sparkfun.io Phant servers) is now obsolete.
 * 
 * Written/rehashed/butchered by Keiran Cantilina
 * Attribution-NonCommercial licesnse 3.0 United States (CC BY-NC 3.0 US)
 * 
 * Boilerplate from original Adafruit MQTT Library ESP8266 Example code
 * is included below. Please include both the above and below text in any
 * redistribution. 
 * 
 */


/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

 ****************************************************/
 
 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h" // Found at https://github.com/adafruit/Adafruit_MQTT_Library
#include <ESP8266_AT_Client.h> // Found at https://github.com/WickedDevice/ESP8266_AT_Client

// sketch must instantiate a buffer to hold incoming data
// 1500 bytes is way overkill for MQTT, but if you have it, may as well
// make space for a whole TCP packet
#define ESP8266_INPUT_BUFFER_SIZE (1500)
uint8_t input_buffer[ESP8266_INPUT_BUFFER_SIZE] = {0};     

/************************* WiFi Access Point *********************************/

#define NETWORK_SSID     "NETGEAR16"
#define NETWORK_PASSWORD "coolsparrow028"

// Arduino digital the pin that is used to reset/enable the ESP8266 module
int esp8266_enable_pin = 23; 

// Serial1 is the 'stream' the AT command interface is on
Stream * at_command_interface = &Serial1;  


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Keirancantilina"
#define AIO_KEY         "5526f708aeb648ea907a38d95af2b5a2"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// ORR...Wildfire client object
ESP8266_AT_Client esp(esp8266_enable_pin, at_command_interface); // instantiate the client object


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&esp, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/keiran-block");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ren-block");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial1.begin(115200);              // AT command interface

  char ip_str[16] = {0};
  char mac_str[18] = {0};
  uint8_t mac_addr[6] = {0};
  uint32_t ip_addr = 0;
  
  esp.setInputBuffer(input_buffer, ESP8266_INPUT_BUFFER_SIZE); // connect the input buffer up
  esp.reset();                                                 // reset the module
  
  
  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.print("Set Mode to Station...");
  esp.setNetworkMode(1);
  Serial.println("OK");
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(NETWORK_SSID);

  esp.connectToNetwork(NETWORK_SSID, NETWORK_PASSWORD, 60000, NULL); 
  Serial.println("OK");  
  Serial.println();
  Serial.println("WiFi connected");
  
  
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton);
}

//Initialize variable used for payload value
uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }

  // Now we can publish stuff!
  Serial.print(F("\nSending photocell val "));
  Serial.print(x);
  Serial.print("...");
  if (! photocell.publish(x++)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
