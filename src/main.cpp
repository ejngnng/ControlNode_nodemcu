
/*
 * Description:  control relay with ssdp and mqtt
 *
 * Author: ninja
 *
 * Date: 2017-05-06
 *
 */

#include <PubSubClient.h>
#include "SSDP/SSDPClient.h"
#include <ESP8266WebServer.h>

#include "wifi/wifi.h"
#include "Operate/deviceManipulation.h"
#include "GPIO/io.h"


const char* mqtt_server = "www.futureSmart.top";

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
deviceManipulation devicMp(&MQTTClient);


#define SSDP_PORT 1883

const char* subTopic = "device/device_operate";
const char* pubTopic = "device/device_register";

const char* device_name = "switch2";



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

// global variables
state_t sw_state = OFF;

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




bool jsonPaser(byte* payload){

// detect duplicate msg
  unsigned long currentMsg = millis();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& data = jsonBuffer.parse(payload);
  if(!data.success()){
    return false;
  }
  const char* name = data["name"];
  const char* target_id = data["target_id"];
  const char* action = data["action"];
  const char* value = data["value"];

#ifdef DEBUG_JSON
    DEBUG_JSON.println(name);
    DEBUG_JSON.println(target_id);
    DEBUG_JSON.println(action);
    DEBUG_JSON.println(value);
#endif
  if(!strcmp(target_id, device_name)){
    if(!strcmp(value,"0")){
      TurnOFF(&sw_state);
    }else if(!strcmp(value,"1")){
      TurnON(&sw_state);
    }else{
      return false;
    }
  }else{
    return false;
  }
  return true;

}

void callback(char* topic, byte* payload, unsigned int length){
#ifdef DEBUG_MQTT
  DEBUG_MQTT.print(TimeStamp + "Message arrived [");
  DEBUG_MQTT.print(topic);
  DEBUG_MQTT.print("] ");

  for (int i = 0; i < length; i++) {
    DEBUG_MQTT.print((char)payload[i]);
  }
  DEBUG_MQTT.println();
#endif

  //jsonPaser(payload);
  if(strcmp(topic, "device/device_operate") == 0)
    devicMp.deviceOperate(payload);

}

bool MQTTSetup(){
  Serial.println(TimeStamp + "start MQTTSetup...");
#if 0
  char mqtt_server[255] = "";
  String location = SSDPClient.getLocation();
  if(location == ""){
    return false;
  }
  location.toCharArray(mqtt_server,location.length()+1);
#endif
#ifdef DEBUG_MQTT
  DEBUG_MQTT.print(TimeStamp + "mqtt server: ");
  DEBUG_MQTT.println(mqtt_server);
#endif
  MQTTClient.setServer(mqtt_server, SSDP_PORT);
  MQTTClient.setCallback(callback);
  return true;
}

void reconnect() {
  // Loop until we're reconnected
  if(!MQTTClient.connected()) {
 #ifdef DEBUG_MQTT
    DEBUG_MQTT.println(TimeStamp + "Attempting MQTT connection...");
 #endif
    MQTTSetup();
    // Create client name
    String clientId = "ESP8266-";
    clientId += device_name;
    // Attempt to connect
    if (MQTTClient.connect(clientId.c_str())) {
#ifdef DEBUG_MQTT
      DEBUG_MQTT.println(TimeStamp + "mqtt broker connected");
#endif
      // Once connected, publish an announcement...
      MQTTClient.publish(pubTopic, "hello world");
      //device register
      //MQTTClient.publish(pubTopic, "{\"type\":\"lamp\",\"vendor\":\"ht\",\"MAC\":\"2c3ae82205b1\"}");
      devicMp.deviceRegister();

      // ... and resubscribe
      MQTTClient.subscribe(subTopic);
    } else {

#ifdef DEBUG_MQTT
      DEBUG_MQTT.print(TimeStamp + "failed, rc=");
      DEBUG_MQTT.print(MQTTClient.state());
      DEBUG_MQTT.println(" try again in 5 seconds");
#endif
      //delay(1000);
    }
  }
}

void SSDPClientSetup(){
    SSDPClient.schema(espClient);
    SSDPClient.begin();
}


void setup(){
  serial_init();

  portInit();

  //Connect_WiFi();
  SmartConfig();

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
