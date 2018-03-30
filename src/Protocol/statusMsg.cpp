#include "statusMsg.h"

String statusMsg::statusMsgGenerator(){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  String msg;
  root["UUID"] = this->_uuid;
  root["attribute"] = this->_attribute;
  root["value"] = this->_value;
  root.printTo(msg);
  return msg;
}
