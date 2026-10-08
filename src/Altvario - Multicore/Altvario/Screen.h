#ifndef screen_h
#define screen_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Vario.h"
#include "Key.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define S_VARIO1    0
#define S_VARIO2    1
#define S_VARIO3    2
#define S_VARIO4    3
#define S_STATUS    4
#define S_CHARGE    5

#define S_SETUP_1   7
#define S_SETUP_2   8
#define S_EDIT      9

#define MAX_SOUND 2
#define MAX_CONTRAST 16
#define MAX_BT_PROTOCOL 2
#define MAX_SENS 4
#define MAX_CHART_SPEED 4

// FONTS / GFX
#define FONT_MICRO u8g2_font_micro_tr
#define FONT_BIG   u8g2_font_helvB14_tn
//#define FONT_MONO  u8g2_font_victoriabold8_8r
//#define FONT_NOKIA u8g2_font_nokiafc22_tr
#define U8G2_PCD8544 U8G2_PCD8544_84X48_F_4W_HW_SPI

class Screen {
  public:

    void init(U8G2_PCD8544 *u8g); 

    void update(int mode, int section, Vario *vario, float Vcc, float ver); 

    // char only
    void show(const char *A1_label, char *A1_data, const char *A2_label, char *A2_data, const char *B_label, char *B_data);
    void show(const char *A1_label, char *A1_data, const char *A2_label, char *A2_data, const char *B1_label, char *B1_data, const char *B2_label, char *B2_data);

    // int/floats
    void show(const char *A1_label, int A1_data, const char *A2_label, int A2_data, const char *B_label, float B_data);
    void show(const char *A1_label, int A1_data, const char *A2_label, int A2_data, const char *B1_label, char *B1_data, const char *B2_label, float B2_data);

    void show(const char *a,char*b,const char*c,char*d); 
    void show(const char *a,float b,const char*c,float d); 
    void show(const char *a,int b,const char*c,float d); 
    
    void editShow(char *val, const char *title); 
    int editProcess(int *val, char *disp, int dflt, Key *key); 

    void setupCharSection ( boolean inv, int x, int y, int w, const char *label, char *dispval);
    void setupFltSection ( boolean inv, int x, int y, int w, const char *label, float val);
    void setupNumSection ( boolean inv, int x, int y, int w, const char *label, int val);
    void setupGaugeSection ( boolean inv, int x, int y, int w, const char *label, unsigned int val, unsigned int maxval);

    void showVario(const char *label,CircularBuffer *cb, float climb, int alt1, int alt2);     
    void setupCharShow( int sel, const char *A, char *A_data, const char *B, char *B_data, const char *C, char *C_data, const char *D, char *D_data); 
    void setupNumShow( int sel, const char *A, int A_data, const char *B, int B_data, const char *C, int C_data, const char *D, int D_data); 
    void setupGaugeShow(
                      int sel,
                      const char *A, unsigned int A_data, unsigned int A_max,
                      const char *B, unsigned int B_data, unsigned int B_max,
                      const char *C, unsigned int C_data, unsigned int C_max,
                      const char *D, unsigned int D_data, unsigned int D_max);

    void showPointer(int x, int y);
    void showChart(const char *label, CircularBuffer *cb, float climb, int alt1, int alt2);     
    void showChartFull(const char *label, CircularBuffer *cb, float climb, int alt1, int alt2);     

    void battery(int x,int y,bool charge, float Vcc);
    void arrowUp(int y,bool fill);
    void arrowDown(int y, bool fill);

  private: 

    Math          math;
    boolean       blink_char;
    unsigned long blink_time;
    int pos;
    //char editval[5]; 
    //char dispval[5]; 
        
    U8G2_PCD8544 *u8g;
};

#endif
