#include "Screen.h"

void Screen::init(U8G2_PCD8544 *_u8g)
{
  u8g = _u8g;
  blink_char=false;
  blink_time=millis();
  pos = 0;
  u8g->setDisplayRotation(U8G2_R2); // rotate 180
}

int Screen::editProcess(int *val, char *dispval, int dflt, Key *key) 
{
  char editval[5];
  
  // fetch value to int. buffer incl. default when hold
  if (key->isSetHold()) {
      dtostrf(dflt,4,0,editval);
  } else {
      dtostrf(*val,4,0,editval);
  }

  for (int i=0;i<3;i++) { // leading zeroes
    if (editval[i]==' ') editval[i]='0';
  }
  editval[4]=0;
  constrain(pos,0,3);
  
  // select position
  if (key->isSetPressed()) {
      pos++;
      if (pos>3) pos=0;
  }

  // edit char at position
  if (key->isSelPressed()) {
      char c = (char)editval[pos];
      c++;
      if (c > '9') {
        c=(char)'0';
      };
      editval[pos] = (char)c;
  }

  // edit char at position
  if (key->isSelPressed()) {
      char c = (char)editval[pos];
      c++;
      if (c > '9') {
        c=(char)'0';
      };
      editval[pos] = (char)c;
  }
  
  // display buffer handling / blinking
  dispval[4]=0;
  for (int i=0;i<4;i++) dispval[i]=editval[i]; // incl last '\0';
  if ((millis() - blink_time) > 40) {
    blink_time=millis();
    if (blink_char) {
       blink_char=false;
       dispval[pos]='-';
    }
    else
    {
       blink_char=true;
    }
  }
  
  // editval back to val
  *val = atoi(editval);  
  
  // edit char at position
  return (key->isModePressed());
}

void Screen::editShow(char *dispval, const char *title) 
{  
  u8g->setFont(FONT_MICRO);
  u8g->drawStr( 0, 5, title);
  u8g->setFont(FONT_BIG);
  int w1 = u8g->getStrWidth(dispval);
  u8g->drawStr( 30, 20, dispval);
  
  u8g->drawHLine(0, 38, 84);  
  u8g->setFont(FONT_MICRO);  
  u8g->drawStr( 0, 46, "MODE to exit");  
}

void Screen::setupCharSection ( boolean inv, int x, int y, int w, const char *title, char *dispval)
{
  if (inv) {
    u8g->drawBox(x, y, w, 23);
    u8g->setDrawColor(0);
  }
  u8g->setFont(FONT_MICRO);
  u8g->drawStr( x+2, y+6, title);
  u8g->setFont(FONT_BIG);
  int wl = u8g->getStrWidth(dispval);
  u8g->drawStr( x+w-wl-3, y+21, dispval);
  u8g->setDrawColor(1);
}

void Screen::setupNumSection ( boolean inv, int x, int y, int w, const char *label, int val)
{
  char A[10];
  dtostrf(val,1,0,A); 
  setupCharSection ( inv, x, y, w, label, A);
}

void Screen::setupGaugeSection ( boolean inv, int x, int y, int w, const char *title, unsigned int val, unsigned int maxval)
{ 
  if (inv) { // selected
    u8g->drawBox(x, y, w-2, 23);    
    u8g->setDrawColor(0);
    u8g->drawBox(x+4, y+10, 34, 10);  
  }
  u8g->setFont(FONT_MICRO);
  u8g->drawStr( x+2, y+6, title);

  //u8g->setDefaultForegroundColor();
  u8g->setDrawColor(1);
  u8g->drawFrame(x+4, y+10, 34, 10);  
  
  if (val>0) {
    unsigned int wx = 30.00*val/maxval;
    u8g->drawBox(x+6, y+12, wx, 6);
    //u8g->setDefaultBackgroundColor();    
    u8g->setDrawColor(0);
    u8g->drawHLine(x+7, y+13, wx-2);  
    //u8g->setDefaultForegroundColor();        
    u8g->setDrawColor(1);
  }
}

