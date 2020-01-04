/*This is the third generation codebase for the LED Relationship Light Project.
  This codebase is compatible with the Adafruit Feather HUZZAH (and theoretically with other ESP8266-based boards) and NeoPixel RGB-LEDs.
  Adafruit.io is used as the MQTT server, and the WifiManager library is used to handle user-side Wifi config via a captive portal that pops up
  if connection fails.

  This third generation code differs from the second generation in that it includes some fun extras, like crossfading transitions.
    
  The most recent hardware uses these WS2812 Neopixels: https://www.adafruit.com/product/1612

  Pins used:
  IDE pin 14 (marked on board as 14)(Physical Pin 5): WS2812 DataIn
  VBUS (marked on board as USB)OR 3V3(if 5V fails)  : WS2812 Power In
  GND                                               : WS2812 GND
  IDE pin 4 (marked on board as 4)(Physical pin D13): Button GND
  IDE pin 5 (marked on board as 5)(Physical pin D14): Button Detect (Interrupt attached and using internal pullup)
  IDE pin 0 (marked on board as 0)(Physical pin D3) : Status LED (built-in power LED, write HIGH to turn off ONLY AFTER BOOT)
  
*/


/********************* Libraries ****************************/
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h> // Found at https://github.com/adafruit/Adafruit_MQTT_Library

#include <Adafruit_NeoPixel.h>

/********************* LED Setup ****************************/
#define NEOPIXEL_PIN 14 // NeoPixels are connected to this pin
#define LED_COUNT 1
#define BUTTON_PIN 5
#define BUTTON_GND 4
#define DEBOUNCE_DELAY 100 // in ms
#define STATUS_LED 0

Adafruit_NeoPixel RGBlight(LED_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // Declare Neopixel object

uint32_t last_interrupt_time = 0;
uint8_t led_status = 0; //  Debugging LED flag
volatile int flag = 0;
volatile long times_run = 0;

int current_color[3] = {0,0,0};
int blue_array[3] = {0,0,255};
int red_array[3] = {255,0,0};

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Keirancantilina"
#define AIO_KEY         "5526f708aeb648ea907a38d95af2b5a2"

/************ Global State (you don't need to change this!) ******************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'publish_feed' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish publish_feed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/keiran-block");

// Setup a feed called 'subscribe_feed' for subscribing to changes.
Adafruit_MQTT_Subscribe subscribe_feed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/keiran-block");


// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup() {

/********************* RGB and Button Init ****************************/
  // Initialize Neopixel
  RGBlight.begin();
  RGBlight.clear();
  RGBlight.show();
  
  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // Setup interrupt handling and LED for debug purposes
  //pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr_handler, CHANGE); // CHANGE for dome lights, RISING for little button switches
  pinMode(STATUS_LED, OUTPUT);

  // This is our fake ground connection for the switch
  pinMode(BUTTON_GND, OUTPUT);
  digitalWrite(BUTTON_GND, LOW);

  // Start with LED red to show setup
  RGBlight.setPixelColor(0,RGBlight.gamma32(RGBlight.Color(red_array[0],red_array[1],red_array[2])));
  RGBlight.show();
  memcpy(current_color,red_array,sizeof(current_color));

/********************* Serial Setup ****************************/  
  //Initialize Serial comms
  Serial.begin(115200);

  //Give things time to warm up (this is purely magical thinking, I know)
  delay(10);
  Serial.println(F("Keiran's Long-Distance Friendship Cube"));
  
/********************* Wifi Setup ****************************/
  //Initialize WifiManager captive portal
  WiFiManager wifiManager;

  //Callback to function that executes right before config mode
  wifiManager.setAPCallback(configModeCallback);

  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("ESP8266_KKC", "password");

  Serial.println("WiFi connected! Yay");
/********************* END Wifi Setup ****************************/

  // Setup MQTT subscription for subscribe_feed.
  mqtt.subscribe(&subscribe_feed);
}



//Initialize variable used for payload value (RGB color)
volatile long x=0;


