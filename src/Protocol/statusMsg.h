#ifndef statusMsg_H
#define statusMsg_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "defs.h"

class statusMsg{
  private:
    String _uuid;
    String _attribute;
    String _value;
    deviceStatus_t status;
    
  public:
    statusMsg(deviceStatus_t *status);
    String statusMsgGenerator();

};









#endif