void Screen::setupNumShow(
  int sel,
  const char *A, int A_data,
  const char *B, int B_data,
  const char *C, int C_data,
  const char *D, int D_data)
  {
    setupNumSection((sel==0), 0, 0,42,A,A_data);
    setupNumSection((sel==1),43, 0,42,B,B_data);
    setupNumSection((sel==2), 0,26,42,C,C_data);
    setupNumSection((sel==3),43,26,42,D,D_data);
    
    u8g->drawHLine(0, 24, 40);
    u8g->drawHLine(43,24, 46);
    u8g->drawVLine(41, 0, 23);
    u8g->drawVLine(41,26, 23);
  }

void Screen::setupGaugeShow(
  int sel,
  const char *A, unsigned int A_data, unsigned int A_max,
  const char *B, unsigned int B_data, unsigned int B_max,
  const char *C, unsigned int C_data, unsigned int C_max,
  const char *D, unsigned int D_data, unsigned int D_max)
  {
    setupGaugeSection((sel==0), 0, 0,42,A,A_data,A_max);
    setupGaugeSection((sel==1),43, 0,42,B,B_data,B_max);
    setupGaugeSection((sel==2), 0,26,42,C,C_data,C_max);
    setupGaugeSection((sel==3),43,26,42,D,D_data,D_max);
    
    u8g->drawHLine(0, 24, 40);
    u8g->drawHLine(43,24, 46);
    u8g->drawVLine(41, 0, 23);
    u8g->drawVLine(41,26, 23);
  }

void Screen::show(const char *A_label, char *A_data, const char *B_label, char *B_data) 
{
  u8g->setFont(FONT_MICRO);
  u8g->drawStr( 0, 5, A_label);
  u8g->setFont(FONT_BIG);
  int w1 = u8g->getStrWidth(A_data);
  u8g->drawStr( 80-w1, 20, A_data);
  
  u8g->drawHLine(0, 24, 84);
  
  u8g->setFont(FONT_MICRO);  
  u8g->drawStr( 0, 32, B_label);
  u8g->setFont(FONT_BIG);
  int w2 = u8g->getStrWidth(B_data);
  u8g->drawStr( 80-w2, 48, B_data);
}

void Screen::show(const char *A1_label, char *A1_data, const char *A2_label, char *A2_data, const char *B_label, char *B_data) 
{
  setupCharSection(false, 0, 0,42,A1_label,A1_data);
  setupCharSection(false,44, 0,42,A2_label,A2_data);

  u8g->drawHLine(0, 24, 84);
  
  u8g->setFont(FONT_MICRO);  
  u8g->drawStr( 0, 32, B_label);
  u8g->setFont(FONT_BIG);
  int w2 = u8g->getStrWidth(B_data);
  u8g->drawStr( 80-w2, 48, B_data);
}

void Screen::show(const char *A1_label, char *A1_data, const char *A2_label, char *A2_data, const char *B1_label, char *B1_data, const char *B2_label, char *B2_data) 
{
  setupCharSection(false, 0,  0,42,A1_label,A1_data);
  setupCharSection(false,44,  0,42,A2_label,A2_data);
  setupCharSection(false, 0, 24,42,B1_label,B1_data);
  setupCharSection(false,44, 23,42,B2_label,B2_data);
  
  u8g->drawHLine(0, 24, 84);
}

void Screen::show(const char *A_label, float A_data, const char *B_label, float B_data) 
{
  char A[10];
  char B[10];
  dtostrf(A_data,1,2,A);
  dtostrf(B_data,1,2,B);
  show(A_label,A,B_label,B);
}

// Watch
void Screen::show(const char *A1_label, int A1_data, const char *A2_label, int A2_data, const char *B_label, float B_data) 
{
  char A1[10];
  char A2[10];
  char B[10];
  dtostrf(A1_data,1,0,A1);
  dtostrf(A2_data,1,0,A2);
  dtostrf(B_data,1,2,B);
}

