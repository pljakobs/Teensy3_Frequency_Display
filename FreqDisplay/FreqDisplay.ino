/*
 * This is a simple audio visualization project.
 * it is based on the teensy audio library and requires a teensy 3.x to run
 * The configuration is:
 * -> (currently) 8x8 Adafruit NeoPixel array (or other WS2812B) attached to MatrixPin (here: 2)
 * -> a 128x32 I2C OLED display (well, that's the current config, you may be able to use a larger one), attached to hardware i2c
 * -> a rotary encoder attached to Pins ePin1 and ePin2 (here: 14 & 15) plus it's push function on bPin (here: 17)
 * 
 * you will need to install additional libraries:
 * 
 * -> Adafruit GFX
 * -> Adafruit NeoMatrix
 * -> Adafruit NeoPixel
 * -> Encoder
 * -> Audio
 * -> MenuSystem
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
//#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Encoder.h>
#include <MenuSystem.h>

#define MatrixPin 2

#define ePin1 14
#define ePin2 15
#define ROT_OFFS 3
#define bPin  17
#define OLED_RESET 13

#define VIS_FREQ      0
#define VIS_FREQ_PEAK 1
#define VIS_ROLL      2

#define DISP_ARRAY 1
#define DISP_OLED  2

//#define DEBUG

MenuSystem ms;
Menu mainMenu("Menu");
MenuItem mainMenu_1("exit");
Menu visMenu("Visualization");
MenuItem visMenu_1("Frequency Graph");
MenuItem visMenu_2("Frequency Peak Hold");
MenuItem visMenu_3("Roll");
MenuItem visMenu_4("exit");
Menu confMenu("Config");
MenuItem confMenu_1("LED Brightness");
MenuItem confMenu_2("Wait Time");
MenuItem confMenu_3("exit");

Adafruit_SSD1306 display(OLED_RESET);
Encoder myEncoder(ePin1, ePin2);

// GUItool: begin automatically generated code
AudioInputAnalog         audioInput;     //xy=140,189
AudioAnalyzeFFT1024      myFFT;          //xy=382,189
AudioConnection          patchCord1(audioInput, myFFT);
// GUItool: end automatically generated code

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 1, 1, MatrixPin,
  NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_ROWS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS ,
  NEO_GRB + NEO_KHZ800);
/*Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 1, 1, PIN,
  NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_ROWS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);
  */
// The scale sets how much sound is needed in each frequency range to
// show all 8 bars.  Higher numbers are more sensitive.
float scale = 60.0;

// array to hold the frequency bands
float level[8][8];

// array holding the bar hights
uint8_t bar[8];

uint16_t   visualizationMode;

elapsedMillis timeSinceLastFrame;

uint8_t menuItem,menuButton;
uint8_t bright=20;
uint8_t waitTime=40;
bool pressed,inMenu,barsValid;
long oldPosition,Position;

#define IN_MENU 0x80
#define IN_BRI  0x01
#define IN_DEL  0x02

uint8_t inState;

const float e=2.718281828;
float decayVal=0.01;

void setup(){
  Serial.begin(115200);
  #ifdef DEBUG
    Serial.println("Starting setup");
  #endif
  pinMode(bPin,INPUT_PULLUP);

  AudioMemory(12);
  myFFT.windowFunction(AudioWindowHanning1024);

  matrix.begin();
  matrix.setBrightness(bright);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.fillScreen(0);
  display.setTextColor(WHITE);
  display.print("Frequency Analyzer");
  display.display();
  
  matrix.fillScreen(matrix.Color(255,0,0));
  matrix.show();
  #ifdef DEBUG
    Serial.println("starting fill screen fade");
  #endif
  for(int c=1;c!=53;c++){
    #ifdef DEBUG
      Serial.printf("step %i\n",c);
    #endif
    matrix.fillScreen(matrix.Color((uint8_t)(255*pow(e,-decayVal*c)),0,0));
    matrix.show();
    delay(10);
  }
  visualizationMode=VIS_FREQ;
  buildMenu();
}

