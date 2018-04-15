#ifndef __WIFI_H
#define __WIFI_H

#include <Arduino.h>

#include "common/common.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>


#define WiFi_SSID_LEN_ADDR  0
#define WiFi_PSW_LEN_ADDR   1

#define WiFi_SSID_ADDR      2
#define WiFi_PSW_ADDR      34

typedef struct WiFi_Obj_s{
  unsigned int ssid_len;
  unsigned int psw_len;
  String       wifi_ssid;
  String       wifi_psw;
}WiFi_Obj_t;

void SmartConfig();
void Connect_WiFi();
void clear_eeprom();
void get_wifi(WiFi_Obj_t *wifi_obj);
void store_wifi();

String getMAC();

#endif
