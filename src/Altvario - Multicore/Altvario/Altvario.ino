#define SERIAL_MONITOR
#define SERIAL_MONITOR_KEYS
#define SERIAL_MONITOR_CFG
#define SERIAL_MONITOR_BT

#define SOUND_ON
#define BARO_ON // otherwise EMU
#define DISPLAY_ON 
#define BT_ON // BT is quite an expensive >500 kB flash / 70 mA

#include <Wire.h>
#include "pitch.h"
#include "Vario.h"
#include "Sound.h"
#include "Screen.h"
#include "Key.h"
#include <EEPROM.h>
#include <esp32-hal-cpu.h>
#include <esp32-hal-bt.h>

#ifdef BT_ON
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#endif

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#ifdef DISPLAY_ON
#include <U8g2lib.h>
#endif

#ifdef BARO_ON
#include <MS561101BA.h>
static MS561101BA baro = MS561101BA();
#endif

// Vario version
const float ver = 0.61;

const int8_t CE_PIN   = U8X8_PIN_NONE;
const byte ledPin     = U8X8_PIN_NONE;

// ESP32 MH-ET Live
//-------------------------------------------------------------------------
const int8_t RST_PIN  = 05;
const int8_t DC_PIN   = 26;
const int8_t DIN_PIN  = 23;
const int8_t CLK_PIN  = 18;
const int8_t ADC_PIN  = 35;
// I2C SDA (21)
// I2C SCL (22)
const byte sndPin     = 02; // speaker PIN / onboard LED
const byte buttonSet  = 19; // Button #1
const byte buttonSel  = 17; // Button #2
const byte buttonMode = 16; // Button #3

#ifdef DISPLAY_ON
static U8G2_PCD8544 u8g2(U8G2_R0,CE_PIN,DC_PIN,RST_PIN);
#endif

const byte DISPLAY_WIDTH = 84;
const byte DISPLAY_HEIGHT = 48;

// Global classes
static Vario vario;
static Sound sound;
static Screen screen;
static Key key;

// BT
#ifdef BT_ON
BluetoothSerial SerialBT;
#endif

// Global vars
static unsigned long _time = 0;  // timing
static unsigned long _time_0 = 0;  // timing
static unsigned long _time_1 = 0;  // timing
static int dcnt=0;               // Display step
static int bcnt=0;               // BT step
static int scnt=0;               // SND step
//static bool baro_on = true;    // MS5611 on board
static int mode = 0;             // mode of vario (see screen.h)
static int section=0;            // section of vario ( quarters of setup screen )
static int EditorTmpVal=0;       // Temp. value for editor
static char DispVal[5];          // Disp. buffer for editor

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t TaskUI;
TaskHandle_t TaskSensor;

void readingData()
{
  float rBaro = 1013;
  float rTemp = 20;  

  // Reading of MS5611
  #ifdef BARO_ON
      rTemp = baro.getTemperature(MS561101BA_OSR_4096);  
      rBaro = baro.getPressure(MS561101BA_OSR_4096);      
  #else    
      rBaro = 900 + (0.75 * sin(millis()/5000.00)) + (0.50 * sin(40+millis()/3000.00));
      rTemp = 23.12;
  #endif

  if (rBaro>0) {
    vario.update(rBaro,rTemp);  
    if (scnt-- < 0) {    
        scnt=4;
        #ifdef SOUND_ON
        sound.update(vario.ClimbRate);
        #endif
    }
  }
}

volatile bool get_data;

void loopSensor(void * parameter)
 {
  for(;;)
  {
    if ( get_data ) {
        portENTER_CRITICAL(&timerMux);
        get_data = false;
        portEXIT_CRITICAL(&timerMux);
        readingData();             
        vTaskSuspend(NULL);
    }
  }
}
 
void IRAM_ATTR onTime() {
    portENTER_CRITICAL_ISR(&timerMux);
    get_data = true;
    vTaskResume(TaskSensor);
    portEXIT_CRITICAL_ISR(&timerMux);
}

void loop() 
{
}