void loop() {

  uint32_t color;
  uint16_t r,g,b;

  if (myFFT.available()){
    collectFFT();
    barsValid=false;
  }
  
  if(timeSinceLastFrame>waitTime){
    #ifdef DEBUG
      Serial.printf("wait Time: %ims\n", waitTime);
    #endif
    drawVisualization(DISP_ARRAY);
    if(inState==0) drawVisualization(DISP_OLED);
    timeSinceLastFrame=0;
  }
  
  if(buttonPressed(bPin)){
    switch(inState){
      case 0x00: //not in menu
        inState |=IN_MENU; //enter menu
        displayMenu();
        break;
      case (IN_MENU|IN_BRI):
        inState &= ~IN_BRI; //reset brightness flag
        ms.back();
        displayMenu();
        break;
      case (IN_MENU|IN_DEL):
        inState &= ~IN_DEL; //reset delay flag
        ms.back();
        displayMenu();
        break;
      case IN_MENU:
        ms.select();
        displayMenu();
        break; //this is being handled by the menu functions themselves
    }
  }     
  
  Position=myEncoder.read();
  switch(inState){
    case 0x00: //not in menu
      break;   //do nothing
    case IN_MENU:
      if(Position>oldPosition+ROT_OFFS){
        oldPosition=Position;
        ms.next();
        displayMenu();
      }else if(Position<oldPosition-ROT_OFFS){
        oldPosition=Position;
        ms.prev();
        displayMenu();
      }
      break;
    case (IN_MENU|IN_BRI):
      adjustBrightness();
      break;
    case (IN_MENU|IN_DEL):
      adjustDelay();
      break;
  }     
}
void showProgress(int p){
  p/=4;
  int x,y,q;
  for(q=0;q<63;q++){
    y=q/8;
    x=q-8*y;
    if(q<p){
      setPixel(x,y,makeColor(0,0,127));
    }else{
      setPixel(x,y,makeColor(127,127,127));
    }
  }
  matrix.show();
}

void collectFFT(){
    //Serial.println("got new FFT");
    for(int i=7;i>=1;i--){
      for(int j=0;j<=7;j++){
        level[i][j]=level[i-1][j];
      }
    }
    level[0][0] = level[0][0]/2 + myFFT.read(0,  1)/2;
    level[0][1] = level[0][1]/2 + myFFT.read(2,  6)/2;
    level[0][2] = level[0][2]/2 + myFFT.read(7,  15)/2;
    level[0][3] = level[0][3]/2 + myFFT.read(16, 32)/2;
    level[0][4] = level[0][4]/2 + myFFT.read(33, 66)/2;
    level[0][5] = level[0][5]/2 + myFFT.read(67, 131)/2;
    level[0][6] = level[0][6]/2 + myFFT.read(132,257)/2;
    level[0][7] = level[0][0]/2 + myFFT.read(258,511)/2;
}

void computeBars(){
  for(uint16_t y=0;y<=7;y++){
  static uint8_t decayTime;
    if(decayVal==0){
      bar[y]=level[0][y]*8;
    }else{
      if(level[0][y]*8>=bar[y]){
        bar[y]=level[0][y]*8;
        decayTime=0;
      }else{
        bar[y]=level[0][y]*8*pow(e,(-decayVal*decayTime));
        #ifdef DEBUG
          Serial.printf("decayTime: %i\n", decayTime);
        #endif
      }
    }
  }
  barsValid=true;
}

void drawVisualization(int screen){
  switch(visualizationMode){
    case VIS_FREQ:
      visualizeFreq(screen,false);
      break;
    case VIS_FREQ_PEAK:
      visualizeFreq(screen,true);
      break;
    case VIS_ROLL:
      if(screen==DISP_OLED){
        visualizeFreq(screen,false); //monochrome OLED can't do rolling color
      }else{
        visualizeRoll(screen);
      }
      break;
  }
}

