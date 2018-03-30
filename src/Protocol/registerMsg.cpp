#include "registerMsg.h"


String registerMsg::registerMsgGenerator(){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  String msg;
  root["BSSID"] = this->_bssid;
  root["UUID"] = this->_uuid;
  root["type"] = this->_type;
  root["vendor"] = this->_vendor;
  root["MAC"] = this->_uuid;
  root.printTo(msg);
  return msg;
}
