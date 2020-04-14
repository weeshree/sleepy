#ifndef musicbox_h
#define musicbox_h
#include "Arduino.h"
class MusicBox
{
  private:

  const note_t notes[12] = {NOTE_C, NOTE_Cs, NOTE_D, NOTE_Eb, NOTE_E, NOTE_F, NOTE_Fs, NOTE_G, NOTE_Gs, NOTE_A, NOTE_Bb, NOTE_B};
  const int blues[7] = {3,5,6,7,10,12};  
  const int dist[7] = {1,40,10,6,3,2,0}; 
  const int JINGLE_LEN=100;
  
  int duration;
  
  int lenSoFar;
  int music_timer;
  int curTonic;
  int curNote;

  int RED_CHANNEL;
  int GREEN_CHANNEL;
  int BLUE_CHANNEL;
  int SOUND_CHANNEL;

  public:
    boolean playingJingle;
    MusicBox();
    void setChannels(int a, int b, int c, int d);
    void playJingle();
    int randInterval();
    void updateNote();
    
};
#endif
