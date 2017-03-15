/*
 * menuing functions
 */
 
void buildMenu(){
  mainMenu.add_menu(&visMenu);
    visMenu.add_item(&visMenu_1,&selectFreqGraph);
    visMenu.add_item(&visMenu_2,&selectPeakGraph);
    visMenu.add_item(&visMenu_3,&selectRollGraph);
    visMenu.add_item(&visMenu_4,&onBack);
  mainMenu.add_menu(&confMenu);
    confMenu.add_item(&confMenu_1,&setLEDBrightness);
    confMenu.add_item(&confMenu_2,&setDelayTime);
    confMenu.add_item(&confMenu_3,&onBack);
  mainMenu.add_item(&mainMenu_1,&onExit);
  ms.set_root_menu(&mainMenu);
}

void displayMenu() {
  display.fillScreen(0);
  display.setCursor(0,0);

  // Display the menu
  Menu const* cp_menu = ms.get_current_menu();
  display.print(cp_menu->get_selected()->get_name());
  display.display();
}

// Menu callback functions
void onBack(MenuItem* p_menu_item) {
  #ifdef DEBUG
    Serial.print("onBack");
  #endif
  ms.back();
}

void onExit(MenuItem* p_menu_item) {
  #ifdef DEBUG
    Serial.print("onExit");
  #endif
  inState &= ~IN_MENU;
  ms.back();
}

void selectFreqGraph(MenuItem* p_menu_item){
  myConfig.visualizationMode=VIS_FREQ;
  ms.back();
  displayMenu();
}

void selectPeakGraph(MenuItem* p_menu_item){
  myConfig.visualizationMode=VIS_FREQ_PEAK;
}

void selectRollGraph(MenuItem* p_menu_item){
  myConfig.visualizationMode=VIS_ROLL;
}

void setLEDBrightness(MenuItem* p_menu_item){
  displayMenu();
  inState |= IN_BRI;
}

void adjustBrightness(){
  if(Position>oldPosition){
    myConfig.bright<250?myConfig.bright+=5:myConfig.bright=255;
    matrix.setBrightness(myConfig.bright);
    drawVisualization(false);
    oldPosition=Position;
  } else if(Position<oldPosition){
    myConfig.bright>5?myConfig.bright-=5:myConfig.bright=0;
    matrix.setBrightness(myConfig.bright);
    drawVisualization(false);
    oldPosition=Position;
  }
  display.setCursor(0,16);
  display.fillRect(70,16,20,8,0);
  display.printf("Brightness: %i",myConfig.bright);          
  display.display();  
}

void setDelayTime(MenuItem* p_menu_item){
  displayMenu();
  inState |= IN_DEL;
}

void adjustDelay(){
  if(Position>oldPosition){
    myConfig.waitTime<400?myConfig.waitTime+=5:myConfig.waitTime=400;
    oldPosition=Position;
  } else if(Position<oldPosition){
    myConfig.waitTime>5?myConfig.waitTime-=5:myConfig.waitTime=0;
    oldPosition=Position;
  }
  display.setCursor(0,16);
  display.fillRect(70,16,20,8,0);
  display.printf("Delay Time: %ims",myConfig.waitTime);          
  display.display();  
}

bool buttonPressed(uint8_t pin){
  //Serial.print("Menu Button has been ");
  static bool pressed;
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