void Screen::show(const char *A_label, int A_data, const char *B_label, float B_data) 
{
  char A[10];
  char B[10];
  dtostrf(A_data,1,0,A);
  dtostrf(B_data,1,2,B);
  show(A_label,A,B_label,B);
}

// Watch - screen 4
void Screen::show(const char *A1_label, int A1_data, const char *A2_label, int A2_data, const char *B1_label, char* B1_data, const char *B2_label, float B2_data)
{
  char A1[10];
  char A2[10];
  char F1[10];
  char F2[10];
  dtostrf(A1_data,1,0,A1);
  dtostrf(A2_data,1,0,A2);
  dtostrf(B2_data,1,1,F2);
  //show(A1_label,A1,A2_label,A2,B1_label,B1_data,B2_label,F2);

  setupCharSection(false, 0,  0,42,A1_label,A1);
  setupCharSection(false,44,  0,42,A2_label,A2);
  setupCharSection(false, 0, 25,42,B1_label,B1_data);
  setupCharSection(false,44, 25,42,B2_label,F2);
  
  u8g->drawHLine(0, 24, 84);

}

/*
void Screen::showVario(const char *label, float climb, int alt1, int alt2) 
{
  char a1[10];
  char a2[10];
  char c[10];
  dtostrf(alt1,1,0,a1);
  dtostrf(alt2,1,0,a2);
  dtostrf(climb,1,2,c);
   
  //u8g->setDefaultBackgroundColor();     
  for (int x=1;x<12;x++) {
    if ((climb*2)>x) {
      u8g->drawBox(4+(x*8),47-(x*4), 7, (x*4));
      u8g->setDrawColor(0);
      u8g->drawVLine(5+(x*8),48-(x*4),(x*4)-2);
      u8g->setDrawColor(1);
    }
    else
    {
       u8g->drawFrame(4+(x*8),47-(x*4), 7, (x*4));
    }
  }
 
  u8g->setFont(FONT_MICRO);
  u8g->drawStr( 2, 5, label);
  u8g->drawHLine(0,6,46);
  u8g->setFont(FONT_BIG);
  u8g->drawStr( 5, 24, c);

  //u8g->setFont(FONT_BIG);
  //u8g->drawStr( 5, 40, a1);  
}
*/


void Screen::showPointer(int x,int y)
{  
  u8g->drawBox(x+0,y-2,4,4);
  u8g->drawVLine(x+4,y-1,2);
  u8g->setDrawColor(0);    
  u8g->drawHLine(x+1,y-1,2);
  u8g->setDrawColor(1);      
}

void Screen::battery(int x,int y, bool charge, float val)
{
  if (val<5.00)
  {
    // Voltage to % if needed
    if (charge)
    {
      // USB charging 3.5 = 0% 4.5 = 100%      
      val=(val-3.50)*100.00;
    }
    else
    {
      // Discharge 3.00 = 0% 4.1 = 100%      
      val=(val-3.00)*91.00;
    }
  }
  if (val<0) val = 0;
  if (val>100.00) val=100;
  
  u8g->setDrawColor(1);
  u8g->drawFrame(x+0, y+0, 29, 12);  
  u8g->drawVLine(x+29, y+3, 6);  
  u8g->drawVLine(x+30, y+3, 6);  
  
  if (val>0) {
    unsigned int wx = 25.00*val/100;
    u8g->drawBox(x+2, y+2, wx, 8);
    u8g->setDrawColor(0);
    u8g->drawHLine(x+3, y+3, wx-2);  
    u8g->setDrawColor(1);
  }  
}

void Screen::arrowUp(int y, bool fill)
{  
  u8g->drawVLine(0,y+3,5);
  u8g->drawVLine(6,y+3,5);  
  if (fill)
  {
    u8g->drawVLine(1,y+2,5);
    u8g->drawVLine(2,y+1,5);    
    u8g->drawVLine(3,y,5);    
    u8g->drawVLine(4,y+1,5);    
    u8g->drawVLine(5,y+2,5);    
  }
  else
  {  
    u8g->drawPixel(1,y+3);
    u8g->drawPixel(5,y+3);
    u8g->drawPixel(1,y+2);
    u8g->drawPixel(5,y+2);
    u8g->drawPixel(2,y+2);
    u8g->drawPixel(4,y+2);
    u8g->drawPixel(2,y+1);
    u8g->drawPixel(4,y+1);
    u8g->drawPixel(3,y+1);
    u8g->drawPixel(3,y);
  
    u8g->drawPixel(1,y+7);
    u8g->drawPixel(5,y+7);
    u8g->drawPixel(2,y+6);
    u8g->drawPixel(4,y+6);
    u8g->drawPixel(3,y+5);
  }
}

