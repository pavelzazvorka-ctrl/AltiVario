#ifndef vario_h
#define vario_h

#include <Arduino.h>
#include "atmosphere.h"
#include "CircularBuffer.h"
#include "Math.h"

class VarioCfg {
  public:
    VarioCfg();

    // vario thresholds in cm/sec for generating different
    // audio tones. Between the sink threshold and the zero threshold,
    // the vario is quiet. netvario is the integration time is secs. max 15secs default 4secs

    int SinkThreshold;  // -090; // low treshold for sound
    int ClimbThreshold; // +010; // high treshold for sound  
    int ZeroThreshold;  // -80   // lifty air ZERO_THRESHOLD 
    int DisrciminationThreshold; // 25 CLIMB_DISCRIMINATION_THRESHOLD 
    int NetVario;

    unsigned int Sensitivity;     // 1;     // sensitivity
    unsigned int BeepStyle;       // 1;     // sound style
    unsigned int Contrast;
    unsigned int BT_protocol;     // 0=off
    unsigned int ChartSpeed;      // History chart speed
    unsigned int Screens;         // bitmask of active screens
    
    int offset2;
    int offset3;    
    unsigned int stdpressure; // 101325;
};

class Vario {
  public:
    Vario();

    void init(); 
    void update(float pressure, float temperature); // sensor input

    float Temperature;
    float Pressure;
    float Altitude;
    float Altitude2;
    float Altitude3;
    float ClimbRate;

    int offset2;
    int offset3; 
    
    float getZeroPressure();
    void setZeroPressure(float zeropressure);
    void setZeroPressure(float _altitude, float _pressure, float _temperature);
   
    VarioCfg Cfg;
    float rawAltitude;    
    CircularBuffer histBuf;
    CircularBuffer altBuf;
    unsigned long Stopwatch;

  private:
    int tempStep;
    int warmup;
    int chartStep;
    CircularBuffer circBuf;
    CircularBuffer tempBuf;
    unsigned long lastUpdate;
    float rawTemperature;
    float rawPressure;
    
    int msStep; // 20ms default step

    Kalman         kalmanFilter;
    LowPass        lowPassFilter;
    Atmosphere     atmosphere;
    Math           math;
    
};

#endif
