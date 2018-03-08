/*
 * Description:  ssdp client 
 * 
 * Author: ninja
 * 
 * Date: 2017-05-08
 * 
 */
#include "SSDPClient.h"

//const char* ssdp_Search_Template = "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nST: upnp:rootdevice\r\nMAN: ssdp:discover\r\nMX: 1\r\n";
//unsigned int ssdp_port = 1900;

#define LWIP_OPEN_SRC
#include <functional>
#include "WiFiUdp.h"
#include "debug.h"

extern "C" {
  #include "osapi.h"
  #include "ets_sys.h"
  #include "user_interface.h"
}

#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "include/UdpContext.h"

//#define DEBUG_SSDP  Serial

#define SSDP_INTERVAL     2000
#define SSDP_PORT         1900
#define SSDP_METHOD_SIZE  10
#define SSDP_URI_SIZE     2
#define SSDP_BUFFER_SIZE  64
#define SSDP_MULTICAST_TTL 1

#define LED_SSDP  14 //nodemcu D5

volatile bool SSDP_Status = false;

static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);



static const char* _ssdp_response_template =
  "HTTP/1.1 200 OK\r\n"
  "EXT:\r\n";

static const char* _ssdp_notify_template =
  "NOTIFY * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "NTS: ssdp:alive\r\n";

static const char* _ssdp_msearch_template = 
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "ST: upnp:rootdevice\r\n"
  "MAN: ssdp:discover\r\n"
  "MX: 1\r\n";

static const char* _gssdp_notify_template =
  "NOTIFY * HTTP/1.1\r\n"
  "Host: 239.255.255.250:1900\r\n"
  "Cache-Control: max-age=2\r\n"
  "Location: 192.168.1.35\r\n"
  "Server: Linux/#970 SMP Mon Feb 20 19:18:29 GMT 2017 GSSDP/0.14.10\r\n"
  "NTS: ssdp:alive\r\n"
  "NT: upnp:rootdevice\r\n"
  "USN: uuid:5911c26e-ccc3c-5421-3721-b827eb3ea653::urn:schemas-upnp-org:service:voice-master:1\r\n";

static const char* _ssdp_packet_template =
  "%s" // _ssdp_response_template / _ssdp_notify_template
  "CACHE-CONTROL: max-age=%u\r\n" // SSDP_INTERVAL
  "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
  "USN: uuid:%s\r\n" // _uuid
  "%s: %s\r\n"  // "NT" or "ST", _deviceType
  "LOCATION: http://%u.%u.%u.%u:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
  "\r\n";

static const char* _ssdp_schema_template =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n"
  "Connection: close\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n"
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" // WiFi.localIP(), _port
    "<device>"
      "<deviceType>%s</deviceType>"
      "<friendlyName>%s</friendlyName>"
      "<presentationURL>%s</presentationURL>"
      "<serialNumber>%s</serialNumber>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>%s</modelURL>"
      "<manufacturer>%s</manufacturer>"
      "<manufacturerURL>%s</manufacturerURL>"
      "<UDN>uuid:%s</UDN>"
    "</device>"
//    "<iconList>"
//      "<icon>"
//        "<mimetype>image/png</mimetype>"
//        "<height>48</height>"
//        "<width>48</width>"
//        "<depth>24</depth>"
//        "<url>icon48.png</url>"
//      "</icon>"
//      "<icon>"
//       "<mimetype>image/png</mimetype>"
//       "<height>120</height>"
//       "<width>120</width>"
//       "<depth>24</depth>"
//       "<url>icon120.png</url>"
//      "</icon>"
//    "</iconList>"
  "</root>\r\n"
  "\r\n";


struct SSDPTimer {
  ETSTimer timer;
};

SSDPClass::SSDPClass() :
_server(0),
_timer(new SSDPTimer),
_port(80),
_ttl(SSDP_MULTICAST_TTL),
_respondToPort(0),
_pending(false),
_delay(0),
_process_time(0),
_notify_time(0),
_location("")

{
  _uuid[0] = '\0';
  _modelNumber[0] = '\0';
  sprintf(_deviceType, "urn:schemas-upnp-org:device:Basic:1");
  _friendlyName[0] = '\0';
  _presentationURL[0] = '\0';
  _serialNumber[0] = '\0';
  _modelName[0] = '\0';
  _modelURL[0] = '\0';
  _manufacturer[0] = '\0';
  _manufacturerURL[0] = '\0';
  sprintf(_schemaURL, "ssdp/schema.xml");
}

