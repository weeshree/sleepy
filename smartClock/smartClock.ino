#include "Arduino.h"
#include "arduinoFFT.h"
#include "math.h"
#include "fft.h"
#include "clockHelper.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>

#define VIS 0
#define CLO 1

const int BUT_PIN = 16;

Visualizer vis;
Clock clo; 


int state = VIS;

WiFiClientSecure client;

const char* NETWORK="";     // your network SSID (name of wifi network)
const char* PASSWORD=""; // your network password

void setup() {
  Serial.begin(115200);

  
  WiFi.begin(NETWORK, PASSWORD); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

  pinMode(BUT_PIN, INPUT_PULLUP);
  
}


unsigned long lastPress;
void loop()
{
//  Serial.println("balls");
  if(!digitalRead(BUT_PIN) && millis()-lastPress>1500)
  {
    lastPress=millis();
    state = 1-state; 
  }
  
  if(state==VIS)
  {
    vis.VisLoop();
  }
  else
  {
    clo.ClockLoop();
  }
}
