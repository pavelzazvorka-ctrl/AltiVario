#include "Math.h"

float Math::getGain(CircularBuffer *cb) {    
  // Get a diff of first and last non-nul value
  int n = cb->size-1; // !!
  float alt1=0;  
  float alt2=0;  
  for (int i=0; i<n; i++){
    float alt = cbIndex(cb,i);
    if (alt!=0 && alt < 3000 && alt>0 )
    {  
      if (alt1==0)
      {
        alt1=alt;
      }
      else
      {
        alt2=alt;
      }      
    }    
  }
  return alt2-alt1;
}

float Math::getMax(CircularBuffer *cb) {    
  int n = cb->size-1; // !!
  float vmax=-100000;  
  for (int i=0; i<n; i++){
    float v = cbIndex(cb,i);
    if (vmax<v) vmax=v;
  }  
  return vmax;
}

float Math::getMin(CircularBuffer *cb) {    
  int n = cb->size-1; // !!
  float vmin=100000;  
  for (int i=0; i<n; i++){
    float v = cbIndex(cb,i);
    if (vmin>v) vmin=v;
  }
  return vmin;
}

float Math::getTimeRange(CircularBuffer *cb) {  
  int n = cb->size-1; // !!
  unsigned long tsmin=4294967295 ;
  unsigned long tsmax=0;
  // Normalise time to avoid overflow 
  for (int i=0; i<n; i++){
    unsigned long ts = cbTstamp(cb,i);
    if (tsmin>ts) tsmin=ts;
    if (tsmax<ts) tsmax=ts;
  }
  return (tsmax-tsmin)/1000.00;
}

float Math::getAvg(CircularBuffer *cb) {  
  int n = cb->size-1; // !!
  float vs=0;  
  int valid=0;
  for (int i=0; i<n; i++){
    float v = cbIndex(cb,i);
    if (v!=0) {
        vs+=v;
        valid++;
    }
  }
  if (valid==0) return 0;
  return vs/valid;
}

float Math::getSlope(CircularBuffer *cb) {  
  int n = cb->size-1; // !!

  // Let's calculate slope on data in circ buffer
  // source McNeal WIKI https://wiki.mcneel.com/developer/scriptsamples/linearregression

  // initialize variables
  float sumx=0;
  float sumx2=0;
  float sumy=0;
  float sumy2=0;
  float sumxy=0;
  float sxx=0;
  float syy=0;
  float sxy=0;
  
  unsigned long tsmin=4294967295 ;
  unsigned long tsmax=0;

  // Normalise time to avoid overflow 
  for (int i=0; i<n; i++){
    unsigned long ts = cbTstamp(cb,i);
    if (tsmin>ts) tsmin=ts;
    if (tsmax<ts) tsmax=ts;
  }
        
  // linear regression
  for (int i=0; i<n; i++){
    float x = (float)(cbTstamp(cb,i)-tsmin);
    float y = cbIndex(cb,i)*1000.00; // let's count in mm / msec
    
    sumx  = sumx  + x;
    sumy  = sumy  + y;
    sumx2 = sumx2 + (x * x);
    sumy2 = sumy2 + (y * y);
    sumxy = sumxy + (x * y);
  }

  float slope=0;
  if (n>0)
  {
    sxx = sumx2 - (sumx * sumx / n);
    syy = sumy2 - (sumy * sumy / n);
    sxy = sumxy - (sumx * sumy / n);  
    if (sxx != 0) {
      slope = sxy / sxx;         
    }
  }  
  return slope;
}

LowPass::LowPass() {
  lowPass=0;
}

float LowPass::Filter(float gain, float val,int limit) 
{
  if (limit>0) {
    int jump = constrain((int)(10*abs(lowPass-val)),1,limit); 
    if (jump == limit) lowPass=val;
  }  
  lowPass = lowPass + (val - lowPass) * gain;
  return lowPass;
}

Kalman::Kalman () {
  q = 1;
  r = 1;
  p = 1;
  x = 330;
}

void Kalman::update(double measurement)
{
  //prediction update
  p = p + q;

  //measurement update
  k = p / (p + r);
  x = x + k * (measurement - x);
  p = (1 - k) * p;  
}

double Kalman::value()
{
  return x;
}
