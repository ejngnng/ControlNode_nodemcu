#include "registerMsg.h"

registerMsg::registerMsg(deviceObj_t *device){
  this->device = *device;
  this->_bssid = WiFi.BSSIDstr();

  unsigned char mac[6];
  memset(mac, 0, 6);
  WiFi.macAddress(mac);
  String temp = "";
  for(unsigned char i=0; i<6; i++){
    temp += String(mac[i], HEX);
  }
  this->_uuid = temp;
  Serial.print("wifi macAddress: ");
  Serial.println(this->_uuid);
  this->_type = device->deviceType;
  this->_vendor = device->deviceVendor;
}

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
