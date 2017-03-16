void dumpConfig(byte* address, int len) {
  Serial.printf("dumping config\n0x%04x: ",CONFIG_START);
  for(uint16_t t=0;t<len;t++){
    Serial.printf("0x%02x ",EEPROM.read(CONFIG_START+t));
  }
}

int loadConfig(byte* address, int len) {
  uint8_t b;
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  uint16_t configVersion = (EEPROM.read(CONFIG_START + 1)<<8)+EEPROM.read(CONFIG_START);
  Serial.printf("\nfound config at 0x%04x, version 0x%04x\n",CONFIG_START,configVersion);
  if (configVersion == CONFIG_VERSION){
        for (unsigned int t=0; t<len; t++){
          b=EEPROM.read(CONFIG_START + t);
          Serial.printf("0x%02x ",b);
          //*((char*)&address + t) = b;
          address[t]=b;
        }
        Serial.printf("read %i bytes from EEPROM\n", len);
        return len;
      }else{
        Serial.printf("config version mismatch! expected 0x%04x, got 0x%04x\n",CONFIG_VERSION,configVersion);
        return false;
      }
}

void saveConfig(uint8_t* address, int len){
  //EEPROM.write(CONFIG_START + 0,(CONFIG_VERSION&0xff00)>>8);
  //EEPROM.write(CONFIG_START + 1,CONFIG_VERSION&0xff);
  for (unsigned int t=0; t<len; t++){
    EEPROM.write(CONFIG_START + t, address[t]);
    Serial.printf("0x%02x ", address[t]);
  }
  Serial.printf("\n wrote %i to EEPROM, config file Version 0x%04x", len,address[1]<<8|address[0]);
}
