#include <Arduino.h>
#include "VarioAudio.h"
#include "config.h"

void VarioAudio::analogWrite(int pin, uint32_t value)
{
  ledcAttachPin(pin, audiochannel_);
  ledcWrite(audiochannel_, value);
}

void VarioAudio::analogWriteFreq(double freq)
{
  ledcWriteTone(audiochannel_, freq);
}

void VarioAudio::Config(int pinPWM, int audiochannel,VarioCfg *cfg) {  
  sinkToneCps_       	=  -(cfg->SinkThreshold);  // SINK_THRESHOLD -200
  climbToneCps_      	=  cfg->ClimbThreshold; // CLIMB_THRESHOLD 30
  liftyAirToneCps_   	=  cfg->ZeroThreshold;  // ZERO_THRESHOLD -80
  varioState_ 		    =  VARIO_STATE_QUIET;
  discrimThreshold_   =  cfg->DisrciminationThreshold; // CLIMB_DISCRIMINATION_THRESHOLD 25
  beepPeriodTicks_	  = 0;
  beepEndTick_ 		    = 0;
  beepCps_ 			      = 0;
  varioCps_ 			    = 0;
  freqHz_  			      = 0;
  pinPWM_ 			      = pinPWM;
  audiochannel_		    = audiochannel;
  analogWrite(pinPWM_, 0);
}

