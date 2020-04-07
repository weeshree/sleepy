#include "Arduino.h"'
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>

TFT_eSPI tft = TFT_eSPI();

const uint8_t PIN_LCD = 27;
const int AUDIO_IN = A0; //pin where microphone is connected
const int SOUND_PIN = 13;

const int RED_PIN = 25;
const int BLUE_PIN = 12;
const int GREEN_PIN = 14;
const int SERVO_PIN = 19;
const int SERVO_PIN2 = 17;

const int PHOTO_PIN = 34;
const int TEMP_PIN = A3;

const int SOUND_CHANNEL = 0;
const uint32_t RED_CHANNEL = 1; //hardware pwm channel used in secon part
const uint32_t BLUE_CHANNEL = 2; //hardware pwm channel used in secon part
const uint32_t GREEN_CHANNEL = 3; //hardware pwm channel used in secon part

const int JINGLE_LEN = 100;

const char* NETWORK     = "";     // your network SSID (name of wifi network)
const char* PASSWORD = ""; // your network password

uint32_t timer;
uint32_t lastClap;
uint32_t secondLastClap;
uint32_t thirdLastClap;
uint32_t fourthLastClap;

boolean wakeUp = false;

WiFiClientSecure client; //global WiFiClient Secure object

void setup() {
  Serial.begin(115200);               // Set up serial port
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  Serial.begin(115200); //begin serial comms
  delay(100); //wait a bit (100 ms)

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
  timer = millis();
  digitalWrite(PIN_LCD, HIGH);
}


boolean tripped = false; // to make sure we only detect things once
boolean lcdOn = true;
boolean playingJingle = false;



int numReadings = 0;
float avgLight = 0;
float avgTemp = 0;
float avgSound = 0;
int active = 0;

int hr = 0;
int mint = 0;

int zeroCounter = 0;

boolean nightlightOn = false;
int nightlightTimer = 0;
int arr[300];
void loop() {
  // put your main code here, to run repeatedly:
  
  if(nightlightOn && millis()-nightlightTimer<5000)
  {
    ledcWrite(RED_CHANNEL, (int) (255. * (5000-millis()+nightlightTimer)/5000.));
    ledcWrite(GREEN_CHANNEL, (int) (20. * (5000-millis()+nightlightTimer)/5000.));
    ledcWrite(BLUE_CHANNEL, 0);
  }
  else if(nightlightOn && millis()-nightlightTimer>=5000)
  {
    nightlightOn = false;
  }
  else if(!playingJingle && wakeUp)
  {
      ledcWrite(RED_CHANNEL,255);
      ledcWrite(GREEN_CHANNEL,0);
      ledcWrite(BLUE_CHANNEL,0);
  }
  else if(playingJingle)
  {
    playJingle();
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
  
  if(millis() % 60000 == 0)
  {
    int fanPos = getFan();
    if(fanPos%4==0) {digitalWrite(SERVO_PIN, LOW); digitalWrite(SERVO_PIN2, LOW);}
    if(fanPos%4==1) {digitalWrite(SERVO_PIN, LOW); digitalWrite(SERVO_PIN2, HIGH);}
    if(fanPos%4==2) {digitalWrite(SERVO_PIN, HIGH); digitalWrite(SERVO_PIN2, LOW);}
    if(fanPos%4==3) {digitalWrite(SERVO_PIN, HIGH); digitalWrite(SERVO_PIN2, HIGH);}
    
    post(avgLight, avgTemp, avgSound, active);
    avgLight = 0;
    avgTemp = 0;
    avgSound = 0;
    numReadings = 0;
    active = 0;
    delay(2);
  }
//  Serial.printf("%d | %f", light, temp);
//  Serial.println();

//  Serial.println(sound);
  arr[numReadings%30] = sound;
  int maxS = -1, minS = 1500000;
  for(int i=0; i<30; i++) 
  {
    maxS = max(maxS, arr[i]);
    minS = min(minS, arr[i]);
  }
//  if(sound == 0) {Serial.println(0); zeroCounter++;}
//  else {
//    if(zeroCounter > 0) Serial.println("drats");
//    zeroCounter=0;
//  }
  
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
      playingJingle = true;
      
      
      lcdOn = !lcdOn;
      if(lcdOn) digitalWrite(PIN_LCD, HIGH);
      else digitalWrite(PIN_LCD, LOW);
    }
  }
}