void Screen::arrowDown(int y,bool fill)
{  
  u8g->drawVLine(0,y,5);
  u8g->drawVLine(6,y,5);

  if (fill)
  {
    u8g->drawVLine(1,y+1,5);
    u8g->drawVLine(2,y+2,5);    
    u8g->drawVLine(3,y+3,5);    
    u8g->drawVLine(4,y+2,5);    
    u8g->drawVLine(5,y+1,5);    
  }
  else
  {
    u8g->drawPixel(1,y);
    u8g->drawPixel(5,y);
    u8g->drawPixel(1,y+1);
    u8g->drawPixel(5,y+1);
    u8g->drawPixel(2,y+1);
    u8g->drawPixel(4,y+1);
    u8g->drawPixel(2,y+2);
    u8g->drawPixel(4,y+2);
    u8g->drawPixel(2,y+2);
    u8g->drawPixel(3,y+3);
    u8g->drawPixel(3,y+3);
  
    u8g->drawPixel(1,y+5);
    u8g->drawPixel(5,y+5);
    u8g->drawPixel(2,y+6);
    u8g->drawPixel(4,y+6);
    u8g->drawPixel(3,y+7);
    u8g->drawPixel(3,y+7);
  }
}

void Screen::showVario(const char *label, CircularBuffer *cb, float climb, int alt1, int alt2) 
{
  char a1[10];
  char a2[10];
  char c[10];
  char avg[10];
  char mx[10];
  dtostrf(alt1,1,0,a1);
  dtostrf(alt2,1,0,a2);
  dtostrf(climb,1,1,c);

  float cmax = math.getMax(cb);
  float cavg = math.getAvg(cb);
  if (cmax>9.9) cmax=9.9;
  if (cmax<-9.9) cmax=-9.9;
  if (cavg>9.9) cavg=9.9;
  if (cavg<-9.9) cavg=-9.9;
  
  dtostrf(cavg,1,1,avg);
  dtostrf(cmax,1,1,mx);

  // draw gauge
  if (climb>0.00)
  {
    arrowUp(49,false);
    for (int l=1;l<6;l++)
    {
      if (l<(climb*2))
      {
        if (l<((climb*2)-0.5))
        {
          arrowUp((43-7*l),true);
        }
        else
        {
          arrowUp((43-7*l),false);
        }
      }
    }
  }
  else
  { 
    arrowDown(-5,false);   
    for (int l=-1;l>-6;l--)
    {
      if (l>(climb*2))
      {
        if (l>((climb*2)+0.5))
        {
          arrowDown(-4-(7*l),true);
        }
        else
        {
          arrowDown(-4-(7*l),false);
        }
      }
    }    
  }
 
  u8g->drawHLine(10, 24, 74);

  // Labels / Data
  int w=0;
  u8g->setFont(FONT_MICRO);
  w = u8g->getStrWidth("Alt");
  u8g->drawStr( 50-w, 5, "Alt");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(a1);
  u8g->drawStr( 50-w, 20, a1);
    
  u8g->setFont(FONT_MICRO);  
  w = u8g->getStrWidth("Climb");
  u8g->drawStr( 50-w, 32, "Climb");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(c);
  u8g->drawStr( 50-w, 48, c);  

  // second column
  u8g->setFont(FONT_MICRO);
  w = u8g->getStrWidth("Max");
  u8g->drawStr( 84-w, 5, "Max");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(mx);
  u8g->drawStr( 84-w, 20, mx);
    
  u8g->setFont(FONT_MICRO);  
  w = u8g->getStrWidth("Avg");
  u8g->drawStr( 84-w, 32, "Avg");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(avg);
  u8g->drawStr( 84-w, 48, avg);  

}

