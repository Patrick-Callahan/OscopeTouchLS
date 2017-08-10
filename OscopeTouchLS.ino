/*--------------------------------------------------------------
  Program:       OscopetouchLCDmegaMED

  Description:   Digital Oscilloscope with data  displayed
                 on Color TFT LCD with touch screen
  
  Hardware:      sainsmart mega2560 board with 3.5" Tft lcd touch  module display and shield kit      
                 http://www.sainsmart.com/home-page-view/sainsmart-mega2560-board-3-5-Tft-lcd-module-display-shield-kit-for-atmel-atmega-avr-16au-atmega8u2.html

  Software:      Developed using Arduino 1.0.3 software
                 This program requires the UTft library and the
                 UTouch library from Henning Karlsen.
                 web: http://www.henningkarlsen.com/electronics
                 Version 1.1
  Date:          18 May 2014
 
  Author:        johnag    

  Ported by Patrick Callahan on 10 August 2017 to 2.8" display in Landscape
  Program:       OscopeTouchLS
  Description:   Digital Oscilloscope with data displayed
                 on Color TFT LCD with touch screen in landscape
  Repository:    https://github.com/pat02019/OscopeTouchLS
  Hardware:      Seeed Studios 2.8" Tft Touch Shield V2.0
                 Verified on an Arduino UNO R3 and Elegoo Mega 2560 R3
  Software:      Developed using Arduino 1.8.3 software
                 This program requires a custom version of the Seeed Studios library
                 https://github.com/pat02019/TFT_Touch_Shield_V2
                 The Touchscreen library is the standard one from Seeed Studios
                 https://github.com/Seeed-Studio/Touch_Screen_Driver
                 Version 2.0
  Date:          10 August 2017
  
--------------------------------------------------------------*/

#include <stdint.h>
#include <SPI.h>
#include <Wire.h>
#include <TFTv2LS.h>
#include <SeeedTouchScreen.h>

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // mega
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3
#define MIN_PORT 8   // lowest Analog Port pin to use A8 for Mega 
#define MAX_PORT 10  // highest Analog Port pin to use A10 for Mega 
                     // or customize if different ones are wanted

#elif defined(__AVR_ATmega32U4__) // leonardo
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 18   // can be a digital pin, this is A0
#define XP 21   // can be a digital pin, this is A3 
#define MIN_PORT 4   // lowest Analog Port pin to use A4 
#define MAX_PORT 5   // highest Analog Port pin to use A5 
                     // or customize if different ones are wanted

#else //168, 328, something else
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 14   // can be a digital pin, this is A0
#define XP 17   // can be a digital pin, this is A3 
#define MIN_PORT 4   // lowest Analog Port pin to use A4 
#define MAX_PORT 5   // highest Analog Port pin to use A5 
                     // or customize if different ones are wanted

#endif

//Measured ADC values for (0,0) and (210-1,320-1)
//TS_MINX corresponds to ADC value when X = 0
//TS_MINY corresponds to ADC value when Y = 0
//TS_MAXX corresponds to ADC value when X = 320 -1
//TS_MAXY corresponds to ADC value when Y = 240 -1

// set screen in landscape mode 
//TFT resolution 320*240
#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2

// Set screen in landscape mode 
//#undef MAX_X
//#undef MAX_Y
//TFT resolution 320*240
//#define MIN_X  0
//#define MIN_Y 0
//#define MAX_X 319
//#define MAX_Y 239


// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM);
// a point object holds x y and z coordinates
Point p = ts.getPoint();

// Declare variables
char buf[12];
int x,y,z;
int Input = 0;
byte Sample[320];
byte OldSample[320];
float StartSample = 0; 
float EndSample = 0;
int Max = 0;
int Min = 500;
float fMax = 0.0f;
float fMin = 0.0f;
int mode = 0;
int dTime = 1;
int tmode = 0;
int Trigger = 0;
int SampleSize = 0;
float SampleTime = 0;
int dgvh;
int hpos = 55; //was 105; //set 0v on horizontal  grid
int vsens = 4; // was 3; // vertical sensitivity
int port = MIN_PORT;
// variables for DVM
int sum = 0;                    // sum of samples taken
float averageTT = 0;                  // actually 10 times the average as an int