SSDPClass::~SSDPClass(){
  delete _timer;
}

bool SSDPClass::begin(){
  _pending = false;
#ifdef DEBUG_SSDP
  DEBUG_SSDP.println("ssdp begin function....");
#endif
  uint32_t chipId = ESP.getChipId();
  sprintf(_uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
    (uint16_t) ((chipId >> 16) & 0xff),
    (uint16_t) ((chipId >>  8) & 0xff),
    (uint16_t)   chipId        & 0xff  );

#ifdef DEBUG_SSDP
  DEBUG_SSDP.printf("SSDP UUID: %s\n", (char *)_uuid);
#endif

  if (_server) {
    _server->unref();
    _server = 0;
  }

  _server = new UdpContext;
  _server->ref();

  ip_addr_t ifaddr;
  ifaddr.addr = WiFi.localIP();
  ip_addr_t multicast_addr;
  multicast_addr.addr = (uint32_t) SSDP_MULTICAST_ADDR;
  if (igmp_joingroup(&ifaddr, &multicast_addr) != ERR_OK ) {
#ifdef DEBUG_SSDP
    DEBUG_SSDP.println("SSDP failed to join igmp group ....");
    DEBUGV("SSDP failed to join igmp group");
#endif
    return false;
  }
  if (!_server->listen(*IP_ADDR_ANY, SSDP_PORT)) {
    return false;
  }

  _server->setMulticastInterface(ifaddr);
  _server->setMulticastTTL(_ttl);
  _server->onRx(std::bind(&SSDPClass::_update, this));
  if (!_server->connect(multicast_addr, SSDP_PORT)) {
    return false;
  }

  _startTimer();

  return true;
}


void SSDPClass::_send(ssdp_method_t method){
  char buffer[1460];
  uint32_t ip = WiFi.localIP();
  int len = snprintf(buffer, sizeof(buffer),
    _ssdp_msearch_template
  );
  Serial.println("======================");
  Serial.println(buffer);
  Serial.println(len);
  Serial.println("======================");
  _server->append(buffer, len);

  ip_addr_t remoteAddr;
  uint16_t remotePort;
    if(method == NONE){
#ifdef DEBUG_SSDP
      DEBUG_SSDP.println("method is NONE ...");
#endif
    }
    if(method == NOTIFY){
#ifdef DEBUG_SSDP
      DEBUG_SSDP.println("method is NOTIFY ...");
#endif
    }

    if(method == SEARCH){
#ifdef DEBUG_SSDP
      DEBUG_SSDP.println("method is SEARCH ...");
#endif
    }
    
    remoteAddr.addr = SSDP_MULTICAST_ADDR;
    remotePort = SSDP_PORT;
#ifdef DEBUG_SSDP
    DEBUG_SSDP.println("Sending Notify to ");
    DEBUG_SSDP.print(IPAddress(remoteAddr.addr));
    DEBUG_SSDP.print(":");
    DEBUG_SSDP.println(remotePort);
#endif

  _server->send(&remoteAddr, remotePort);
}

void SSDPClass::schema(WiFiClient client){
  uint32_t ip = WiFi.localIP();
  Serial.println("enter schema function ...");
  client.printf(_ssdp_schema_template,
    IP2STR(&ip), _port,
    _deviceType,
    _friendlyName,
    _presentationURL,
    _serialNumber,
    _modelName,
    _modelNumber,
    _modelURL,
    _manufacturer,
    _manufacturerURL,
    _uuid
  );
}

void SSDPClass::_parsePacket(String gssdp_notify_template){
    
    int Location_Crs = gssdp_notify_template.indexOf("Location:");
    int Server_Crs = gssdp_notify_template.indexOf("Server:");
    int NTS_Crs = gssdp_notify_template.indexOf("NTS:");
    int NT_Crs = gssdp_notify_template.indexOf("NT:");
    int USN_Crs = gssdp_notify_template.indexOf("USN:");

#ifdef DEBUG_SSDP
    Serial.println("start===================================");
#endif

    String locationValue = gssdp_notify_template.substring(Location_Crs,Server_Crs).substring(String("Location:").length());
    locationValue.trim();

    String ntsValue = gssdp_notify_template.substring(NTS_Crs,NT_Crs).substring(String("NTS:").length());
    ntsValue.trim();
    
    String ntValue = gssdp_notify_template.substring(NT_Crs,USN_Crs).substring(String("NT:").length());
    ntValue.trim();
    
    String usnValue = gssdp_notify_template.substring(USN_Crs);
    int Service_Crs = usnValue.indexOf("service:");
    String serviceValue = usnValue.substring(Service_Crs).substring(String("service:").length());
    serviceValue.trim();

#ifdef DEBUG_SSDP
    DEBUG_SSDP.print("location Value: ");
    DEBUG_SSDP.println(locationValue);
    DEBUG_SSDP.print("NTS Value: ");
    DEBUG_SSDP.println(ntsValue);
    DEBUG_SSDP.print("NT Value: ");
    DEBUG_SSDP.println(ntValue);
    DEBUG_SSDP.print("service Value: ");
    DEBUG_SSDP.println(serviceValue);
#endif
    if(serviceValue.compareTo("voice-master:1") == 0){
#ifdef DBUG_SSDP
      DEBUG_SSDP.println("set mqtt server...");
#endif
      _setLocation(locationValue);
    }
 #ifdef DEBUG_SSDP
    Serial.println("end===================================");
 #endif
}

