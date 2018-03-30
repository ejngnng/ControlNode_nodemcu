#ifndef registerMsg_H
#define registerMsg_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "defs.h"

class registerMsg{
  private:
    String _bssid;
    String _uuid;
    String _type;
    String _vendor;
    deviceObj_t device;
  public:
    registerMsg(deviceObj_t *device);
    String registerMsgGenerator();
};


#endif