// Define various ADC prescaler
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

//------------Start Subrutines------------------------------------

//--------draw buttons sub
void buttons(){
   Tft.fillRectangle (250, 1, 60, 50, BLUE);
   Tft.fillRectangle (250, 55, 60, 50, BLUE);
   Tft.fillRectangle (250, 110, 60, 50, BLUE);
   Tft.fillRectangle (250, 165, 60, 50, BLUE);
}
//-------touchscreen position sub
void touch(){
 p = ts.getPoint();
  while (p.z > __PRESURE)
  {
    // Converts from raw XY to screen coord system
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
    x=p.x;
    y=p.y;
    if ((y>=1) && (y<=50))  // Delay row
    {
        if ((x>=1) && (x<=50))  //  Delay Button
//          waitForIt(250, 1, 310, 50);
      mode++ ;
        {
              Tft.drawRectangle (250, 1, 60, 50, BLUE);   
// Select delay times you can change values to suite your needs
        if (mode == 0) dTime = 1;
        if (mode == 1) dTime = 10;
        if (mode == 2) dTime = 20;
        if (mode == 3) dTime = 50;
        if (mode == 4) dTime = 100;
        if (mode == 5) dTime = 200;
        if (mode == 6) dTime = 300;
        if (mode == 7) dTime = 500;
        if (mode == 8) dTime = 1000;
        if (mode == 9) dTime = 5000;
        if (mode == 10) dTime = 10000;
        if (mode > 10) mode = 0;   
      }
    }

        if ((y>=1) && (y<=50))  // Trigger  row
        {
            if ((x>=80) && (x<=130))  // Trigger Button
//                waitForIt(250, 55, 310, 105);
            tmode++;
            {
// Select Software trigger value
        Tft.drawRectangle (250, 55, 60, 50, BLUE);      
        if (tmode == 1) Trigger = 0;
        if (tmode == 2) Trigger = 10;
        if (tmode == 3) Trigger = 20;
        if (tmode == 4) Trigger = 30;
        if (tmode == 5) Trigger = 50;
        if (tmode > 5)tmode = 0;
      }
    }
    if ((y>=1) && (y<=50))  // Port select   row
    {
      if ((x>=160) && (x<=210))  // Port select Button
//        waitForIt(250, 110, 310, 160);
      port++;
      {
        Tft.drawRectangle (250, 110, 60, 50, BLUE); 
        //Tft.clrScr();
        //Tft.fillScreen(0,0,319,239,BLACK);
        buttons();
        if (port > MAX_PORT)
          port = MIN_PORT;
      }
    }
    delay(500);
    p = ts.getPoint();
  }
}
//----------wait for touch sub 
// Not sure why???
void waitForIt(int x1, int y1, int x2, int y2)
{
  while (p.z > __PRESURE)
  z = p.z;
}
//----------draw grid sub
void DrawGrid(){
  for(  dgvh = 0; dgvh < 5; dgvh ++){
    Tft.drawVerticalLine( dgvh * 40, 0, 200, GREEN);
    Tft.drawHorizontalLine(  0, dgvh * 40, 240, GREEN);
  }
  Tft.drawVerticalLine( 200, 0, 200, GREEN);
  Tft.drawVerticalLine( 240, 0, 200, GREEN);
  Tft.drawHorizontalLine( 0, 200, 240, GREEN);
      
  Tft.drawRectangle (250, 1, 60, 50, YELLOW);
  Tft.drawRectangle (250, 55, 60, 50, YELLOW);
  Tft.drawRectangle (250, 110, 60, 50, YELLOW);
  Tft.drawRectangle (250, 165, 60, 50, CYAN);
 
}
// ------ Wait for input to be greater than trigger sub
void trigger(){
  while (Input < Trigger)
  {
    Input = analogRead(port)*5/100;
  }
}


