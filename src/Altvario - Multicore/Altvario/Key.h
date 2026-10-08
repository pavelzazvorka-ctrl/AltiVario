#include <Arduino.h>

#ifndef key_h
#define key_h

class Key {
  public:
    void poll();
    void done();
    void init(int _sndPin, int _modePin, int _selPin, int _setPin); 
    
    boolean isModeDown();
    boolean isSetDown();
    boolean isSelDown();

    boolean isModePressed();
    boolean isSetPressed();
    boolean isSelPressed();

    boolean isModeHold();
    boolean isSetHold();
    boolean isSelHold();

  private:  
    boolean lastModeDown;
    boolean lastSetDown;
    boolean lastSelDown;

    int click; 
    int sndPin;
    int modePin;
    int selPin;
    int setPin;
    int lastButtonState;
    int buttonState;
    int debounceDelay;
    int reading;
    unsigned long lastDebounceTime;
};

#endif