void loopUI(void * parameter) 
{    
  for(;;)
{
  // readingData();
  
  #ifdef DISPLAY_ON

  key.poll();        
         
  // process keys on interactive pages    
  switch (mode)
  { 
    case S_SETUP_1:
        if (key.isModeHold()) { // exit
           // save & exit
           mode=0;
        }
        
        if (key.isModePressed()) {
           section=0;
           mode=S_SETUP_2;    
        }
        
        if (key.isSelPressed()) {
           section++;
           if (section>3) section=0;    
        }
        
        if (key.isSetPressed()) {
           mode=S_EDIT;
           switch (section) {
           case 0:
                EditorTmpVal = vario.getZeroPressure();
                break;
           case 1:     
                EditorTmpVal = (int)vario.Altitude;
                break;
           case 2:     
                EditorTmpVal = (int)(vario.Cfg.SinkThreshold);
                break;
           case 3:     
                EditorTmpVal = (int)(vario.Cfg.ClimbThreshold);
                break;             
           }
        }
        break;
        
    case S_SETUP_2:
        if (key.isModeHold()) { // exit
           // save & exit
           mode=0;
        }
        if (key.isModePressed()) {
           section=0;
           mode=S_SETUP_1;    
        }
        
        if (key.isSelPressed()) {
           section++;
           if (section>3) section=0;    
        }
        
        if (key.isSetPressed()) {
           switch (section) {
           case 0:  
                vario.Cfg.BeepStyle++; 
                if (vario.Cfg.BeepStyle>MAX_SOUND) vario.Cfg.BeepStyle=0;
                sound.setSoundStyle(vario.Cfg.BeepStyle);
                writeCfg(vario.Cfg);
                break;
           case 1:     
                vario.Cfg.Contrast++;
               if (vario.Cfg.Contrast>MAX_CONTRAST) vario.Cfg.Contrast=0;
                u8g2.setContrast(vario.Cfg.Contrast*2+115);
                writeCfg(vario.Cfg);
                break;
           case 2:     
                vario.Cfg.BT_protocol++;
                if (vario.Cfg.BT_protocol>MAX_BT_PROTOCOL) vario.Cfg.BT_protocol=0;
                writeCfg(vario.Cfg);
                break;
           case 3:     
                vario.Cfg.Sensitivity++;
                if (vario.Cfg.Sensitivity>MAX_SENS) vario.Cfg.Sensitivity=1;
                writeCfg(vario.Cfg);
                break;             
           }
        }            
        break;
        
    case S_EDIT:
       if (screen.editProcess(&EditorTmpVal,(char *)DispVal,0,&key)) {
          // done && save
          switch (section) {
          case 0:  
                vario.setZeroPressure(EditorTmpVal);
                vario.Cfg.stdpressure = (unsigned int) (vario.getZeroPressure()*100);
                break;
          case 1:     
                vario.setZeroPressure(EditorTmpVal,vario.Pressure,vario.Temperature);
                vario.Cfg.stdpressure = (unsigned int) (vario.getZeroPressure()*100);
                break;
          case 2:     
                vario.Cfg.SinkThreshold=EditorTmpVal;
                sound.setSinkThreshold(vario.Cfg.SinkThreshold);
                break;
          case 3:     
                vario.Cfg.ClimbThreshold=EditorTmpVal;
                sound.setClimbThreshold(vario.Cfg.ClimbThreshold);
                break;             
          }
          mode = S_SETUP_1;
          writeCfg(vario.Cfg);
        }            
        break;
     }

     if (dcnt--<0)
     {        
      dcnt=5; // 200 ms 5x/sec   
      // display    
      unsigned int vcc = readVcc();
      u8g2.firstPage();  
      do {  
        switch (mode)
        {        
          case S_VARIO1:          
          case S_VARIO2:
          case S_VARIO3:
          case S_VARIO4:
          case S_STATUS:
          case S_CHARGE:
              screen.update(mode,section,&vario,vcc/1000.00,ver);
              break;        
          case S_SETUP_1:
          case S_SETUP_2:
              screen.update(mode,section,&vario,vcc/1000.00,ver);
              break;        
          case S_EDIT:
              switch (section) {
              case 0:
                    screen.editShow(DispVal,"Ref. Pressure [hPa]");
                    break;
              case 1:     
                    screen.editShow(DispVal,"Ref. Altitude [m]");
                    break;
              case 2:     
                    screen.editShow(DispVal,"Sink Threshold [cm]");
                    break;
              case 3:     
                    screen.editShow(DispVal,"Climb Threshold [cm]");
                    break;             
              }
              break;
        }
      } while( u8g2.nextPage() );     
     }

  #endif
     
  #ifdef SERIAL_MONITOR
  if (millis() < _time) {
    //Serial.print(" CLIMB "); 
    //Serial.print(vario.ClimbRate);         
    Serial.print(" ,rawALT "); 
    Serial.print(vario.rawAltitude);         
    Serial.print(" ,ALT "); 
    Serial.println(vario.Altitude);             
  }
  #endif  

  #ifdef BT_ON
  if (bcnt-- < 0 && vario.Cfg.BT_protocol>0) {

    if (vario.Cfg.BT_protocol>0)
    {
      
      // LK8000 EXTERNAL INSTRUMENT SERIES 1 - NMEA SENTENCE: LK8EX1
      // VERSION A, 110217

      // LK8EX1,pressure,altitude,vario,temperature,battery,*checksum

      // Field 0, raw pressure in hPascal:
      //  hPA*100 (example for 1013.25 becomes  101325) 
      //  no padding (987.25 becomes 98725, NOT 098725)
      //  If no pressure available, send 999999 (6 times 9)
      //  If pressure is available, field 1 altitude will be ignored     

      // Field 1, altitude in meters, relative to QNH 1013.25
      //  If raw pressure is available, this value will be IGNORED (you can set it to 99999
      //  but not really needed)!
      //  (if you want to use this value, set raw pressure to 999999)
      //  This value is relative to sea level (QNE). We are assuming that
      //  currently at 0m altitude pressure is standard 1013.25.
      //  If you cannot send raw altitude, then send what you have but then
      //  you must NOT adjust it from Basic Setting in LK.
      //  Altitude can be negative
      //  If altitude not available, and Pressure not available, set Altitude
      //  to 99999  (5 times 9)
      //  LK will say "Baro altitude available" if one of fields 0 and 1 is available.

      // Field 2, vario in cm/s
      //  If vario not available, send 9999  (4 times 9)
      //  Value can also be negative

      // Field 3, temperature in C , can be also negative
      //  If not available, send 99
      
      // Field 4, battery voltage or charge percentage
      //  Cannot be negative
      //  If not available, send 999 (3 times 9)
      //  Voltage is sent as float value like: 0.1 1.4 2.3  11.2 
      //  To send percentage, add 1000. Example 0% = 1000
      //  14% = 1014 .  Do not send float values for percentages.
      //  Percentage should be 0 to 100, with no decimals, added by 1000!
      
      bcnt=5; // 250 ms loop (4x sec)

      char tbuf[80];
      unsigned int checksum, ai, bi;                                                     
      sprintf(tbuf,"LK8EX1,%d,%d,%d,%d,%.2f,",(int)(vario.Pressure*100.00),(int)(vario.Altitude),(int)(vario.ClimbRate*100.00),(int)vario.Temperature,readVcc()/1000.00);
      int len = strnlen(tbuf,80);
      for (checksum = 0, ai = 0; ai < len; ai++)
      {
        bi = (unsigned char)tbuf[ai];
        checksum ^= bi;
      }
      sprintf(tbuf,"$LK8EX1,%d,%d,%d,%d,%.2f,*%x",(int)(vario.Pressure*100.00),(int)(vario.Altitude),(int)(vario.ClimbRate*100.00),(int)vario.Temperature,(readVcc()/1000.00),checksum);
      
      #ifdef SERIAL_MONITOR_BT
        Serial.println(tbuf);
      #endif
      SerialBT.println(tbuf);
    }
  }
  #endif

  // keys / loop

  #ifdef DISPLAY_ON
  while (millis() < _time) {
    key.poll();    
    switch (mode) {    
    case S_EDIT:    // leave key handling on SCREEN when S_EDIT
    case S_SETUP_1: // leave key handling on SCREEN when S_EDIT
    case S_SETUP_2: // leave key handling on SCREEN when S_EDIT
    case S_CHARGE:  // Charging - no way out    
        break;
    case S_VARIO1:
    case S_VARIO2: 
    case S_VARIO3:     
    case S_VARIO4:
       if ( key.isSetHold()) {
           #ifdef SERIAL_MONITOR_KEYS    
           Serial.println("AltX RESET");  
           #endif        
           if (mode==S_VARIO1) { vario.offset2=-vario.Altitude; }; // reset Alt2 on first page       
           if (mode==S_VARIO2) { vario.offset2=-vario.Altitude; }; // reset Alt2 on first page       
           if (mode==S_VARIO3) { vario.offset2=-vario.Altitude; }; // reset Alt2
           if (mode==S_VARIO4) { vario.offset3=-vario.Altitude; }; // reset Alt3
        }         
       if ( key.isSelHold()) {
           #ifdef SERIAL_MONITOR_KEYS    
           Serial.println("Stopwatch RESET");  
           #endif        
           if (mode==S_VARIO3) { vario.Stopwatch=millis(); } // reset Stopwatch
       }            
        // pass through def.
    case S_STATUS:
       if ( key.isSetHold()) {
           #ifdef SERIAL_MONITOR_KEYS    
              Serial.println("CHARGE");  
           #endif        
           mode=S_CHARGE; //S_SHOW_BARO;        
           chargingModeOn();
        }             
    default:    
        if (key.isModePressed()) {
           mode++;
           #ifdef SERIAL_MONITOR_KEYS    
           Serial.print("MODE++ ");  
           Serial.println(mode);  
           #endif        
           if (mode>S_STATUS) mode=0;    
        }
        if ( key.isModeHold()) {
           #ifdef SERIAL_MONITOR_KEYS    
           Serial.println("SETUP");  
           #endif        
           mode=S_SETUP_1; //S_SHOW_BARO;
        }
     }
     if (millis() < (_time-10)) {
        delay(10);
     }
  }
  key.done();      
  #endif

  _time += 50; // 50 ms loop (20x sec)
    
}
}

