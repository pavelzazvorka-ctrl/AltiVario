#ifndef sound_h
#define sound_h

#include <Arduino.h>
#include "Vario.h"
#include "Tone32.h"
#include "VarioAudio.h"

class Sound {
  public:
    Sound();

    void init(int _sndPin, VarioCfg *cfg);
    void setSoundStyle(int beepStyle);
    void setSinkThreshold(int SinkTreshold);
    void setClimbThreshold(int ClimbTreshold);                
    void update(float climbRate); 

  private:

    VarioAudio audio;
    int beepStyle;
    int sinkTreshold;  // *100
    int climbTreshold; // *100
    int sndPin;
        
    unsigned long btime;
    bool bsnd;

    float toneFreqLowpass; // freq low pass
    int ddsAcc;    
    
    LowPass lowPassFilter;
};

#endif
