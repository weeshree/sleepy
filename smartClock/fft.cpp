#include "Arduino.h"
#include "fft.h"
#include "arduinoFFT.h"
#include "math.h"
#include <SPI.h>
#include <TFT_eSPI.h>


#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz


Visualizer::Visualizer()
{

  FFT = arduinoFFT();
  
  tft = TFT_eSPI();

  sampling_period_us = round(1000000 * 1./SAMPLING_FREQUENCY);
  pinMode(PIN_LCD, OUTPUT);
  digitalWrite(PIN_LCD, HIGH);
  
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_WHITE, TFT_BLACK); //set col

  updatePosArray();

  
  bunny = true; // ensures we don't run multiple loops simultaneously

  startColor = 255 << 11 | 0 << 5 | 31;
  endColor = 255 << 11 | 63 << 5 | 0;

  prevNote = "";
  textColor = 0;
}


void Visualizer::updatePosArray()
{
  for(int i=2; i<SAMPLES/2; i++)
  {
    double frequency = i*10000 / SAMPLES;
    double bassCutoff = 5*10000./SAMPLES; // around 200Hz
    
    if(frequency<=bassCutoff) // separate the bass sounds from the rest
    {
      double offset = log(2.*10000 / SAMPLES);
      double posOfBox = (log(frequency) - offset) / (log(bassCutoff) - offset) * SCREEN_XMAX / 5.; // bass sounds take up 1/5 of the screen
      positions[i] = (int) posOfBox;
      if(i<=6)
      {
        Serial.printf("%d_%f", i, posOfBox);
        Serial.println();
      }
    }
    else 
    {
      double offset = log(6*10000./SAMPLES);
      double posOfBox = BAR_WIDTH + (log(frequency)-offset) / (log(1760.) - offset) * SCREEN_XMAX * 4. / 5. + SCREEN_XMAX * 1. / 5.; // rest of sounds take up 4/5 of the screen. 1760 denotes the max frequency we can display given the tiny size of LCD.
      positions[i] = (int) posOfBox;
      if(i<=7)
      {
        Serial.printf("%d %f %f %f %f", i, posOfBox, frequency, offset, SCREEN_XMAX*1./5.);
        Serial.println();
      }
    }
  }
}

void Visualizer::softMaxer(double *arr, int len)
{
  double sum = 0.0;
  for(int i=2; i<len; i++) sum += arr[i];
  for(int i=2; i<len; i++) arr[i] /= sum;

   sum = 0.0;
  for(int i=2; i<len; i++) sum+=arr[i]*(arr[i]);
  for(int i=2; i<len; i++) arr[i] = arr[i]*(arr[i])/sum;
}



int Visualizer::getColorFromPos(double pos)
{
  double frac = 1. - ((1.*pos) / SCREEN_XMAX);
  byte newRed = ((startColor & 255<<11)>>11) * frac + ((endColor & 255<<11)>>11) * (1-frac);
  byte newGreen = ((startColor & 63<<5)>>5) * frac + ((endColor & 63<<5)>>5) * (1-frac);
  byte newBlue = (startColor & 31) * frac + (endColor & 31) * (1-frac);
  return newRed << 11 | newGreen << 5 | newBlue;
}

void Visualizer::draw() {
  for(int i=2; i<SAMPLES/2; i++)
  {
    if(readings[i] < prevReadings[i])
      tft.fillRect(positions[i], 0 , 5*BAR_WIDTH, SCREEN_YMAX, TFT_BLACK);
  }
  
  for(int i=2; i<SAMPLES/2; i++)
  {
    tft.fillRect(positions[i], SCREEN_YMAX, 5*BAR_WIDTH, 4, getColorFromPos(positions[i]));
    for(int j=0; j<readings[i]; j+=5)
    {
      tft.fillRect(positions[i], SCREEN_YMAX - j, 5*BAR_WIDTH, 4, getColorFromPos(positions[i]));
    }
//    tft.fillRect(positions[i], SCREEN_YMAX - readings[i], 5*BAR_WIDTH, readings[i], getColorFromPos(positions[i]));
    prevReadings[i] = readings[i];
  }
}


String Visualizer::getNote(double peak)
{
  int octave = 1;
  while(peak >= 55)
  {
    peak/=2;
    octave+=1; 
  }

  int indexOfNote=0;
  for(int i=0; i<13; i++)
    if((values[indexOfNote]-peak)*(values[indexOfNote]-peak) > (values[i]-peak)*(values[i]-peak)) 
      indexOfNote=i;
  indexOfNote%=12;

  return notes[indexOfNote];
}

void Visualizer::drawNote(boolean loud, double peak)
{
  if(loud)
  {
    String newNote = getNote(peak);
    if(newNote != prevNote)
    {
      tft.setTextColor(TFT_BLACK, TFT_BLACK); 
      tft.drawString(prevNote, SCREEN_XMAX/2+10, 10,4);      
    }
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    prevNote = newNote;
    tft.drawString(prevNote, SCREEN_XMAX/2+10, 10,4);
  }
  else
  {
    tft.setTextColor(TFT_BLACK, TFT_BLACK); 
    tft.drawString(prevNote, SCREEN_XMAX/2+10, 10,4);
  }
}

void Visualizer::VisLoop() {
  digitalWrite(PIN_LCD, HIGH);
  if(!bunny) return;
  bunny = false; 
  
  // put your main code here, to run repeatedly:
  for(int i=0; i < SAMPLES; i++)
  {
    timer = micros();
    vReal[i] = analogRead(AUDIO_IN);
    vImag[i] = 0;

    while(micros() < timer+sampling_period_us) {}
  }

  
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES); // this part updates array vReal of length SAMPLES with the amplitudes for each frequency, i/SAMPLES * SAMPLE_SIZE, i ranging from 1 to SAMPLE_SIZE. Only the first SAMPLES/2 amplitudes are usable though.
  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  
  int indOfMax = 2;
  for(int i=2; i<SAMPLES/2; i++)
  {
    if (vReal[i] > vReal[indOfMax]) indOfMax = i;
  }
  
  double standard = vReal[indOfMax]; // store some standardization number before shifting the amplitudes
  softMaxer(vReal,SAMPLES/2); // amplifies the loud frequencies and dulls the soft frequencies

  boolean loud = false;
  for(int i=2; i<SAMPLES/2; i++)
  {
    double frequency = i*10000 / SAMPLES;
    
    readings[i] = (SCREEN_YMAX * vReal[i]/vReal[indOfMax] * standard/MAX_AMPLITUDE); 
    readings[i] = (readings[i] - readings[i]%5);
    if(readings[i] <= 10) // ignore really low amplitude frequencies
      readings[i] = 0;
    else
      loud=true;
    
  }

  draw();

  drawNote(loud, peak);

  
  bunny=true;
}
