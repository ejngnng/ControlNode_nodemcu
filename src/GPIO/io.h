#ifndef __IO_H
#define __IO_H

#include <Arduino.h>
#include "common/common.h"


#define SW  5 //nodemcu D1

#define LED_SSDP  14  //nodemcu D5
#define LED_MQTT  12  //nodemcu D6
#define LED_RELAY 13  //nodemcu D7

#define LED_MQTT_ON   digitalWrite(LED_MQTT, LOW)
#define LED_MQTT_OFF  digitalWrite(LED_MQTT, HIGH)

#define LED_RELAY_ON  digitalWrite(LED_RELAY, LOW)
#define LED_RELAY_OFF digitalWrite(LED_RELAY, HIGH)

typedef enum state_s{
  ON,
  OFF
}state_t;


void portInit();
void TurnON(state_t *state);
void TurnOFF(state_t *state);


#endif