void chargingModeOn()
{
  Serial.print("Charging mode");   

  vario.Cfg.BeepStyle=0;
  sound.setSoundStyle(0);

  Serial.println(" Sound OFF / BT OFF");
  vario.Cfg.BT_protocol=0;
  #ifdef BT_ON
  Serial.println(" Disable BT");
  SerialBT.flush();  
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println(" Disabled BT");   
  #endif
  
  Serial.print("CPU frequency:");   
  //function takes the following frequencies as valid values:
  //  240, 160, 80    <<< For all XTAL types 
  //  40, 20, 10      <<< For 40MHz XTAL
  //  26, 13          <<< For 26MHz XTAL
  //  24, 12          <<< For 24MHz XTAL
  bool cpu = setCpuFrequencyMhz(10);

  int cpuf    = getCpuFrequencyMhz();  // In MHz
  Serial.print(cpuf);   
  Serial.print(" MHz XTAL: ");   
  int xtal = getXtalFrequencyMhz();
  Serial.print(xtal);   
  Serial.println(" MHz");   

  Serial.println(" Running low power");  
}

void setup() 
{ 
  Serial.begin(115200); 
  float rBaro = 900.35;
  float rTemp = 23.12; 

  Serial.print("CPU frequency:");   
  //function takes the following frequencies as valid values:
  //  240, 160, 80    <<< For all XTAL types 
  //  40, 20, 10      <<< For 40MHz XTAL
  //  26, 13          <<< For 26MHz XTAL
  //  24, 12          <<< For 24MHz XTAL
  bool cpu = setCpuFrequencyMhz(80);

  int cpuf    = getCpuFrequencyMhz();  // In MHz
  Serial.print(cpuf);   
  Serial.print(" MHz XTAL: ");   
  int xtal = getXtalFrequencyMhz();
  Serial.print(xtal);   
  Serial.println(" MHz");   

  // Init
  vario.init();
  EEPROM.begin(64);
  readCfg(&(vario.Cfg));  
  Serial.print("Pressure:");  
  vario.setZeroPressure(vario.Cfg.stdpressure/100.00);
  Serial.println(vario.getZeroPressure());
  
  #ifdef BT_ON
  if (vario.Cfg.BT_protocol>0) {
    Serial.println("BT on");
    SerialBT.begin("BT AltiVario"); //Bluetooth device name        
  }
  else
  {
    Serial.println("BT off");
  }
  #else
  Serial.println("BT disabled");
  #endif

  Serial.println("Vario setup");   
  Wire.begin();
  Wire.setClock(400000); // 10000 low speed // 100000 standard // 400000 fast mode
  
  #ifdef BARO_ON
    baro.init(MS561101BA_ADDR_CSB_LOW); 
  #endif  
  
  sound.init(sndPin,&(vario.Cfg));
  
  #ifdef DISPLAY_ON
  
    screen.init(&u8g2);
    key.init(sndPin,buttonMode,buttonSel,buttonSet);
  
    // GFX init
    u8g2.begin();
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosBaseline(); // Top
    u8g2.setFontDirection(0);
    
    u8g2.setDrawColor(1);         // pixel on
    u8g2.setContrast(vario.Cfg.Contrast*2+115);
  #endif

  hwsetup();  

  // delay and buffers initial update update  
  delay(1000);
  #ifdef BARO_ON
      rTemp = baro.getTemperature(MS561101BA_OSR_4096);  
      rBaro = baro.getPressure(MS561101BA_OSR_4096);      
  #endif
  for (int i=0;i<100;i++)
  {  
    if (rBaro>0) {vario.update(rBaro,rTemp);    
    } 
  }

  _time=millis();
  _time_0=millis();
  
  xTaskCreatePinnedToCore(loopSensor, "loopSensor", 10000, NULL, 1, &TaskSensor,  1);   
  xTaskCreatePinnedToCore(loopUI, "loopUI", 10000, NULL, 1, &TaskUI,  0); 

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTime, true);
  timerAlarmWrite(timer, 4000, true); // 4ms loop
  timerAlarmEnable(timer);
}

