#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic




void setup() {
  //Initialize WifiManager captive portal
  WiFiManager wifiManager;

  //Callback to function that executes right before config mode
  wifiManager.setAPCallback(configModeCallback);
  
  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("ESP8266_KKC", "password");

  pinMode(0, OUTPUT);
}

void loop() {
  //Blinky
  digitalWrite(0, HIGH);
  delay(500);
  digitalWrite(0, LOW);
  delay(500);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // Placeholder for future callback code here (debug, maybe)
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
