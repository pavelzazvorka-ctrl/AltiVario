#include "Key.h"
//#define SERIAL_MONITOR_KEYS

void Key::init(int _sndPin, int _modePin, int _selPin, int _setPin) 
{
  sndPin = _sndPin;  
  modePin = _modePin;
  selPin = _selPin;
  setPin = _setPin;
  lastButtonState=0;
  buttonState=0;
  debounceDelay=50;
  lastDebounceTime=0;  
  
  click = 1000;

  pinMode(sndPin, OUTPUT);  
  pinMode(modePin, INPUT_PULLUP);
  pinMode(selPin,  INPUT_PULLUP);
  pinMode(setPin,  INPUT_PULLUP);
}

void Key::poll()
{
  reading = (!digitalRead(modePin)*4+!digitalRead(selPin)*2+!digitalRead(setPin));
  if (reading != lastButtonState) {
     lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) 
  {     
     if (reading != buttonState) 
     {
        #ifdef SERIAL_MONITOR_KEYS
        Serial.print("buttonState:");    
        Serial.print(buttonState);    
        Serial.print(" lastDebounce:");    
        Serial.print(lastDebounceTime);    
        Serial.print(" millis:");    
        Serial.print(millis());    
        Serial.print(" debounceDelay:");    
        Serial.println((millis() - lastDebounceTime));            
        #endif
        buttonState = reading;          
     }   
  }
}

void Key::done()
{
  lastButtonState = reading;
}

boolean Key::isModePressed()
{
  if (isModeDown() && !lastModeDown) 
  {
    lastModeDown = true;
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("MODE PRESSED");    
    #endif
    return true;
  }
  return false;
}

boolean Key::isModeDown()
{
  if ((buttonState & 4) == 4) 
  {
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("MODE DOWN");    
    #endif
    return true;
  }
  else
  {
    lastModeDown=false;
    return false;
  }
}

boolean Key::isModeHold()
{
  if (((buttonState & 4) == 4) && ((millis()-lastDebounceTime)>click)) 
  {
    lastDebounceTime=millis();
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("MODE HOLD");    
    #endif
    return true;
  }
  return false;
}

boolean Key::isSelPressed()
{
  if (isSelDown() && !lastSelDown) 
  {
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("SEL PRESSED");    
    #endif
    lastSelDown = true;
    return true;
  }
  return false;
}

boolean Key::isSelDown()
{
  if ((buttonState & 2) == 2) 
  {
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("SEL DOWN");    
    #endif
    return true;
  }
  else
  {
    lastSelDown=false;
    return false;
  }
}

boolean Key::isSelHold()
{
  if (((buttonState & 2) == 2) && ((millis()-lastDebounceTime)>click)) {
    lastDebounceTime=millis();
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("SEL HOLD");    
    #endif
    return true;
  }
  return false;
}

boolean Key::isSetPressed()
{
  if (isSetDown() && !lastSetDown) 
  {
    lastSetDown = true;
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("SET PRESSED");    
    #endif
    return true;
  }
  return false;
}

boolean Key::isSetDown()
{
  if ((buttonState & 1) == 1) 
  {
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("SET DOWN");    
    #endif
    return true;
  }
  else
  {
    lastSetDown=false;
    return false;
  }
}

boolean Key::isSetHold()
{
  if (((buttonState & 1) == 1) && ((millis()-lastDebounceTime)>click)) {
    lastDebounceTime=millis();
    #ifdef SERIAL_MONITOR_KEYS
    Serial.println("SET HOLD");    
    #endif
    return true;
  }
  return false;
}
