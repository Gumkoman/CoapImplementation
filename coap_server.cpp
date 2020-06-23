#include "coap_server.h"

#define PACKET_BUFFER_LENGTH        100
#define UDP_SERVER_PORT 1234
bool debug = true;
byte MAC[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a
unsigned int localPort = UDP_SERVER_PORT;

bool isEqual(char* text1, char* text2, int textSize) {
  for (int i = 0; i < textSize; i++) {
    if (text1[i] != text2[i]) {
      return false;
    }
    return true;
  }

}
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
    }//dodac czytanie tokena xd casme moze byc inny niz 0
    for (int i = 0; i < cPacket.tokenlen; i++) {
      cPacket.token[i] = packetBuffer[4 + i];
    }
    //obsługa opcji
    int optionNumber = 0;
    bool isNextOption = true;
    int currentByte = 4 + cPacket.tokenlen;
    while (isNextOption) {
      cPacket.cOption[optionNumber].delta = (packetBuffer[currentByte] & 244 ) >> 4;
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
    }
    for (int i = 0 ; i < 5 ; i++) {
      Serial.print("Coap option nr :");
      Serial.print(i);
      Serial.print(" delta of option: ");
      Serial.print(cPacket.cOption[i].delta);
      Serial.print(" lenght of option value is :");
      Serial.print(cPacket.cOption[i].optionLength);
      Serial.println();
    }

    //handling payload
    uint8_t payloadMarker = 255;
    if (packetBuffer[currentByte] == payloadMarker) {
      Serial.print("LUTAS");
      int nextByte = currentByte;
      while (packetBuffer[nextByte] != NULL) {
        nextByte++;
      }

      Serial.print("nextByte");
      Serial.println(nextByte);
      cPacket.payload = new uint8_t[nextByte - currentByte - 1];
      for (int i = 0; i < (nextByte - currentByte - 1); i++) {
        cPacket.payload[i] = packetBuffer[i + currentByte + 1];

      }



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
    Serial.println();
    //handeled recived message now givew a response
    if (cPacket.code == 1) {
      Serial.println("GET");
      if (isEqual(".well-known", cPacket.cOption[0].optionValue, 11)) {
        if (isEqual("core", cPacket.cOption[1].optionValue, 4)) {
          Serial.println("getwellknowncore");
          cPacket.response.coapVersion = cPacket.coapVersion;
          uint8_t response[100];
          response[0] = cPacket.coapVersion << 6 | 4 << 4 | cPacket.tokenlen;
          response[1] = 69;
          response[2] = cPacket.messageId >> 8;
          response[3] = cPacket.messageId;
          int currentByte = 4;
          response[currentByte] = 194;//
          currentByte++;
          response[currentByte] = 0;
          currentByte++;
          response[currentByte] = 40;
          currentByte++;
          response[currentByte] = 255;
          char payload[] = "</zbior>;if=zbior;</metryka1>;if=metryka1;</metryka2>;if=metryka2;</metryka3>;if=metryka3;";
          currentByte++;
          for (int i = 0; i < sizeof(payload); i++) {
            response[currentByte] = payload[i];
            currentByte++;
          }
          //cPacket.response.token = cPacket.token;
          //cPacket.response.cOption[0].delta =12;
          //cPacket.response.cOption[0].optionLength =2;
          //cPacket.response.payload = "</zbior>;if=zbior;</metryka1>;if=metryka1;</metryka2>;if=metryka2;</metryka3>;if=metryka3;";
          Serial.println("Pokaz wiadmosci");
          for (int i = 0; i < sizeof(response); i++) {
            Serial.print(response[i]);
            Serial.print(" ");
          }
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write(response, sizeof(response));
          Udp.endPacket();

        }
      }

      if (isEqual("zbior", cPacket.cOption[0].optionValue, 5)) {
        //get zbior
        Serial.println("zbior");
        uint8_t response[100];
        response[0] = cPacket.coapVersion << 6 | 4 << 4 | cPacket.tokenlen;
        response[1] = 69;
        response[2] = cPacket.messageId >> 8;
        response[3] = cPacket.messageId;
        int currentByte = 4;
        if (cPacket.tokenlen > 0) {
          for (int i = 0; i < cPacket.tokenlen; i++) {
            response[currentByte] = cPacket.token[i];
            currentByte++;
          }
        }
        response[currentByte] = 194;
        currentByte++;
        response[currentByte] = 0;
        currentByte++;
        response[currentByte] = 0;
        char *payload;
        if (cPacket.token == 1) {
          Serial.println("NWW NWD");
        } else if (cPacket.token == 0) {
          Serial.println("POKAZANIE ZBIORU");
          if (this->zasob[0] == NULL) {
            payload = "ZbiorPusty";
            Serial.println("ZbiorPusty");
          } else {
            int i =0;
            Serial.println("cos jest");
            
          }
        }
        Serial.println("Payload");
        Serial.print(*payload);  
        
      }



    }



    if (cPacket.code == 3) {
      Serial.println("PUT");
    }


  }


}