//---------------End Subrutine  ----------------------

void setup()
{
// Initialize Screen and touch functions
    TFT_BL_ON;      // turn on the background light
    Tft.TFTinit();  // init TFT library
    Tft.fillScreen(0,0,319,239,BLACK);
    buttons();
    //pinMode(0, INPUT); 
// set up the ADC
    ADCSRA &= ~PS_128;  // remove bits set by Arduino library

// you can choose a prescaler from below.
// PS_16, PS_32, PS_64 or PS_128
    ADCSRA |= PS_128;    // set our own prescaler 
}

void loop()
{
// a point object holds x y and z coordinates
Point p = ts.getPoint();
    while(1)
  {
        DrawGrid();
        touch();
    //trigger();

// Collect the analog data into an array
    StartSample = micros();
    for( int xpos = 0;xpos < 245; xpos ++)
    {
      Sample[ xpos] = analogRead(port)*5/102;
      delayMicroseconds(dTime);
    }
    EndSample = micros();
  
// Display the collected analog data from array
    for( int xpos = 0; xpos < 239; xpos ++)
    {
// Erase previous display
      Tft.drawLine (xpos + 1, 255-OldSample[ xpos + 1]* vsens-hpos, xpos + 2, 255-OldSample[ xpos + 2]* vsens-hpos, BLACK);
      if (xpos == 0)
        Tft.drawLine(xpos + 1, 1, xpos + 1, 239,BLACK);
 //Draw the new data
      Tft.drawLine (xpos, 255-Sample[ xpos]* vsens-hpos, xpos + 1, 255-Sample[ xpos + 1]* vsens-hpos, WHITE);
    }
// Determine sample voltage peak to peak
// Including the minimum, maximum and average
    Max = Sample[ 100];
    Min = Sample[ 100];
    for( int xpos = 0; xpos < 240; xpos ++)
    {
      sum += Sample[xpos];
      OldSample[ xpos] = Sample[ xpos];
      if (Sample[ xpos] > Max)
        Max = Sample[ xpos];
      if (Sample[ xpos] < Min)
        Min = Sample[ xpos];
    }
    averageTT = sum/240.0/10.0;
    sum =0;
    
// display the sample time, delay time and trigger level
    Tft.drawString("Delay", 250, 5, 2, WHITE);
    Tft.drawString(itoa(dTime, buf, 10), 250, 25, 2, WHITE);

    Tft.drawString("Trig.", 252, 60,2,WHITE);
    Tft.drawString(itoa(Trigger, buf, 10), 270, 80, 2, WHITE);

    SampleTime =( EndSample/1000 - StartSample/1000)/6;
    Tft.drawString("mS/dv", 250, 170, 2, WHITE);
    Tft.drawFloat(SampleTime, 1, 250, 190, 2, WHITE);

    Tft.drawString("Port", 254, 120, 2, WHITE);
    Tft.drawString(itoa(port, buf, 10), 270, 140, 2, WHITE);

// Display heading in yellow
    Tft.drawString("AveV", 10, 202, 2, YELLOW);
    Tft.drawString("MaxV", 100, 202, 2, YELLOW);
    Tft.drawString("MinV", 180, 202, 2, YELLOW);
    Tft.drawString("1V/d",250 ,219, 2, YELLOW);

// Display the average, maximum and minimum values of the display
    Tft.fillRectangle (10, 219, 60, 25, BLACK);   // Clears the previous number
    Tft.drawFloat(averageTT,1, 10, 219, 3, GREEN);
    
    Tft.fillRectangle (100, 219, 60, 25, BLACK);   // Clears the previous number
    fMax = Max/10.0f;
    Tft.drawFloat(fMax,1, 100, 219, 3, GREEN);

    Tft.fillRectangle (180, 219, 60, 25, BLACK);   // Clears the previous number
    fMin = Min/10.0f;
    Tft.drawFloat(fMin,1, 180, 219, 3, GREEN);

  }
}
