#include "Arduino.h"
#include "musicbox.h"

MusicBox::MusicBox() {
  lenSoFar = 0;
  music_timer = 0;
  curTonic = 70;
  curNote = 70;
  playingJingle = false; 
  duration=50;
}

void MusicBox::setChannels(int red, int green, int blue, int sound) {
  RED_CHANNEL=red;
  BLUE_CHANNEL=blue;
  GREEN_CHANNEL=green;
  SOUND_CHANNEL=sound;
}
void MusicBox::playJingle()
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

int MusicBox::randInterval() {
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

void MusicBox::updateNote() {
    int up = random(2)*2 - 1;

    int curPos = 0;
    for(int i=0; i<6; i++) if((curNote+1200-curTonic)%12 == blues[i]%12) curPos = i;

    int interval = randInterval();
    int tempNote = curNote + blues[(curPos + interval*up + 600)%6]-blues[curPos];
    if(up>0 && tempNote < curNote) curNote = tempNote + 12;
    else if(up<0 && tempNote > curNote) curNote = tempNote - 12;
    else curNote = tempNote;
}