// **********************************************************************************

long readVcc() {
  long result = 4150; // system_get_vdd33();
  result = analogRead(ADC_PIN)*1.71;  
  return result;
}

void DrawWelcomeScreen(float Vcc, float Ver,int stp) 
{
#ifdef DISPLAY_ON  
  char vcc[10];
  dtostrf(Vcc,1,2,vcc);

  char ver[10];
  dtostrf(Ver,1,2,ver);

  u8g2.setFont(FONT_MICRO);
  u8g2.drawBox(0, 26, 84, 12);
  u8g2.setDrawColor(0);
  u8g2.drawStr( 13, 35, "Altimeter/Vario");
  u8g2.setDrawColor(1);
  u8g2.drawStr( stp*2-36, 48, "MS5611");
  
  u8g2.drawHLine(48-stp, 39, 84);  
  u8g2.drawHLine(0, 24, stp+36);  

  screen.setupCharSection(false, 0, 0,42,"SW",ver);
  screen.setupCharSection(false,44, 0,42,"Batt.",vcc);
#endif  
}

int batNote[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4};
int batLevel[] = {3500, 3750, 3900, 4050, 4200};

void hwsetup() {
  
  //if (ledPin != U8X8_PIN_NONE)
  //{
  //pinMode(ledPin, OUTPUT);
  //analogWrite(ledPin,255);      
  //}
    
  int vcc = readVcc();

  Serial.print("Vcc:");
  Serial.print(vcc);
  Serial.println(" mV");
  
  // Voltage soundcheck
  for (int i = 0; i < 5; i++) {    
    if (vcc > batLevel[i]) {
      #ifdef SOUND_ON
      tone(sndPin, batNote[i], 150,0);
      #endif
    } else {
      #ifdef SOUND_ON
      tone(sndPin, batNote[i], 50,0); 
      #endif
    }
    for (int ii = 0; ii < 5; ii++) {    
#ifdef DISPLAY_ON        
    u8g2.firstPage();  
    do {
      DrawWelcomeScreen(vcc/1000.00,ver,i*10+ii*2); 
    } while( u8g2.nextPage() );
#else
    delay(300);
#endif    
    }
  }  
  delay(1000);
}

