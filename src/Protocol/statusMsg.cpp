#include "statusMsg.h"

statusMsg::statusMsg(deviceStatus_t *status){
  this->status = *status;
  switch(status->attribute){
    case ONOFF:
        this->_attribute = "onoff";
        break;
    case LIGHTNESS:
        this->_attribute = "lightness";
        break;
    case TEMPERATURE:
        this->_attribute = "temperature";
        break;
    case MODE:
        this->_attribute = "mode";
        break;
    case TIMER_ON:
        this->_attribute = "timer_on";
        break;
    case TIMER_OFF:
        this->_attribute = "timer_off";
        break;
    case ONOFFLINE:
        this->_attribute = "online";
        break;
    default:
        break;
  }

  unsigned char mac[6];
  memset(mac, 0, 6);
  WiFi.macAddress(mac);
  String temp = "";
  for(unsigned char i=0; i<6; i++){
    temp += String(mac[i], HEX);
  }
  this->_uuid = temp;
  this->_value = status->value;
}

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
