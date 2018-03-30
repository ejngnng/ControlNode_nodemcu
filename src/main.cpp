
/*
 * Description:  control relay with ssdp and mqtt
 *
 * Author: ninja
 *
 * Date: 2017-05-06
 *
 */

#include "SSDP/SSDPClient.h"
#include <ESP8266WebServer.h>

#include "wifi/wifi.h"
#include "Operate/deviceManipulation.h"
#include "GPIO/io.h"
#include "MQTT/mqtt.h"



extern WiFiClient espClient;
extern PubSubClient MQTTClient;
extern deviceManipulation devicMp;

extern state_t sw_state;

//volatile bool MQTT_Status = false;

// pub msg variables
unsigned long previousMillis = 0;
unsigned long aliveInterval = 20; // 20 ms
String aliveMsg = "";

typedef struct Sub_Msg_s{
  const char* TopicType;
  const char* target_id;
  const char* action;
  const char* value;
}Sub_Msg_t;

typedef struct Device_Obj_s{
  const char* deviceName;
  const char* deviceID;
  state_t state;
}Device_Obj_t;



#if 0
String gssdp_notify_template =
  "NOTIFY * HTTP/1.1\r\n"
  "Host: 239.255.255.250:1900\r\n"
  "Cache-Control: max-age=2\r\n"
  "Location: 192.168.1.35\r\n"
  "Server: Linux/#970 SMP Mon Feb 20 19:18:29 GMT 2017 GSSDP/0.14.10\r\n"
  "NTS: ssdp:alive\r\n"
  "NT: upnp:rootdevice\r\n"
  "USN: uuid:5911c26e-ccc3c-5421-3721-b827eb3ea653::urn:schemas-upnp-org:service:voice-master:1\r\n";

#endif



void SSDPClientSetup(){
    SSDPClient.schema(espClient);
    SSDPClient.begin();
}


void setup(){
  serial_init();

  portInit();

  Connect_WiFi();
  //SmartConfig();

  //SSDPClientSetup();
  MQTTSetup();

  Serial.println(TimeStamp + "Setup Done ...");
}

void loop(){

  if(!MQTTClient.connected()){
    LED_MQTT_OFF;
    reconnect();
  }else{
    LED_MQTT_ON;
  }

  if(MQTTClient.connected())
  {
    MQTTClient.loop();

  //Attempting to keepalive
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis > aliveInterval) {
      previousMillis = currentMillis;
      aliveMsg = "Device is alive";
      //MQTTClient.publish(pubTopic, aliveMsg.c_str());
    }
  }

//get cmd from uart
  char onoff;
  while(Serial.available()>0){
    onoff = Serial.read();//读串口第一个字节
    Serial.print("Serial.read: ");
    Serial.print(onoff);

    if(onoff == 0x55)
      TurnON(&sw_state);
    else if(onoff == 0x56)
      TurnOFF(&sw_state);
    delay(100);
    }

}
