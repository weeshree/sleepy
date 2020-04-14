#ifndef clockHelper_h
#define clockHelper_h
#include "Arduino.h"
#include "musicbox.h"
#include "getpost.h"
#include <SPI.h>
#include <WiFiClientSecure.h>

class Clock
{
  private:

  const uint8_t PIN_LCD=27;
  const int AUDIO_IN=A0; //pin where microphone is connected
  const int SOUND_PIN=13;
  
  const int RED_PIN=25;
  const int BLUE_PIN=12;
  const int GREEN_PIN=26;
  const int SERVO_PIN=19;
  const int SERVO_PIN2=17;
  
  const int PHOTO_PIN=34;
  const int TEMP_PIN=A3;
  
  const int SOUND_CHANNEL=0;
  const uint32_t RED_CHANNEL=1; //hardware pwm channel used in secon part
  const uint32_t BLUE_CHANNEL=2; //hardware pwm channel used in secon part
  const uint32_t GREEN_CHANNEL=3; //hardware pwm channel used in secon part

  uint32_t timer;
  uint32_t lastClap;
  uint32_t secondLastClap;
  uint32_t thirdLastClap;
  uint32_t fourthLastClap;
  
  boolean wakeUp;

  boolean tripped; // to make sure we only detect things once
  boolean lcdOn;
  boolean playingJingle;

  int numReadings;
  float avgLight;
  float avgTemp;
  float avgSound;
  int active;
  
  int hr;
  int mint;
  
  int zeroCounter;
  
  boolean nightlightOn;
  int nightlightTimer;
  int arr[300];
  public:
    Clock();
    GetPost gp;
    MusicBox box;
    void clockSetup();
    void ClockLoop();
    
};
#endif