// EEPROM somehow cant be located in VarioCfg

void writeCfg( VarioCfg cfg )
{
    #ifdef SERIAL_MONITOR_CFG
    Serial.println("cfg write");    
    Serial.print("cfg.SinkThreshold ");
    Serial.println(cfg.SinkThreshold);
    Serial.print("cfg.ClimbThreshold ");
    Serial.println(cfg.ClimbThreshold);
    Serial.print("cfg.Sensitivity ");
    Serial.println(cfg.Sensitivity);    
    Serial.print("cfg.BeepStyle ");
    Serial.println(cfg.BeepStyle);
    Serial.print("cfg.Contrast ");
    Serial.println(cfg.Contrast);
    Serial.print("cfg.BT_protocol ");
    Serial.println(cfg.BT_protocol);
    Serial.print("cfg.stdpressure ");
    Serial.println(cfg.stdpressure);
    // Serial.print("cfg.ChartSpeed ");
    // Serial.println(cfg.ChartSpeed);    
    #endif        

    int val  = 0;
    val = (unsigned int)EEPROM.read(0); if (val != cfg.SinkThreshold)  EEPROM.write(0,(unsigned byte)(cfg.SinkThreshold));   
    val = (unsigned int)EEPROM.read(1); if (val != cfg.ClimbThreshold) EEPROM.write(1,(unsigned byte)(cfg.ClimbThreshold));    
    val = (unsigned int)EEPROM.read(2); if ((unsigned int)val != cfg.Sensitivity)   EEPROM.write(2,(unsigned byte)(cfg.Sensitivity));    
    val = (unsigned int)EEPROM.read(3); if ((unsigned int)val != cfg.BeepStyle)     EEPROM.write(3,(unsigned byte)(cfg.BeepStyle));
    val = (unsigned int)EEPROM.read(4); if ((unsigned int)val != cfg.Contrast)      EEPROM.write(4,(unsigned byte)(cfg.Contrast));    
    val = (unsigned int)EEPROM.read(5); if ((unsigned int)val != cfg.BT_protocol)   EEPROM.write(5,(unsigned byte)(cfg.BT_protocol));
    //val = (unsigned int)EEPROM.read(6); if ((unsigned int)val != cfg.ChartSpeed)    EEPROM.write(6,(unsigned byte)(cfg.ChartSpeed));

    // stdpressure mutlibyte
    byte four = (cfg.stdpressure & 0xFF);
    byte three = ((cfg.stdpressure >> 8) & 0xFF);
    byte two = ((cfg.stdpressure >> 16) & 0xFF);
    byte one = ((cfg.stdpressure >> 24) & 0xFF);
    EEPROM.write(7, four);
    EEPROM.write(8, three);
    EEPROM.write(9, two);
    EEPROM.write(10, one);
    
    EEPROM.commit();
}