void Screen::showChart(const char *label, CircularBuffer *cb, float climb, int alt1, int alt2)
{
  char a1[10];
  char a2[10];
  char c[10];
  dtostrf(alt1,1,0,a1);
  dtostrf(alt2,1,0,a2);
  dtostrf(climb,1,1,c);

  // automatic scale handling 2/4 m/s
  float cmin = 100.00;
  float cmax = -100.00;
  for (int x=0;x<80;x++)
  {
    float c = cbIndex(cb,x);
    if (c<cmin) cmin=c;
    if (c>cmax) cmax=c;
  }   
  int scale=12; // 2 m/s range
  if (cmin<-2.00 || cmax>2.00)
  {
    scale=6; // 4 m/s range
  }

  // Axis
  int offset=40;
  u8g->drawHLine(40,24,44);  
  for (int x=0;x<40;x+=4) // just half of the screen
  {
    // scale 2 m/s
    u8g->drawPixel(offset+x+4,12);
    u8g->drawPixel(offset+x+4,36);
    if (scale==6) 
    {
      // scale 4 m/s - extra lines
      u8g->drawPixel(offset+x+4,6);
      u8g->drawPixel(offset+x+4,18);
      u8g->drawPixel(offset+x+4,30);    
      u8g->drawPixel(offset+x+4,42);    
    }
  }
  for (int x=0;x<40;x+=10)
  {
    // vertical ticks per 10 sec
    u8g->drawVLine(offset+x+4,23,3);
  }
  
  // Labels / Data
  int w=0;
  u8g->setFont(FONT_MICRO);
  w = u8g->getStrWidth("Alt");
  u8g->drawStr( 39-w, 5, "Alt");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(a1);
  u8g->drawStr( 39-w, 20, a1);
    
  u8g->setFont(FONT_MICRO);  
  w = u8g->getStrWidth("Climb");
  u8g->drawStr( 39-w, 32, "Climb");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(c);
  u8g->drawStr( 39-w, 48, c);  
 
  // Gauge 
  int y = (int)(climb*scale);
  showPointer(41,24-y);
  
  // Data
  for (int x=0;x<40;x++)
  {
    // 1 m/s = 6 pxl
    float y = (int)(cbIndex(cb,80-x)*scale);
    u8g->drawPixel(offset+x+4,24-(int)y);
    if (x<35 && y!=0 )
    {
      u8g->drawPixel(offset+x+4,23-(int)y);
    }
  }   
}

void Screen::showChartFull(const char *label, CircularBuffer *cb, float climb, int alt1, int alt2)
{
  char a1[10];
  char a2[10];
  char c[10];
  dtostrf(alt1,1,0,a1);
  dtostrf(alt2,1,0,a2);
  dtostrf(climb,1,1,c);


  // automatic scale handling 2/4 m/s
  float cmin = 100.00;
  float cmax = -100.00;
  for (int x=0;x<80;x++)
  {
    float c = cbIndex(cb,x);
    if (c<cmin) cmin=c;
    if (c>cmax) cmax=c;
  }   
  int scale=12; // 2 m/s range
  if (cmin<-2.00 || cmax>2.00)
  {
    scale=6; // 4 m/s range
  }

  // Axis
  u8g->drawHLine(0,24,84);  
  for (int x=0;x<80;x+=4) // just half of the screen
  {
    // scale 2 m/s
    u8g->drawPixel(x+4,12);
    u8g->drawPixel(x+4,36);
    if (scale==6) 
    {
      // scale 4 m/s - extra lines
      u8g->drawPixel(x+4,6);
      u8g->drawPixel(x+4,18);
      u8g->drawPixel(x+4,30);    
      u8g->drawPixel(x+4,42);    
    }
  }
  for (int x=0;x<80;x+=10)
  {
    // vertical ticks per 10 sec
    u8g->drawVLine(x+4,23,3);
  }
  
  // Labels / Data
  int w=0;
  u8g->setFont(FONT_MICRO);
  w = u8g->getStrWidth("Alt");
  u8g->drawStr( 80-w, 5, "Alt");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(a1);
  u8g->drawStr( 80-w, 20, a1);
    
  u8g->setFont(FONT_MICRO);  
  w = u8g->getStrWidth("Climb");
  u8g->drawStr( 80-w, 32, "Climb");
  u8g->setFont(FONT_BIG);
  w = u8g->getStrWidth(c);
  u8g->drawStr( 80-w, 48, c);  
 
  // Gauge 
  int y = (int)(climb*scale);
  showPointer(0,24-y);
  
  // Data
  for (int x=0;x<80;x++)
  {
    // 1 m/s = 6 pxl
    float y = (int)(cbIndex(cb,80-x)*scale);
    u8g->drawPixel(x+4,24-(int)y);
    if (x<35 && y!=0 )
    {
      u8g->drawPixel(x+4,23-(int)y);
    }
  }   
}

