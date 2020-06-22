#include "coap_server.h"

#define PACKET_BUFFER_LENGTH        100
#define UDP_SERVER_PORT 1234
bool debug = true;
byte MAC[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a
unsigned int localPort = UDP_SERVER_PORT;
bool coapServer::start() {

  ObirEthernet.begin(MAC);
  Serial.print(F("My IP address: "));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(ObirEthernet.localIP()[thisByte], DEC); Serial.print(F("."));
  }
  Serial.println();
  Udp.begin(localPort);
}
bool coapServer::loop() {
  //Serial.println("Petelka kurwy");
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    Serial.println("Witam udp ty kurwo jebana");
    int len = Udp.read(packetBuffer, PACKET_BUFFER_LENGTH);
    Serial.print("Recieved: ");
    packetBuffer[len] = '\0';
    packetMessage[len] = '\0';
    Serial.println((char*)packetBuffer);
    //for(int i=0;i<packetSize;i++){
    //    Serial.println(packetBuffer[i]);
    //}
    coapPacket cPacket;
    cPacket.coapVersion=packetBuffer[0]&196>>6;
    cPacket.type=packetBuffer[0]&48>>4;
    cPacket.tokenlen=packetBuffer[0]&15;
    cPacket.code = packetBuffer[1];
    cPacket.messageId = (packetBuffer[2] << 8) | packetBuffer[3];//łaczenie dwój bajtów 
    //obsluga tokena 
    if(cPacket.tokenlen>0){
      uint8_t token = new uint8_t[cPacket.tokenlen];
    }else{
      uint8_t token = NULL;
    }
    //obsługa opcji
    cPacket.cOption.delta = packetBuffer[4+cPacket.tokenlen]&244>>4;
    cPacket.cOption.optionLength = packetBuffer[4+cPacket.tokenlen]&15;
    if(debug){Serial.print("delta przed");Serial.println(cPacket.cOption.delta);}
    int isOptionDeltaExtended = 0;
    if(cPacket.cOption.delta > 13){
      cPacket.cOption.delta+=packetBuffer[5+cPacket.tokenlen];  
      isOptionDeltaExtended=1;
    }
    if(debug){Serial.println(cPacket.cOption.delta);}
    
    if(cPacket.cOption.optionLength==13){
      
    }else if(cPacket.cOption.optionLength==14){
      
    }
    if(cPacket.cOption.optionLength>0){
      cPacket.cOption.optionValue = new uint8_t[cPacket.cOption.optionLength];  
    }
    for(int i = 0;i<cPacket.cOption.optionLength;i++){
      cPacket.cOption.optionValue[i] = packetBuffer[3+cPacket.tokenlen+1+isOptionDeltaExtended+i];
    }
    Serial.print("kutas");
    for(int i = 0;i<cPacket.cOption.optionLength;i++){
      Serial.println((char)cPacket.cOption.optionValue[i]);
    }
    //testy se kurwa robimy a co
    if(debug){Serial.println();}
    Serial.print("coapver ");
    Serial.println(cPacket.coapVersion);
    Serial.print("code ");
    Serial.println(cPacket.code);
    Serial.print("tokenlen ");
    Serial.println(cPacket.tokenlen);
    Serial.print("type ");
    Serial.println(+cPacket.type);
    Serial.print("messageId ");
    Serial.println(cPacket.messageId);
    Serial.print("optiondelta ");
    Serial.println(cPacket.cOption.delta);
    Serial.print("optionvaluelen ");
    Serial.println(cPacket.cOption.optionLength);

    //coapPacket coapPacket;
    //coapPacket.getMessageFromPacket();
  }
}