void readCfg(VarioCfg *cfg) {  
    cfg->SinkThreshold  = constrain((unsigned int)EEPROM.read(0),(unsigned int)0,(unsigned int)200);
    cfg->ClimbThreshold = constrain((unsigned int)EEPROM.read(1),(unsigned int)0,(unsigned int)050);      
    cfg->Sensitivity   = constrain((unsigned int)EEPROM.read(2),(unsigned int)1,(unsigned int)MAX_SENS);
    cfg->BeepStyle     = constrain((unsigned int)EEPROM.read(3),(unsigned int)0,(unsigned int)MAX_SOUND);
    cfg->Contrast      = constrain((unsigned int)EEPROM.read(4),(unsigned int)0,(unsigned int)MAX_CONTRAST);    
    cfg->BT_protocol   = constrain((unsigned int)EEPROM.read(5),(unsigned int)0,(unsigned int)MAX_BT_PROTOCOL);    
    //cfg->ChartSpeed    = constrain((unsigned int)EEPROM.read(6),(unsigned int)0,(unsigned int)MAX_CHARTSPEED);    

    unsigned int four = EEPROM.read(7);
    unsigned int three = EEPROM.read(8);
    unsigned int two = EEPROM.read(9);
    unsigned int one = EEPROM.read(10);
    cfg->stdpressure      = constrain(((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF),(unsigned int)80000,(unsigned int)120001);    

    if (cfg->stdpressure < 80001) cfg->stdpressure = 101325;
    if (cfg->stdpressure > 120000) cfg->stdpressure = 101325;
    
    #ifdef SERIAL_MONITOR_CFG
    Serial.println("cfg read");
    Serial.print("cfg.SinkThreshold ");
    Serial.println(cfg->SinkThreshold);
    Serial.print("cfg.ClimbThreshold ");
    Serial.println(cfg->ClimbThreshold);
    Serial.print("cfg.Sensitivity ");
    Serial.println(cfg->Sensitivity);    
    Serial.print("cfg.BeepStyle ");
    Serial.println(cfg->BeepStyle);
    Serial.print("cfg.Contrast ");
    
    Serial.println(cfg->Contrast);
    Serial.print("cfg.BT_protocol ");
    Serial.println(cfg->BT_protocol);
    Serial.print("cfg.stdpressure ");
    Serial.println(cfg->stdpressure);
    // Serial.print("cfg.ChartSpeed ");
    // Serial.println(cfg->ChartSpeed);    

    
    #endif    
}
