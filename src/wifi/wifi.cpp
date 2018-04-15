#include "wifi.h"


const char* _ssid = "ZQKL";
const char* _password = "zqkl123456..";

/* SmartConfig */
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
  WiFi.begin(wifi_value.wifi_ssid.c_str(), wifi_value.wifi_psw.c_str());
//  WiFi.begin(_ssid, _password);
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
  DEBUG_WiFi.printf("SSID: %s\n", _ssid);
  DEBUG_WiFi.println("IP address: ");
  DEBUG_WiFi.println(WiFi.localIP());
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

String getMAC(){
  unsigned char mac[6];
  memset(mac, 0, 6);
  WiFi.macAddress(mac);
  String temp = "";
  for(unsigned char i=0; i<6; i++){
    temp += String(mac[i], HEX);
  }
  return temp;
}
