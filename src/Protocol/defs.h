#ifndef __DEFS_H
#define __DEFS_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "common/common.h"

enum attribute_type{
  ONOFF = 0,
  LIGHTNESS,
  TEMPERATURE,
  COLOR,
  MODE,
  TIMER_ON,
  TIMER_OFF,
  ONOFFLINE
};


enum mode_value{
  LIGHTNING = 0,
  READING,
  MEAL,
  MOVIE,
  PARTY,
  NIGHTLAMP
};

enum online_value{
  OFFLINE = 0,
  ONLINE
};

typedef struct deviceObj_s{
  String deviceID;
  String deviceName;
  String deviceType;
  String deviceVendor;
}deviceObj_t;

typedef struct deviceStatus_s{
  attribute_type attribute;
  String value;
}deviceStatus_t;

#endif
