#ifndef math_h
#define math_h

#include "CircularBuffer.h"

class Math {
	public:
            static float getSlope(CircularBuffer *cb);
            static float getGain(CircularBuffer *cb);
            static float getMax(CircularBuffer *cb);
            static float getMin(CircularBuffer *cb);
            static float getAvg(CircularBuffer *cb);
            static float getTimeRange(CircularBuffer *cb);
};

class Kalman {
  public:
    Kalman();
    void update(double measurement);
    double value();

  private:
    double q; //process noise covariance
    double r; //measurement noise covariance
    double x; //value
    double p; //estimation error covariance
    double k; //kalman gain
};

class LowPass {
  public:
    LowPass();
    float Filter(float gain, float val,int limit);

  private:
    float lowPass; //last data
};

#endif
