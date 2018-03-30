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

  public:
    String statusMsgGenerator();

};









#endif
