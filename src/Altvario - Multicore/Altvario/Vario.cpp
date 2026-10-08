#include "Vario.h"
#include "Arduino.h"
#include "EEPROM.h" 

Vario::Vario() {
  msStep = 4; // 4ms default step
  cbInit(&circBuf,250); // 2000 ms buffer
  cbInit(&histBuf,80);
  cbInit(&altBuf,80);
  cbInit(&tempBuf,250);
  warmup=250; //faster run on init to fill buffers;
  Stopwatch=millis();
}

void Vario::init() {
  lastUpdate=millis();
  chartStep=0;
}

float  Vario::getZeroPressure()
{
  return atmosphere.getZeroPressure();
}

void  Vario::setZeroPressure(float zeropressure)
{
  atmosphere.setZeroPressure(zeropressure);
}

void  Vario::setZeroPressure(float actAltitude, float actPressure, float actTemperature)
{
  atmosphere.setZeroPressure(actAltitude, actPressure, actTemperature);
}

void Vario::update(float _pressure, float _temperature) 
{
  // circullar buffer 250 samples per 4 ms 

  // pressure filtering (FAST)
  kalmanFilter.update((double)_pressure);
  rawTemperature = _temperature;
  rawPressure = (float) kalmanFilter.value();  

  // Temperature filtering (SLOW)
  if (warmup-->0) {
      cbWrite(&tempBuf,rawTemperature);  
  } else {
      if (tempStep-- < 0)
      {
        tempStep=4; // update temperature just every 4-th run = 5 min rolling average filter
        cbWrite(&tempBuf,rawTemperature);  
      }
  }  
  Temperature = math.getAvg(&tempBuf);  
  rawAltitude = atmosphere.getAltitude(rawPressure, Temperature);
  
  Pressure = rawPressure;  
  Altitude = lowPassFilter.Filter((0.03+(0.02*Cfg.Sensitivity)),rawAltitude,10);    

  // calc Alt1,2
  Altitude2 = Altitude + offset2;    
  Altitude3 = Altitude + offset3;    

  // calc climb rate
  cbWrite(&circBuf,Altitude); 
  ClimbRate=(math.getSlope(&circBuf)); 

  // history trail
  chartStep--;
  if (chartStep<=0)
  {
      chartStep = Cfg.ChartSpeed;
      cbWrite(&histBuf,ClimbRate);
      cbWrite(&altBuf,Altitude);
  }
}

VarioCfg::VarioCfg()
{
    SinkThreshold  = +200; // low treshold for sound
    ClimbThreshold = +030; // high treshold for sound  
    ZeroThreshold = -80;
    DisrciminationThreshold = 25;    
    NetVario      = 15;
    Sensitivity   = 1;    // sensitivity
    BeepStyle     = 1;    // sound style
    Contrast      = 100;
    BT_protocol   = 0;
    ChartSpeed    = 125;
    Screens       = 255;
    stdpressure   = 101325;
}