// Main program loop
void loop() {
  
  //THis used to be a function but for whatever reason it broke mqtt connection even before it was ever called...somehow it works just pasted in...
  // x = rand_color();

  //Randomly generate RGB values for output as the payload. Values are 0-255 with 111 added to eliminate problems with 0 padding.
  long red_int = random(111,366);
  long green_int = random(111,366);
  long blue_int = random(111,366);

  long million = 1000000;
  long thousand = 1000;
  
  // Ints get concatenated together into the payload "string". For whatever reason strings and chars are no good so this whole thing is a giant long. WHYYYY
  x = (red_int*million)+(green_int*thousand)+(blue_int);
  
  mqtt.subscribe(&subscribe_feed);
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  

  Adafruit_MQTT_Subscribe *subscription;

  
  
  while (flag == 0 && (subscription = mqtt.readSubscription(1000))) {
    if (subscription == &subscribe_feed) {
      Serial.println();
      Serial.print(F("Message recieved: "));
      Serial.println((char *)subscribe_feed.lastread);
      Serial.println();

      // Write received color to LED
      int new_color[3];
      new_color[0] = string_to_red((char *)subscribe_feed.lastread)-111;
      new_color[1] = string_to_green((char *)subscribe_feed.lastread)-111;
      new_color[2] = string_to_blue((char *)subscribe_feed.lastread)-111;


      // LED crossfade (is inline because it was too slow as a function, for some reason)
      int greatest;
      int color_difference[3] = {new_color[0]-current_color[0],new_color[1]-current_color[1],new_color[2]-current_color[2]};
  
      if(abs(color_difference[0])>= abs(color_difference[1])){
        if(abs(color_difference[0])>= abs(color_difference[2])){
          greatest = color_difference[0];
        }
        else{
          greatest = color_difference[2];
         }
      }
      else{
        if(abs(color_difference[1])>= abs(color_difference[2])){
          greatest = color_difference[1];
        }
        else{
          greatest = color_difference[2];
        }
      }
  
      for (int i=0; i< abs(greatest); i++){
        for (int j=0; j<3;j++){
          if (current_color[j] != new_color[j]){
            current_color[j] = current_color[j] + sgn(color_difference[j]);
          }
        }

        RGBlight.setPixelColor(0,RGBlight.gamma32(RGBlight.Color(current_color[0],current_color[1],current_color[2])));
        RGBlight.show();
        delay(10);
      }
      // ---------------------------------------------------------------------------------------
    }  
  }

  //If first time running, turn LED blue to indicate ready status and turn off status red LED
  if (times_run == 0){
    RGBlight.setPixelColor(0,RGBlight.gamma32(RGBlight.Color(blue_array[0],blue_array[1],blue_array[2])));
    RGBlight.show();
    memcpy(current_color, blue_array, sizeof(current_color));
    
    digitalWrite(STATUS_LED, HIGH);
  }
  
  if (flag == 1){
    //x = rand_color();
    Serial.print(F("\nSending color value: "));
    Serial.print(x);
    Serial.print("...");
    //if (! publish_feed.publish((char *)x)) {
    char buffer [10];
    int ret = snprintf(buffer, sizeof(buffer), "%ld", x);
    char * const p = buffer;
    if (! publish_feed.publish(p)) {
      Serial.println(F("Failed"));
      Serial.println();
    } else {
      Serial.println(F("OK!"));
      Serial.println();
    }
    flag = 0;
  }
    

  Serial.println("No message so far! Will look again...");

  // Increment times run counter
  if(times_run == 0){
    times_run++;
  }
  
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
//  if(! mqtt.ping()) {
//    mqtt.disconnect();
//  }
  
}




// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT...");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         RGBlight.setPixelColor(0,RGBlight.gamma32(RGBlight.Color(red_array[0],red_array[1],red_array[2]))); // Red light for error
         RGBlight.show();
         memcpy(current_color,red_array,sizeof(current_color));
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  Serial.println();
}



// Functions to take string with color data and turn it into rgb values
int string_to_red(char color_string[]){
  char r[]={0,0,0};
  for (byte q = 0; q < 3; q = q + 1) {
    r[q] = color_string[q];
  }
  return atol(r);
}
int string_to_green(char color_string[]){
  char g[]={0,0,0};
  for (byte q = 0; q < 3; q = q + 1) {
    g[q] = color_string[q+3];
  }
  return atol(g);
}
int string_to_blue(char color_string[]){
  char b[]={0,0,0};
  for (byte q = 0; q < 3; q = q + 1) {
    b[q] = color_string[q+6];
  }
  return atol(b);
}



// Interrupt handling
ICACHE_RAM_ATTR void isr_handler() {
  uint32_t interrupt_time = millis();
  // Debounce
  if (interrupt_time - last_interrupt_time > DEBOUNCE_DELAY) {
    led_status = !led_status;
    //digitalWrite(LED_BUILTIN, led_status);  // Enable this for visual debugging switch/interrupt stuff
    flag = 1;
  }
  last_interrupt_time = interrupt_time;
}



// Function for handling failed wifi connections
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // Placeholder for future callback code here (debug, maybe)
  Serial.println(myWiFiManager->getConfigPortalSSID());
}



// Sign finding function
inline int sgn(int val) {
  if (val < 0) return -1;
  if (val==0) return 0;
  return 1;
}