void SSDPClass::_update(){
  SSDP_Status = !SSDP_Status;
  digitalWrite(LED_SSDP, (SSDP_Status)?HIGH:LOW);
  if(_server->next()) {

//    _respondToAddr = _server->getRemoteAddress();
//    _respondToPort = _server->getRemotePort();

    String notifyBuffer = "";
    boolean msg_state = false;
    
    while(_server->getSize() > 0){
      char c = _server->read();
      notifyBuffer += c;
      if(c == '*'){
        if(notifyBuffer.startsWith("NOTIFY")){
          msg_state = true;
        }else if(notifyBuffer.startsWith("M-SEARCH")){
          msg_state = false;
          return;
        }else{
          msg_state = false;
          return;
        }
      }
    }
    
    if(msg_state){
      _parsePacket(notifyBuffer);
    }

  }

  if((millis() - _process_time) > _delay){
    _pending = false; _delay = 0;
//    _send(NONE);
  } else if(_notify_time == 0 || (millis() - _notify_time) > (SSDP_INTERVAL * 1000L)){
    _notify_time = millis();
//    _send(NOTIFY);
  }

  if (_pending) {
    while (_server->next())
      _server->flush();
  }

}


void SSDPClass::setSchemaURL(const char *url){
  strlcpy(_schemaURL, url, sizeof(_schemaURL));
}

void SSDPClass::setHTTPPort(uint16_t port){
  _port = port;
}

void SSDPClass::setDeviceType(const char *deviceType){
  strlcpy(_deviceType, deviceType, sizeof(_deviceType));
}

void SSDPClass::setName(const char *name){
  strlcpy(_friendlyName, name, sizeof(_friendlyName));
}

void SSDPClass::setURL(const char *url){
  strlcpy(_presentationURL, url, sizeof(_presentationURL));
}

void SSDPClass::setSerialNumber(const char *serialNumber){
  strlcpy(_serialNumber, serialNumber, sizeof(_serialNumber));
}

void SSDPClass::setSerialNumber(const uint32_t serialNumber){
  snprintf(_serialNumber, sizeof(uint32_t)*2+1, "%08X", serialNumber);
}

void SSDPClass::setModelName(const char *name){
  strlcpy(_modelName, name, sizeof(_modelName));
}

void SSDPClass::setModelNumber(const char *num){
  strlcpy(_modelNumber, num, sizeof(_modelNumber));
}

void SSDPClass::setModelURL(const char *url){
  strlcpy(_modelURL, url, sizeof(_modelURL));
}

void SSDPClass::setManufacturer(const char *name){
  strlcpy(_manufacturer, name, sizeof(_manufacturer));
}

void SSDPClass::setManufacturerURL(const char *url){
  strlcpy(_manufacturerURL, url, sizeof(_manufacturerURL));
}

void SSDPClass::setTTL(const uint8_t ttl){
  _ttl = ttl;
}

void SSDPClass::_setLocation(String location){
  _location = location;
}

String SSDPClass::getLocation(){
  return _location;
}

void SSDPClass::_onTimerStatic(SSDPClass* self) {
  self->_update();
}

void SSDPClass::_startTimer() {
  ETSTimer* tm = &(_timer->timer);
  const int interval = 1000;
  os_timer_disarm(tm);
  os_timer_setfn(tm, reinterpret_cast<ETSTimerFunc*>(&SSDPClass::_onTimerStatic), reinterpret_cast<void*>(this));
  os_timer_arm(tm, interval, 1 /* repeat */);
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SSDP)
SSDPClass SSDPClient;
#endif
