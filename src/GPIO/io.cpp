#include "io.h"


void portInit(){
  pinMode(SW, OUTPUT);
  digitalWrite(SW, LOW);

  pinMode(LED_SSDP, OUTPUT);
  digitalWrite(LED_SSDP, HIGH);

  pinMode(LED_MQTT, OUTPUT);
  digitalWrite(LED_MQTT, HIGH);

  pinMode(LED_RELAY, OUTPUT);
  digitalWrite(LED_RELAY, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

}


void TurnON(state_t *state){
  if(*state == ON){
    // already on
    return;
  }
  digitalWrite(SW, HIGH);
  LED_RELAY_ON;
  *state = ON;

#ifdef DEBUG_SW
  DEBUG_SW.println(TimeStamp + "turn On light...");
#endif
}

void TurnOFF(state_t *state){
  if(*state == OFF){
    // already off
    return;
  }
  digitalWrite(SW, LOW);
  LED_RELAY_OFF;
  *state = OFF;

#ifdef DEBUG_SW
  DEBUG_SW.println(TimeStamp + "turn Off light...");
#endif
}
