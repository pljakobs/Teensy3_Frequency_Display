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
    confMenu.add_item(&confMenu_3,&setRotation);
    confMenu.add_item(&confMenu_4,&onExitConfig);
  mainMenu.add_item(&mainMenu_1,&onExit);
  ms.set_root_menu(&mainMenu);
}

void handleControls(){
    if(buttonPressed(bPin)){
    switch(inState){
      case 0x00: //not in menu
        inState |=IN_MENU; //enter menu
        displayMenu();
        break;
      case (IN_MENU|IN_BRI):
        saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
        inState &= ~IN_BRI; //reset brightness flag
        //ms.back();
        displayMenu();
        break;
      case (IN_MENU|IN_DEL):
        saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
        inState &= ~IN_DEL; //reset delay flag
        //ms.back();
        displayMenu();
        break;
      case (IN_MENU|IN_ROT):
        saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
        inState &= ~IN_ROT; //reset delay flag
        //ms.back();
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
    case (IN_MENU|IN_ROT):
      adjustRotation();
      break;
  }     
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
  displayMenu();
}

void onExit(MenuItem* p_menu_item) {
  #ifdef DEBUG
    Serial.print("onExit");
  #endif
  inState &= ~IN_MENU;
  ms.back();
}

void onExitConfig(MenuItem* p_menu_item){
  Serial.println("onExitConfig");
  saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
  ms.back();
  displayMenu();
}

void selectFreqGraph(MenuItem* p_menu_item){
  myConfig.visualizationMode=VIS_FREQ;
  saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
  ms.back();
  displayMenu();
}

void selectPeakGraph(MenuItem* p_menu_item){
  myConfig.visualizationMode=VIS_FREQ_PEAK;
  saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
  ms.back();
  displayMenu();
}

void selectRollGraph(MenuItem* p_menu_item){
  myConfig.visualizationMode=VIS_ROLL;
  saveConfig((uint8_t*)&myConfig,sizeof(configStruct));
  ms.back();
  displayMenu();
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

void setRotation(MenuItem* p_menu_item){
  displayMenu();
  inState |= IN_ROT;
}

void adjustRotation(){
  if(Position>oldPosition+ROT_OFFS){
    myConfig.rotation<3?myConfig.rotation++:myConfig.rotation=0;
    matrix.setRotation(myConfig.rotation);
    oldPosition=Position;
  } else if(Position<oldPosition){
    myConfig.rotation>=0?myConfig.rotation--:myConfig.rotation=3;
    matrix.setRotation(myConfig.rotation);
    oldPosition=Position;
  }
  display.setCursor(0,16);
  display.fillRect(50,16,20,8,0);
  display.printf("Rotation: %i",myConfig.rotation);          
  display.display();  
}

bool buttonPressed(uint8_t pin){
  static bool pressed;
  if(digitalRead(pin)){
      if (pressed){
        pressed=false;
        return true;
      }else{
        return false;
      }
  }else{
    pressed=true;
    return false;
  }
}

