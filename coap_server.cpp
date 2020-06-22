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
    cPacket.coapVersion = packetBuffer[0] & 196 >> 6;
    cPacket.type = packetBuffer[0] & 48 >> 4;
    cPacket.tokenlen = packetBuffer[0] & 15;
    cPacket.code = packetBuffer[1];
    cPacket.messageId = (packetBuffer[2] << 8) | packetBuffer[3];//łaczenie dwój bajtów
    //obsluga tokena
    if (cPacket.tokenlen > 0) {
      uint8_t token = new uint8_t[cPacket.tokenlen];
    } else {
      uint8_t token = NULL;
    }
    //obsługa opcji
    int optionNumber = 0;
    bool isNextOption = true;
    int currentByte = 4 + cPacket.tokenlen;
    while (isNextOption) {
      cPacket.cOption[optionNumber].delta = packetBuffer[currentByte] & 244 >> 4;
      cPacket.cOption[optionNumber].optionLength = packetBuffer[currentByte] & 15;
      //handling extended option and lnght
      if (cPacket.cOption[optionNumber].delta == 13) {
        currentByte++;
        cPacket.cOption[optionNumber].delta += packetBuffer[currentByte];
      }
      if (cPacket.cOption[optionNumber].optionLength == 13) {
        currentByte++;
        cPacket.cOption[optionNumber].optionLength += packetBuffer[currentByte];
      }
      Serial.print("delta");
      Serial.println(cPacket.cOption[optionNumber].delta);
      Serial.print("lenght");
      Serial.println(cPacket.cOption[optionNumber].optionLength);
      //reading option Value
      currentByte++;
      if (cPacket.cOption[optionNumber].optionLength > 0) {
        cPacket.cOption[optionNumber].optionValue = new uint8_t[cPacket.cOption[optionNumber].optionLength];
        for (int i = 0 ; i < cPacket.cOption[optionNumber].optionLength; i++) {
          cPacket.cOption[optionNumber].optionValue[i] = packetBuffer[currentByte];
          currentByte++;
        }
      }
      //checking if there are more options
      if (packetBuffer[currentByte] == NULL) {
        isNextOption = false;
      }
      if (packetBuffer[currentByte] == 255) {
        isNextOption = false;
      }
      optionNumber++;
      Serial.println("Petleka");
    }

    for(int i = 0 ; i < 5 ; i++){
      Serial.print("Coap option nr :");
      Serial.print(i);
      Serial.print(" delta of option: ");
      Serial.print(cPacket.cOption[i].delta);
      Serial.print(" lenght of option value is :");  
      Serial.print(cPacket.cOption[i].optionLength);
      Serial.println();
    }
    Serial.print("Size od udp");
    Serial.println(packetSize);
    Serial.print("current Byte");
    Serial.println(currentByte);
    //handling payload
    if(currentByte==255){
//      Cpacket.payload = new uint8_t[];
    }
    /*cPacket.cOption[optionNumber].delta = packetBuffer[4+cPacket.tokenlen]&244>>4;
      cPacket.cOption[optionNumber].optionLength = packetBuffer[4+cPacket.tokenlen]&15;
      //option handling
      int isOptionDeltaExtended = 0;
      if(cPacket.cOption[optionNumber].delta > 13){
      cPacket.cOption[optionNumber].delta+=packetBuffer[5+cPacket.tokenlen];
      isOptionDeltaExtended=1;
      }
      if(debug){Serial.println(cPacket.cOption[optionNumber].delta);}

      if(cPacket.cOption[optionNumber].optionLength==13){

      }else if(cPacket.cOption[optionNumber].optionLength==14){

      }
      if(cPacket.cOption[optionNumber].optionLength>0){
      cPacket.cOption[optionNumber].optionValue = new uint8_t[cPacket.cOption[optionNumber].optionLength];
      }
      for(int i = 0;i<cPacket.cOption[optionNumber].optionLength;i++){
      int currentPacket=
      cPacket.cOption[optionNumber].optionValue[i] = packetBuffer[3+cPacket.tokenlen+1+isOptionDeltaExtended+i];
      }
      Serial.print("kutas");
      for(int i = 0;i<cPacket.cOption[optionNumber].optionLength;i++){
      Serial.println((char)cPacket.cOption[optionNumber].optionValue[i]);
      }
      if(optionNumber=4){
      isNextOption=false;
      }*/
  
  //testy se kurwa robimy a co
  if (debug) {
    Serial.println();
  }
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
  //    Serial.print("optiondelta ");
  //    Serial.println(cPacket.cOption.delta);
  //    Serial.print("optionvaluelen ");
  //    Serial.println(cPacket.cOption.optionLength);

  //coapPacket coapPacket;
  //coapPacket.getMessageFromPacket();
}
}