void VarioAudio::VarioBeep(int32_t nCps,int beepStyle) 
{
  // beepPeriodTicks_ 30 is full period
  // beepEndTick_ = 2;

  // There is running loop incrementing tick++ and decrementing beepPeriodTicks_--
  // Once beepEndTick>ticks is shut the tone off
  // beepPeriodTicks_ kick in the half ... 

  // Table for time slots is defined in VarioAudio.h
  // table for beep duration and repeat rate based on vertical speed (stale at 8 m/s)
  // BEEP beepTbl_[10] = {{13,4},{11,4},{9,4},{8,4},{7,4},{6,4},{5,3},{4,2},{3,1},{3,1},};  
  
  int32_t newFreqHz = 0;

  if (
    (beepPeriodTicks_ <= 0)
    ||   ((tick_ >= beepPeriodTicks_ / 2) && (ABS(nCps - varioCps_) > discrimThreshold_)) // check if we are in the middle of interval & and change bigger than 25
    ||   ((nCps >= climbToneCps_) && (varioCps_ < climbToneCps_))                         // check if we just broke the limit for ascend
    ||   (beepStyle>1) && ((nCps >= liftyAirToneCps_) && (varioCps_ < liftyAirToneCps_))  // or check if we just broke the limie for lifty air (if it is configured)
  ) 
  {
    varioCps_ = nCps;

    // if sinking much faster than glider sink rate, generate continuous tone alarm
    if (varioCps_ <= sinkToneCps_) {   // we are in the area where we should indicate sink
      varioState_ = VARIO_STATE_SINK;
      beepCps_ = varioCps_;
      if (beepCps_ < -VARIO_MAX_CPS) { // limit max sink tone (10 m/s)
        beepCps_ = -VARIO_MAX_CPS;
      }
      tick_ = 0;
      if (beepCps_ == -VARIO_MAX_CPS) { // if we went to the limit use a special tune
        beepPeriodTicks_ = 10;
        beepEndTick_ = 10;
        newFreqHz = offScaleLoTone_[0];
        freqHz_ = newFreqHz;
        SetFrequency(freqHz_);
      }
      else { // sinking, but still within limits
        beepPeriodTicks_ = 20;
        beepEndTick_  = 18;
        newFreqHz = VARIO_SINK_FREQHZ + ((sinkToneCps_ - beepCps_) * (VARIO_MAX_FREQHZ - VARIO_SINK_FREQHZ)) / (VARIO_MAX_CPS + sinkToneCps_); // calculation of sinking frequency
        //           base frequency                                       adjust the frequency range according to define range   
        CLAMP(newFreqHz, VARIO_MIN_FREQHZ, VARIO_MAX_FREQHZ); // restrict frequency by physical capabilities of the speaker
        freqHz_ = newFreqHz;
        SetFrequency(freqHz_);
      }
    }

    //if climbing, generate beeps
    else {

      if (varioCps_ >= climbToneCps_) {
        varioState_ = VARIO_STATE_CLIMB;
        beepCps_ = varioCps_;
        if (beepCps_ > VARIO_MAX_CPS) { // limit maximum lift to (10 m/s)
          beepCps_ = VARIO_MAX_CPS;
        }
        tick_ = 0;
        if (beepCps_ == VARIO_MAX_CPS) { // if we went over the limit, play a special tune
          beepPeriodTicks_ = 10;
          beepEndTick_ = 10;
          newFreqHz = offScaleHiTone_[0];
          freqHz_ = newFreqHz;
          SetFrequency(freqHz_);
        }
        else {

          // Time table / length lookup table to set beep timeframe
          int index = beepCps_ / 100;
          if (index > 9) index = 9;
          beepPeriodTicks_ = beepTbl_[index].periodTicks;
          beepEndTick_ = beepTbl_[index].endTick;
          
          if (beepCps_ > VARIO_XOVER_CPS) {
            newFreqHz = VARIO_XOVER_FREQHZ + ((beepCps_ - VARIO_XOVER_CPS) * (VARIO_MAX_FREQHZ - VARIO_XOVER_FREQHZ)) / (VARIO_MAX_CPS - VARIO_XOVER_CPS); // calculation for climb
          }
          else {
            newFreqHz = VARIO_MIN_FREQHZ + (beepCps_ * (VARIO_XOVER_FREQHZ - VARIO_MIN_FREQHZ)) / VARIO_XOVER_CPS; // calculation for lifty air
          }
          CLAMP(newFreqHz, VARIO_MIN_FREQHZ, VARIO_MAX_FREQHZ);
          freqHz_ = newFreqHz;
          SetFrequency(freqHz_);
        }
      }
      else   // in "lifty-air" band, indicate with a ticking sound with longer interval
        if ((beepStyle>1) && (varioCps_ >= liftyAirToneCps_)) 
        {
          varioState_ = VARIO_STATE_LIFTY_AIR;
          beepCps_ = varioCps_;
          tick_ = 0;
          beepPeriodTicks_ = 30;
          beepEndTick_ = 2;
          newFreqHz = VARIO_TICK_FREQHZ + (beepCps_ * (VARIO_XOVER_FREQHZ - VARIO_TICK_FREQHZ)) / VARIO_XOVER_CPS;
          CLAMP(newFreqHz, VARIO_TICK_FREQHZ, VARIO_MAX_FREQHZ);
          freqHz_ = newFreqHz;
          SetFrequency(freqHz_);  // higher frequency as you approach climb threshold          
        }

      // not sinking enough to trigger alarm,  be quiet
        else {
          varioState_ = VARIO_STATE_QUIET;
          tick_ = 0;
          beepPeriodTicks_ = 0;
          beepEndTick_  = 0;
          freqHz_ = 0;
          SetFrequency(freqHz_);
        }
    }
  }
  else {
    tick_++;
    beepPeriodTicks_--;
    newFreqHz = freqHz_;
    if (tick_ >= beepEndTick_) { // shut off tone
      newFreqHz = 0;
    }
    else if (beepCps_ == VARIO_MAX_CPS) {
      newFreqHz = offScaleHiTone_[tick_]; // offscale Hi table tone sequence as we reached the limit
    }
    else if (beepCps_ == -VARIO_MAX_CPS) {
      newFreqHz = offScaleLoTone_[tick_]; // offscale Lo table tone sequence as we reached the limit
    }
    else if (varioState_ == VARIO_STATE_SINK) {
      newFreqHz = freqHz_ - 10;
    }
    if (newFreqHz != freqHz_) { // update frequency when needed
      freqHz_ = newFreqHz;
      SetFrequency(freqHz_);
    }
  }
}

void VarioAudio::SetFrequency(int32_t fHz) {
  if (fHz ) {

    analogWriteFreq(fHz);
    analogWrite(pinPWM_, 512);
  }
  else {
    analogWrite(pinPWM_, 0);
  }
}

void VarioAudio::GenerateTone(int32_t fHz, int ms) {
  SetFrequency(fHz);
  delay(ms);
  SetFrequency(0);
}
