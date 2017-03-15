int loadConfig(byte* address, int len) {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == (CONFIG_VERSION&0xff00)>>8 &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION&0x00ff){
        for (unsigned int t=0; t<len; t++){
          *((char*)&address + t) = EEPROM.read(CONFIG_START + t);
        }
        Serial.printf("read %i bytes from EEPROM\n", len);
        return len;
      }else{
        Serial.println("config version mismatch!");
        return -1;
      }
}

void saveConfig(uint8_t* address, int len){
  //EEPROM.write(CONFIG_START + 0,(CONFIG_VERSION&0xff00)>>8);
  //EEPROM.write(CONFIG_START + 1,CONFIG_VERSION&0xff);
  for (unsigned int t=0; t<len; t++){
    EEPROM.write(CONFIG_START + t, *((char*)&address + t));
    Serial.printf("wrote %i bytes to EEPROM, config file Version 0x%04x", len,address[0]<<8|address[1]);
  }
}