void Screen::update(int mode, int section, Vario *vario, float Vcc, float ver) 
{    
    switch (mode) {
    case S_VARIO1:
      {
        showChart("Climb [m/s]", &(vario->histBuf), (vario->ClimbRate), (int)(vario->Altitude), (int)(vario->Altitude));
      }
      break;          
    case S_VARIO2:
      {
        showChartFull("Climb [m/s]", &(vario->histBuf), (vario->ClimbRate), (int)(vario->Altitude), (int)(vario->Altitude));
      }
      break;          
    case S_VARIO3:    
      {
        // calculate time      
        unsigned int tm = millis()-(vario->Stopwatch);
        unsigned long allSeconds=tm/1000;
        int runHours= allSeconds/3600;
        int secsRemaining=allSeconds%3600;
        int runMinutes=secsRemaining/60;
        int runSeconds=secsRemaining%60;
        char tbuf[10];
        //sprintf(tbuf,"%02d:%02d:%02d",runHours,runMinutes,runSeconds);
        sprintf(tbuf,"%01d:%02d",runHours,runMinutes);
        show("Alt [m]",(int)(vario->Altitude),"Alt/2 [m]",(int)(vario->Altitude2),"Runtime",tbuf,"Climb [m/s]",(vario->ClimbRate));    
      }
      break;          
    case S_VARIO4:
      {
          char gain[10];
          float again = math.getGain(&(vario->altBuf));  
          dtostrf(again,1,0,gain);
          show("Alt [m]",(int)(vario->Altitude),"Alt/3 [m]",(int)(vario->Altitude3),"Alt.gain",gain,"Climb [m/s]",(vario->ClimbRate));    
      }      
      break;          
    case S_STATUS:
      {
        show("Temp [C]",vario->Temperature,"Battery [V]",Vcc);
        battery(2,35,false,Vcc);
      }
      break;      
    case S_CHARGE:
      {
        vario->Cfg.BeepStyle=0;
        vario->Cfg.BT_protocol=0;
        show("Charging Temp [C]",vario->Temperature,"Charging Voltage [V]",Vcc);
        battery(2,35,true,Vcc);
      }
      break;      
    case S_SETUP_1:
      {
        setupNumShow( section,
          "Ref. Baro", (int)vario->getZeroPressure(),
          "Alt [m]   ",(int)vario->Altitude,
          "Sink [cm] ",(int)vario->Cfg.SinkThreshold,
          "Climb [cm]",(int)vario->Cfg.ClimbThreshold
          );
      }
      break;        
    case S_SETUP_2:    
      {
        setupGaugeShow( section,
          "Sound",   (vario->Cfg.BeepStyle),MAX_SOUND,
          "Contrast",(vario->Cfg.Contrast),MAX_CONTRAST,
          "BT",      (vario->Cfg.BT_protocol),MAX_BT_PROTOCOL,
          "HSens.",  (vario->Cfg.Sensitivity),MAX_SENS);
      }
      break;  
    }
}  
