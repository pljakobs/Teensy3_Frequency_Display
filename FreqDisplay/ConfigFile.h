#include <EEPROM.h>
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_VERSION  0x0316
#define CONFIG_START    0x00

struct configStruct{
  uint16_t  configVersion;
  uint16_t  visualizationMode;
  uint8_t   bright=20;
  uint8_t   waitTime=40;
  uint8_t   rotation;
};

#endif
