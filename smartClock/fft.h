#ifndef fft_h
#define fft_h
#include "Arduino.h"
#include "arduinoFFT.h"
#include "math.h"
#include <SPI.h>
#include <TFT_eSPI.h>

#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz

class Visualizer
{
  private:

  TFT_eSPI tft;

  
  unsigned int sampling_period_us;
  unsigned long timer;
  
  double vReal[SAMPLES];
  double vImag[SAMPLES];
  
  arduinoFFT FFT;
  const int AUDIO_IN = A0; //pin where microphone is connected
  const uint8_t PIN_LCD = 27; // pin where I connected my LCD, change stuff if LCD connected to 3V3 instead
  
  const uint8_t BAR_WIDTH = 2;
  const uint8_t SCREEN_YMAX = 159;
  const double SCREEN_XMAX = 125;
  const uint16_t MAX_AMPLITUDE = 40000;
  
  const String notes[13] = {"A", "Bb", "B", "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A"};
  const double values[13] = {27.5, 29.1352, 30.8677, 32.7032, 34.6478, 36.7081, 38.8909, 41.2034, 43.6535, 46.2493, 48.9994, 51.9131, 55};

    
  int positions[SAMPLES/2];

  boolean bunny;

  byte red;
  byte green;
  byte blue;

  unsigned int startColor;
  unsigned int endColor;

  uint16_t readings[SAMPLES/2];
  uint16_t prevReadings[SAMPLES/2];

  String prevNote;
  int textColor;
  public:
    void updatePosArray();
    Visualizer();
    void softMaxer(double *arr, int len);
    int getColorFromPos(double pos);
    void draw();
    String getNote(double peak);
    void drawNote(boolean loud, double peak);
    void VisLoop();
};
#endif