void visualizeFreq(int disp,bool peak){
  /* ------------------------------------------------------------------  
   * levels[x][y] contain the last eight fft results
   * this should mainly draw levels[0][y], but use some falloff damping.
   * I will try to do that by applying 1/(2*x)*levels[x][y] first
   * ------------------------------------------------------------------ */  

  // Serial.println("visualizeFreq()");
  #ifdef DEBUG
    Serial.printf("entered visualizeFreq with disp=%i and peak=%b",disp,peak);
  #endif
  if(!barsValid) computeBars();

  if(disp==DISP_ARRAY){
    for(uint16_t x=0;x<=7;x++){
      for(uint16_t y=0;y<=7;y++){
        if(y+1>bar[x]){
          // Serial.println();
          setPixel(x,y,0);
        }else if(y+1<6){
          // Serial.print("o");
          setPixel(x,y,makeColor(0,255,0));
        }else if(y+1<8){
          // Serial.print("#");
          setPixel(x,y,makeColor(255,255,0));
        }else{
          // Serial.print("+");
          setPixel(x,y,makeColor(255,0,0));
        }
      }
    }
    matrix.show();
  }else if(disp==DISP_OLED){
    display.fillScreen(0);
    for(uint16_t x=0;x<=7;x++){
      display.fillRect(x*16,32-(bar[x]*4),16,bar[x]*4,WHITE);
      display.drawRect(x*16,32-(bar[x]*4),16,bar[x]*4,0);
      #ifdef DEBUG
        Serial.printf("x: %i, y: %i, w: %i, h: %i\n",x*16,64-(bar[x]*8),16,bar[x]*8);
        Serial.println("==============");
      #endif
    }
    display.display();
  }
}

void visualizeRoll(int disp){
  #ifdef DEBUG
  #endif
  for(uint16_t x=0;x<=7;x++){
    float avg=(level[x][0]+level[x][1]+level[x][2]+level[x][3]+level[x][4]+level[x][5]+level[x][6]+level[x][7])/8;
    for(uint16_t y=0;y<=7;y++){
      uint32_t c=hsv2rgb((int)(level[x][y]*210)+150,255,(uint16_t)(255*avg));
      //
      //Serial.printf("x: %i, y: %i, level: %f [%i] bright: %f [%i] r: %i, g: %i, b:%i \n",x,y,level[x][y],270-(int)(level[x][y]*180),avg,(uint16_t)(255*avg),(c&0xff0000)>>16,(c&0xff00)>>8,c&0xff);
      setPixel(x,y,c); 
    }
    //Serial.println();
   }
   matrix.show();
}

void visualizePeakHold(bool screen){
  
}

uint32_t makeColor(uint16_t r, uint16_t g, uint16_t b){
  return r<<16|g<<8|b;
}

void setPixel(uint16_t x, uint16_t y, uint32_t color){
  /* -------------------------------------------
   *  wrapper to set 32 Bit color
   * ------------------------------------------- */
  matrix.setPassThruColor(color);
  matrix.drawPixel(7-x,y,0); //just a dummy color
}

uint32_t hsv2rgb(uint16_t _h, uint16_t _s, uint16_t _v){
  float _r, _g, _b;
  // Serial.printf("==hsv2rgb(%i,%i,%i)\n",_h,_s,_v);
  float h = (float)(_h*255 / 360);
  float s = (float)(_s*255 / 255);
  float v = (float)(_v*255 / 255);
  // Serial.printf("===hsv2rgb h: %f s: %f v: %f (pre scale)\n",h,s,v);
  h=h/255;
  s=s/255;
  v=v/255;
  // Serial.printf("===hsv2rgb h: %f s: %f v: %f\n",h,s,v);
  int i = floor(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);
  
  switch (i % 6) {
    case 0: _r = v, _g = t, _b = p; break;
    case 1: _r = q, _g = v, _b = p; break;
    case 2: _r = p, _g = v, _b = t; break;
    case 3: _r = p, _g = q, _b = v; break;
    case 4: _r = t, _g = p, _b = v; break;
    case 5: _r = v, _g = p, _b = q; break;
  }
  //Serial.printf("==hsv2rgb return r: %f, g: %f, b: %f\n", _r,_g,_b);
  //Serial.printf("==hsv2rgb return r: %i, g: %i, b: %i\n", (uint16_t)(_r*255),(uint16_t)(_g*255),(uint16_t)(_b*255));
  return makeColor((uint16_t)(_r*255),(uint16_t)(_g*255),(uint16_t)(_b*255));
}

