#ifndef COAP_SERVER_H
#define COAP_SERVER_H
#include <ObirDhcp.h>           //dla pobierania IP z DHCP - proforma dla ebsim'a 
#include <ObirEthernet.h>       //niezbedne dla klasy 'ObirEthernetUDP'
#include <ObirEthernetUdp.h>
#define PACKET_BUFFER_LENGTH 100

class coapOption{
  public:
  uint8_t delta = 0;
  uint8_t optionLength = 0;  
  uint8_t *optionValue;
};
class coapResponse{
  public:
  uint8_t coapVersion;
  coapOption cOption[5];
  uint8_t type = 0;
  uint8_t code = 0;
  uint8_t *token = NULL;
  uint8_t tokenlen = 0;
  uint8_t *payload ;
  size_t payloadlen = 0;
  uint16_t  messageId = 0;
  uint8_t optionnum = 0;
};

class coapPacket{
  public:
  uint8_t coapVersion;
  coapOption cOption[5];
  uint8_t type = 0;
  uint8_t code = 0;
  uint8_t *token = NULL;
  uint8_t tokenlen = 0;
  uint8_t *payload ;
  size_t payloadlen = 0;
  uint16_t  messageId = 0;
  uint8_t optionnum = 0;
  coapResponse response;
  void bufferToPacket(uint8_t buffer[],int32_t packetlen);
  
};

class coapServer{
  public:
  ObirEthernetUDP Udp;
  unsigned char packetBuffer[PACKET_BUFFER_LENGTH];
  unsigned char packetMessage[PACKET_BUFFER_LENGTH]; 
  bool start();
  bool loop();
  bool addNumber(int newNumber,int currentSize);
  int zasob[5]={NULL, NULL, NULL, NULL, NULL};
};



#endif
