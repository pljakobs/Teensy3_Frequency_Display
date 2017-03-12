#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Encoder.h>

#define MatrixPin 2

#define ePin1 14
#define ePin2 15
#define ROT_OFFS 3
#define bPin  17
#define OLED_RESET 13

Adafruit_SSD1306 display(OLED_RESET);
Encoder myEncoder(ePin1, ePin2);

// GUItool: begin automatically generated code
AudioInputAnalog         audioInput;     //xy=140,189
AudioAnalyzeFFT1024      myFFT;          //xy=382,189
AudioConnection          patchCord1(audioInput, myFFT);
// GUItool: end automatically generated code

#define VIS_TIME 0
#define VIS_ROLL 1
#define VIS_PEAK 2

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

// An array to hold the 16 frequency bands
float level[8][8];

// This array holds the on-screen levels.  When the signal drops quickly,
// these are used to lower the on-screen level 1 bar per update, which
// looks more pleasing to corresponds to human sound perception.
uint32_t   pixel[8][8];

uint16_t   visualizationMode;

elapsedMillis timeSinceLastFrame;

char* mainMenu[]={"Visualization","Brightness","Switch Off","Exit"};
uint8_t mainItems=4;
char* visMenu[] ={"Time graph","Rolling","Peak hold"};
uint8_t visItems=3;
uint8_t menuItem,menuButton;
uint8_t bright=20;
bool pressed;
long oldPosition,Position;

void setup(){
  //pinMode(ePin1,INPUT_PULLUP);
  //pinMode(ePin2,INPUT_PULLUP);
  pinMode(bPin,INPUT_PULLUP);
  Serial.begin(115200);
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
  delay(500);
  for(int c=255;c!=0;c--){
    matrix.fillScreen(matrix.Color(c,0,0));
    matrix.show();
    delay(10);
  }
  visualizationMode=VIS_TIME;
}

void loop() {

  //float n;
  //int i;
  uint32_t color;
  uint16_t r,g,b;

  if(!digitalRead(bPin)) menuButton=true;
  if (myFFT.available()){
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

    if(timeSinceLastFrame>40){
      drawVisualization(true);
      timeSinceLastFrame=0;
    }
  }else if(buttonPressed(bPin)) {
    /* 
     *  Menu!
     */
    display.fillScreen(0);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("--Menu--");
    
    menuItem=0;
    Serial.println(mainMenu[menuItem]);
    printMenuItem(1,mainMenu,menuItem);
    display.display();
    while(!buttonPressed(bPin)){
      Position=myEncoder.read();
      delay(10);
      if(Position>oldPosition+ROT_OFFS){
        menuItem==mainItems-1?mainItems:menuItem++;
        printMenuItem(1,mainMenu,menuItem);
        Serial.println(mainMenu[menuItem]);
        oldPosition=Position;
        display.display();
      }else if(Position < oldPosition-ROT_OFFS){
        menuItem==0?0:menuItem--;
        printMenuItem(1,mainMenu,menuItem);
        Serial.println(mainMenu[menuItem]);
        oldPosition=Position;
        display.display();
      }
    }
    //Serial.printf("selected menu item %i-%s",menuItem,mainMenu[menuItem]);
    switch(menuItem){
      case 0:
        printMenuItem(2,visMenu,visualizationMode);
        display.display();
        while(!buttonPressed(bPin)){
          Position=myEncoder.read();  
          if(Position>oldPosition+ROT_OFFS){
            visualizationMode==visItems-1?visItems:visualizationMode++;
            printMenuItem(2,visMenu,visualizationMode);
            display.display();
            oldPosition=Position;
          }else if(Position < oldPosition-ROT_OFFS){
            visualizationMode==0?0:visualizationMode--;
            printMenuItem(2,visMenu,visualizationMode);
            display.display();
            oldPosition=Position;
          }
        }
        display.fillScreen(0);
        display.display();
        break;
      case 1:
        while(digitalRead(bPin)){

          Position=myEncoder.read();  
          if(Position>oldPosition){
            bright<250?bright+=5:bright=255;
            matrix.setBrightness(bright);
            drawVisualization(false);
            oldPosition=Position;
          } else if(Position<oldPosition){
            bright>5?bright-=5:bright=0;
            matrix.setBrightness(bright);
            drawVisualization(false);
            oldPosition=Position;
          }
          display.setCursor(0,16);
          display.fillRect(70,16,20,8,0);
          display.printf("Brightness: %i",bright);          
          display.display();  
        }
        display.fillScreen(0);
        display.display();
        break;
      case 2:
        display.fillScreen(0);
        display.display();
        break;
      case 3:
        display.fillScreen(0);
        display.display();
        break;
    }
  }
}

void drawVisualization(bool screen){
        switch(visualizationMode){
        case VIS_TIME:
          visualizeTime(screen);
          break;
        case VIS_ROLL:
          visualizeRoll(screen);
          break;
        case VIS_PEAK:
          visualizePeakHold(screen);
          break;
      }
}
bool buttonPressed(uint8_t pin){
  //Serial.print("Menu Button has been ");
  if(digitalRead(pin)){
      if (pressed){
        Serial.println("released ");
        pressed=false;
        return true;
      }else{
        //Serial.println("untouched ");
        return false;
      }
  }else{
    pressed=true;
    Serial.println("pressed ");
    return false;
  }
}
void printMenuItem(uint8_t line,char* menu[], uint8_t item){
  display.setCursor(0,line*8);
  display.fillRect(0,line*8,128,10,0);
  display.printf("%i %s",item,menu[item]);
}
void visualizeTime(bool screen){
  /* ------------------------------------------------------------------  
   * levels[x][y] contain the last eight fft results
   * this should mainly draw levels[0][y], but use some falloff damping.
   * I will try to do that by applying 1/(2*x)*levels[x][y] first
   * ------------------------------------------------------------------ */  

  uint8_t bar[8];
  // Serial.println("visualizeTime()");
  for(uint16_t y=0;y<=7;y++){
    // 1+1/2+1/4+1/8+...+1/n =2, hence multiplying the whole bunch with 4 gets us to the max value of 8
    bar[y]=(level[0][y]+(1/2)*level[1][y]+(1/4)*level[2][y]+(1/8)*level[3][y]+(1/16)*level[4][y]+(1/32)*level[5][y]+(1/64)*level[6][y]+(1/128)*level[7][y])*6;
  }
  display.fillScreen(0);
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
    if(screen){
      display.fillRect(x*16,32-(bar[x]*4),16,bar[x]*4,WHITE);
      display.drawRect(x*16,32-(bar[x]*4),16,bar[x]*4,0);
      //Serial.printf("x: %i, y: %i, w: %i, h: %i\n",x*16,64-(bar[x]*8),16,bar[x]*8);
     //Serial.println("==============");
    }
  }
  matrix.show();
  if(screen) display.display();
}

void visualizeRoll(bool screen){
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

