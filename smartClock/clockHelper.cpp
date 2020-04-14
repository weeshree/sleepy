
#include "clockHelper.h"
#include "musicbox.h"

#include "getpost.h"
#include <SPI.h>
#include <WiFiClientSecure.h>


Clock::Clock() {
  Serial.begin(115200);
  tripped = false; // to make sure we only detect things once
  lcdOn = true;
    
  numReadings = 0;
  avgLight = 0;
  avgTemp = 0;
  avgSound = 0;
  active = 0;
  
  hr = 0;
  mint = 0;
  
  zeroCounter = 0;
  
  nightlightOn = false;
  nightlightTimer = 0;

  wakeUp = false;

  box.setChannels(RED_CHANNEL, GREEN_CHANNEL, BLUE_CHANNEL,SOUND_CHANNEL);
  clockSetup();
}

void Clock::clockSetup() {
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(SERVO_PIN2, OUTPUT);
  
  pinMode(PIN_LCD, OUTPUT);
  ledcSetup(SOUND_CHANNEL, 1E5, 12);
  ledcAttachPin(SOUND_PIN, SOUND_CHANNEL);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  ledcSetup(RED_CHANNEL, 50, 8);
  ledcAttachPin(RED_PIN, RED_CHANNEL);
  
  ledcSetup(BLUE_CHANNEL, 50, 8);
  ledcAttachPin(BLUE_PIN, BLUE_CHANNEL);
  
  ledcSetup(GREEN_CHANNEL, 50, 8);
  ledcAttachPin(GREEN_PIN, GREEN_CHANNEL);

  timer = millis();
}



void Clock::ClockLoop() {
  // put your main code here, to run repeatedly:
//  Serial.printf("%d %d %d %d %d %d", RED_CHANNEL, GREEN_CHANNEL, BLUE_CHANNEL, RED_PIN, GREEN_PIN, BLUE_PIN);
  digitalWrite(PIN_LCD, LOW);
  if(nightlightOn && millis()-nightlightTimer<1000)
  {
    ledcWrite(RED_CHANNEL, (int) (255. * (1000-millis()+nightlightTimer)/1000.));
    ledcWrite(GREEN_CHANNEL, (int) (20. * (1000-millis()+nightlightTimer)/1000.));
    ledcWrite(BLUE_CHANNEL, 0);
  }
  else if(nightlightOn && millis()-nightlightTimer>=1000)
  {
    nightlightOn = false;
  }
  else if(!box.playingJingle && wakeUp)
  {
    ledcWrite(RED_CHANNEL,255);
    ledcWrite(GREEN_CHANNEL,0);
    ledcWrite(BLUE_CHANNEL,0);
  }
  else if(box.playingJingle)
  {
    box.playJingle();
  }
  else
  {
      ledcWrite(RED_CHANNEL,0);
      ledcWrite(GREEN_CHANNEL,0);
      ledcWrite(BLUE_CHANNEL,0);    
  }

  int light = analogRead(PHOTO_PIN);
  float temp = (analogRead(TEMP_PIN)/1023. - .5)*100 * 9/5 + 32;
  int sound = analogRead(AUDIO_IN);

  numReadings++;
  
  avgLight = avgLight * (numReadings-1)/numReadings + light*1./numReadings;
  avgTemp = avgTemp * (numReadings-1)/numReadings + temp/numReadings;
  avgSound = avgSound * (numReadings-1)/numReadings + sound*1./numReadings;

  if(box.playingJingle) return;
  
  if(millis() % 60000 == 0)
  {
    int fanPos = gp.getFan();
    if(fanPos%4==0) {digitalWrite(SERVO_PIN, LOW); digitalWrite(SERVO_PIN2, LOW);}
    if(fanPos%4==1) {digitalWrite(SERVO_PIN, LOW); digitalWrite(SERVO_PIN2, HIGH);}
    if(fanPos%4==2) {digitalWrite(SERVO_PIN, HIGH); digitalWrite(SERVO_PIN2, LOW);}
    if(fanPos%4==3) {digitalWrite(SERVO_PIN, HIGH); digitalWrite(SERVO_PIN2, HIGH);}
    
    wakeUp = gp.post(avgLight, avgTemp, avgSound, active);
    avgLight = 0;
    avgTemp = 0;
    avgSound = 0;
    numReadings = 0;
    active = 0;
    delay(2);
  }

  arr[numReadings%30] = sound;
  int maxS = -1, minS = 1500000;
  for(int i=0; i<30; i++) 
  {
    maxS = max(maxS, arr[i]);
    minS = min(minS, arr[i]);
  }
  
  if(maxS - minS > 2000 && millis() - lastClap > 100)
  {
    Serial.println("CLAP");
    tripped = false;
    fourthLastClap = thirdLastClap;
    thirdLastClap = secondLastClap;
    secondLastClap = lastClap;
    lastClap = millis();
  }



  if(millis()-lastClap > 1000 && !tripped)
  {
    tripped = true;
    if(lastClap - secondLastClap < 1100 && secondLastClap - thirdLastClap > 1500) // two claps
    {
      Serial.println("TWO CLAPS");
      nightlightTimer = millis();
      nightlightOn = true;
      active = 2;
    }
    if(lastClap - secondLastClap < 1100 && secondLastClap - thirdLastClap < 1100 && thirdLastClap - fourthLastClap > 1500) // three claps
    {
      Serial.println("THREE CLAPS");
      active = 3;
      box.playingJingle = true;
    }
  }
}
