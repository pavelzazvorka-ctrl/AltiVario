#include "Sound.h"

Sound::Sound() {
}

void Sound::init(int _sndPin, VarioCfg *cfg)
{
  int audiochannel=0;
  
  sndPin = _sndPin;    
  beepStyle = cfg->BeepStyle;
  sinkTreshold = cfg->SinkThreshold;
  climbTreshold = cfg->ClimbThreshold;
  pinMode(sndPin, OUTPUT);  

  ledcAttachPin(sndPin, audiochannel);
  ledcSetup(audiochannel, 4000, 14);
  audio.Config(sndPin, audiochannel, cfg);  
}

void Sound::setSoundStyle(int _beepStyle)
{
  beepStyle = _beepStyle;  
}

void Sound::setSinkThreshold(int SinkTreshold)
{
   sinkTreshold = SinkTreshold;
}

void Sound::setClimbThreshold(int ClimbTreshold)             
{
  climbTreshold = ClimbTreshold;
}

void Sound::update(float climbRate) 
{
  int cps = (int)(100.00 * climbRate);   
  if (beepStyle > 0) // multiple styles later on 
  {
    audio.VarioBeep(cps,beepStyle); // varioAudio
  }
}