int lenSoFar = 0;
int duration = 50;
int music_timer = 0;
note_t notes[] = {NOTE_C, NOTE_Cs, NOTE_D, NOTE_Eb, NOTE_E, NOTE_F, NOTE_Fs, NOTE_G, NOTE_Gs, NOTE_A, NOTE_Bb, NOTE_B};
int curTonic = 70;
int curNote = 70;

void playJingle()
{
  if(millis() - music_timer > duration)
  {
    updateNote();
    if(curNote < 30 || curNote > 100) curNote = curTonic;
    if(random(10)==7) curNote -= curNote%12;
    
    music_timer = millis();
    duration = random(20, 100);

    ledcWriteNote(SOUND_CHANNEL, notes[curNote%12], curNote/12);

    ledcWrite(RED_CHANNEL, random(100,255));
    ledcWrite(BLUE_CHANNEL, random(100,255));
    ledcWrite(GREEN_CHANNEL, random(100,255));
    

    lenSoFar++;
    if(lenSoFar == JINGLE_LEN) 
    {
      playingJingle = false;
      lenSoFar = 0;

      ledcWriteNote(SOUND_CHANNEL, notes[curTonic%12], curNote/12);
      delay(20);
      ledcWrite(SOUND_CHANNEL, 0);
      ledcWrite(RED_CHANNEL,0);
      ledcWrite(GREEN_CHANNEL,0);
      ledcWrite(BLUE_CHANNEL,0);
    }
  }
}

int blues[] = {3,5,6,7,10,12};
int dist[] = {1,40,10,6,3,2,0};

int randInterval() {
  int sum = 0;
  for(int i=0; i<7; i++) sum+=dist[i];
  int go = random(sum);

  int ind = -1;
  while(go>=0) 
  {
    ind += 1;
    go -= dist[ind];
  }
  return ind;
}

void updateNote() {
    int up = random(2)*2 - 1;

    int curPos = 0;
    for(int i=0; i<6; i++) if((curNote+1200-curTonic)%12 == blues[i]%12) curPos = i;

    int interval = randInterval();
    int tempNote = curNote + blues[(curPos + interval*up + 600)%6]-blues[curPos];
    if(up>0 && tempNote < curNote) curNote = tempNote + 12;
    else if(up<0 && tempNote > curNote) curNote = tempNote - 12;
    else curNote = tempNote;

//    Serial.printf("%d        | %d %d %d", curNote, curPos, interval, up);
//    Serial.println();
}


const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host

int getFan() {
  sprintf(request_buffer, "GET HTTP/1.1\r\n"); // YOUR URL HERE
  strcat(request_buffer, "Host: \r\n"); // YOUR HOST HERE
  strcat(request_buffer, "\r\n");

  do_http_GET("", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true); // YOUR HOST HERE


  int ans = atoi(response_buffer);
  return ans;
}


void post(float light, float temp, float sound, int awakeState)
{
  char body[200]; //for body;
  sprintf(body, "light=%f&temp=%f&sound=%f&active=%d", light, temp, sound, awakeState); //generate body, posting to User, 1 step
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(request_buffer, "POST  HTTP/1.1\r\n"); // YOUR URL HERE
  strcat(request_buffer, "Host: \r\n"); // YOUR HOST HERE
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer, "\r\n"); //new line from header to body
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true); // YOUR HOST HERE
  Serial.println("AHH");
  Serial.print(response_buffer);
  Serial.println("| AHH");
  Serial.println(strcmp(response_buffer, "keep sleeping\n"));
  wakeUp = !(strcmp(response_buffer, "keep sleeping\n") == 0);
}

void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    //if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      //if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    //if (serial) Serial.println(response);
    client.stop();
    //if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}        




void do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    //if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      //if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    //if (serial) //Serial.println(response);
    client.stop();
    //if (serial) //Serial.println("-----------");
  } else {
    if (serial) //Serial.println("connection failed :/");
    if (serial) //Serial.println("wait 0.5 sec...");
    client.stop();
  }
}

uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}
