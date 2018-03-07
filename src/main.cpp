
/*
 * Description:  control relay with ssdp and mqtt 
 * 
 * Author: ninja
 * 
 * Date: 2017-05-06
 * 
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SSDPClient.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include <WiFiManager.h>
#include <DNSServer.h>

#include <EEPROM.h>

#include "deviceManipulation.h"

const char* ssid = "ZQKL";
const char* password = "zqkl123456..";
//const char* ssid = "Xiaomi_xs";
//const char* password = "chaotongjiayuan";

const char* mqtt_server = "www.futureSmart.top";

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
deviceManipulation devicMp(&MQTTClient);

#define DEBUG_WiFi  Serial
#define DEBUG_MQTT  Serial
//#define DEBUG_JSON  Serial
//#define DEBUG_SW    Serial

#define SSDP_PORT 1883

const char* subTopic = "device/device_operate";
const char* pubTopic = "device/device_register";

const char* device_name = "switch2";

#define SW  5 //nodemcu D1

#define LED_SSDP  14  //nodemcu D5
#define LED_MQTT  12  //nodemcu D6
#define LED_RELAY 13  //nodemcu D7

#define LED_MQTT_ON   digitalWrite(LED_MQTT, LOW)
#define LED_MQTT_OFF  digitalWrite(LED_MQTT, HIGH)

#define LED_RELAY_ON  digitalWrite(LED_RELAY, LOW)
#define LED_RELAY_OFF digitalWrite(LED_RELAY, HIGH)

#define WiFi_SSID_LEN_ADDR  0
#define WiFi_PSW_LEN_ADDR   1

#define WiFi_SSID_ADDR      2
#define WiFi_PSW_ADDR      34


#define TimeStamp   (String("[") + String(millis()) + String("] "))

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

typedef enum SW_state_s{
  ON,
  OFF
}SW_state_t;

typedef struct Device_Obj_s{
  const char* deviceName;
  const char* deviceID;
  SW_state_t state;
}Device_Obj_t;

typedef struct WiFi_Obj_s{
  unsigned int ssid_len;
  unsigned int psw_len;
  String       wifi_ssid;
  String       wifi_psw;
}WiFi_Obj_t;

// global variables
SW_state_t sw_state = OFF; 

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

void TurnON(){
  if(sw_state == ON){
    // already on
    return;
  }
  digitalWrite(SW, HIGH);
  LED_RELAY_ON;
  sw_state = ON;
  
#ifdef DEBUG_SW
  DEBUG_SW.println(TimeStamp + "turn On light...");
#endif
}

void TurnOFF(){
  if(sw_state == OFF){
    // already off
    return;
  }
  digitalWrite(SW, LOW);
  LED_RELAY_OFF;
  sw_state = OFF;
  
#ifdef DEBUG_SW
  DEBUG_SW.println(TimeStamp + "turn Off light...");
#endif
}


void clear_eeprom(){
  EEPROM.begin(512);
  for(unsigned int i=0;i<512;i++){
    EEPROM.write(i,0);
  }
  EEPROM.end();
}

void store_wifi(){
  unsigned int ssid_len = WiFi.SSID().length();
  unsigned int psw_len  = WiFi.psk().length();
  EEPROM.begin(512);
  
  EEPROM.write(WiFi_SSID_LEN_ADDR, ssid_len);
  EEPROM.write(WiFi_PSW_LEN_ADDR, psw_len);
  
  String ssid = WiFi.SSID();
  String psw = WiFi.psk();

  for(unsigned int i=0;i<ssid_len;i++){
    EEPROM.write(WiFi_SSID_ADDR+i, ssid[i]);
  }

  for(unsigned int j=0;j<psw_len;j++){
    EEPROM.write(WiFi_PSW_ADDR+j, psw[j]);
  }
  
  EEPROM.commit();
  EEPROM.end();
  
}

void get_wifi(WiFi_Obj_t *wifi_obj){
  String ssid;
  String psw;
  EEPROM.begin(512);
  unsigned int ssid_len = EEPROM.read(WiFi_SSID_LEN_ADDR);
  unsigned int psw_len = EEPROM.read(WiFi_PSW_LEN_ADDR);

  for(unsigned int i=0; i<ssid_len; i++){
    ssid += (char)EEPROM.read(WiFi_SSID_ADDR + i);
  }

  for(unsigned int j=0; j<psw_len; j++){
    psw += (char)EEPROM.read(WiFi_PSW_ADDR + j);
  }

  EEPROM.commit();
  EEPROM.end();
  wifi_obj->ssid_len = ssid_len;
  wifi_obj->psw_len = psw_len;
  wifi_obj->wifi_ssid = ssid;
  wifi_obj->wifi_psw = psw;

#ifdef DEBUG_WiFi
    DEBUG_WiFi.println("get wifi from eeprom...");
    DEBUG_WiFi.printf("ssid len: %d\n", ssid_len);
    DEBUG_WiFi.printf("psw len: %d\n", psw_len);
    DEBUG_WiFi.printf("ssid: %s\n", ssid.c_str());
    DEBUG_WiFi.printf("psw: %s\n", psw.c_str());
#endif

}

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
      TurnOFF();
    }else if(!strcmp(value,"1")){
      TurnON();
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

void SmartConfig(){
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  Serial.println("attempt to connecting...");
  while(1){
    Serial.print(".");
    delay(500);
    if(WiFi.smartConfigDone()){
      digitalWrite(LED_BUILTIN, HIGH);
#ifdef DEBUG_WiFi
      DEBUG_WiFi.println();
      DEBUG_WiFi.println("SmartConfig succes!!!");
      DEBUG_WiFi.printf("SSID: %s\r\n", WiFi.SSID().c_str());
      DEBUG_WiFi.printf("PSW: %s\r\n", WiFi.psk().c_str());
#endif
      clear_eeprom();
      store_wifi();
      break;
    }
  }
}

void Connect_WiFi(){

  WiFi_Obj_t wifi_value;
  get_wifi(&wifi_value);

  WiFi.mode(WIFI_STA);
  //WiFi.begin(wifi_value.wifi_ssid.c_str(), wifi_value.wifi_psw.c_str());
  WiFi.begin(ssid, password);
  unsigned int timeOut = 10;

  while(timeOut --){
    if(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
    }
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("wifi already connected...");
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    }
    delay(1000);
  }

  if(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    SmartConfig();
  }

#ifdef DEBUG_WiFi
  DEBUG_WiFi.println("");
  DEBUG_WiFi.printf("SSID: %s\n", ssid);
  DEBUG_WiFi.println("IP address: ");
  DEBUG_WiFi.println(WiFi.localIP());
#endif
}

void setup(){
  Serial.begin(9600);
  
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
      TurnON();
    else if(onoff == 0x56)
      TurnOFF();
    delay(100);
    } 

}

