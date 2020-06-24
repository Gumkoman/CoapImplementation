#include "coap_server.h"
#include <math.h>
#define PACKET_BUFFER_LENGTH        100
#define UDP_SERVER_PORT 1234
bool debug = true;
byte MAC[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a
unsigned int localPort = UDP_SERVER_PORT;

bool coapServer::addNumber(int newNumber, int currentSize) {
  this->zasob[currentSize] = newNumber;
}

bool isEqual(char* text1, char* text2, int textSize) {
  for (int i = 0; i < textSize; i++) {
    if (text1[i] != text2[i]) {
      return false;
    }
    return true;
  }

}
int gcd(int a, int b) 
{ 
    if (a == 0) 
        return b; 
    return gcd(b % a, a); 
}

int findlcm(int arr[], int n) 
{ 
    // Initialize result 
    int ans = arr[0]; 
  
    // ans contains LCM of arr[0], ..arr[i] 
    // after i'th iteration, 
    for (int i = 1; i < n; i++) 
        ans = (((arr[i] * ans)) / 
                (gcd(arr[i], ans))); 
  
    return ans; 
}

int findGCD(int arr[], int n) 
{ 
    int result = arr[0]; 
    for (int i = 1; i < n; i++) 
    { 
        result = gcd(arr[i], result); 
  
        if(result == 1) 
        { 
           return 1; 
        } 
    } 
    return result; 
} 
char * toArray(unsigned int number) {
    int length = (int)floor(log10((float)number)) + 1;
    char * arr = new char[length];
    int i = 0;
    do {
        arr[i] = number % 10;
        number /= 10;
        i++;
    } while (number != 0);
    return arr;
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

    //handling payload
    uint8_t payloadMarker = 255;
    if (packetBuffer[currentByte] == payloadMarker) {
      int nextByte = currentByte;
      while (packetBuffer[nextByte] != NULL) {
        nextByte++;
      }
      cPacket.payload = new uint8_t[nextByte - currentByte - 1];
      for (int i = 0; i < (nextByte - currentByte - 1); i++) {
        cPacket.payload[i] = packetBuffer[i + currentByte + 1];

      }



    }
    /*Serial.print("coapver ");
      Serial.println(cPacket.coapVersion);
      Serial.print("code ");
      Serial.println(cPacket.code);
      Serial.print("tokenlen ");
      Serial.println(cPacket.tokenlen);
      Serial.print("type ");
      Serial.println(+cPacket.type);
      Serial.print("messageId ");
      Serial.println(cPacket.messageId);
      Serial.println();*/
    //handeled recived message now givew a response
    if (cPacket.code == 1) {
      if (isEqual(".well-known", cPacket.cOption[0].optionValue, 11)) {
        if (isEqual("core", cPacket.cOption[1].optionValue, 4)) {
          cPacket.response.coapVersion = cPacket.coapVersion;
          uint8_t response[100] = {32};
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
          Serial.println("Pokaz wiadmosci");
          for (int i = 0; i < sizeof(response); i++) {
            Serial.print(response[i]);
            Serial.print(" ");
          }
          free(payload);
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
          Udp.write(response, sizeof(response));
          Udp.endPacket();
          free(response);
        }
      }

      if (isEqual("zbior", cPacket.cOption[0].optionValue, 5)) {
        //get zbior
        Serial.println("zbior");
        byte response1[100];
        response1[0] = cPacket.coapVersion << 6 | 4 << 4 | cPacket.tokenlen;
        response1[1] = 69;
        response1[2] = cPacket.messageId >> 8;
        response1[3] = cPacket.messageId;
        int currentByte = 4;
        if (cPacket.tokenlen > 0) {
          for (int i = 0; i < cPacket.tokenlen; i++) {
            response1[currentByte] = cPacket.token[i];
            currentByte++;
          }
        }
        response1[currentByte] = 194;
        currentByte++;
        response1[currentByte] = 0b00000000;
        currentByte++;
        response1[currentByte] = 0b00000000;
        currentByte++;


        //        response1[currentByte] = 0;
        //        currentByte++;
        //char *payload1;
        char payload1[20] = {(char)32} ;
        Serial.print("adsad");
        Serial.println(cPacket.cOption[1].delta);
        if (cPacket.cOption[1].delta == 6) {
          Serial.println("NWW NWD");
          int zasobLen = 0;
            while (this->zasob[zasobLen] != NULL) {
              zasobLen++  ;
            }
          int nwd = findGCD(this->zasob,zasobLen);
          Serial.print("nwd");
          Serial.println(nwd);
          int nww = findlcm(this->zasob,zasobLen);
          //int nww = 
          payload1[0]='n';
          payload1[1]='w';
          payload1[2]='d';
          payload1[3]=' ';
          payload1[4]=nwd+48;
          payload1[5]='n';
          payload1[6]='w';
          payload1[7]='w';
          payload1[8]=' ';
          char *nwwchar = toArray(nww);
          Serial.print("NWW");
          Serial.println(nww);
          for(int i=0;i<sizeof(nwwchar);i++){
            payload1[9+i]=nwwchar[sizeof(nwwchar)-i-1]+48;
            Serial.println(nwwchar[i]);  
          } 
        } else if (cPacket.token == 0) {
          Serial.println("POKAZANIE ZBIORU");
          if (this->zasob[0] == NULL) {
            char text[] = {"Brak liczb w zasobie"};
            for (int i = 0; i < sizeof(text); i++) {
              payload1[i] = text[i];
            }
          } else {
            int zasobLen = 0;
            while (this->zasob[zasobLen] != NULL) {
              zasobLen++  ;
            }
            Serial.print("zasob len");
            Serial.println(zasobLen);
            for (int i = 0; i < zasobLen; i++) {
              payload1[i] = this->zasob[i] + '0';
            }

          }
        }
          Serial.println("test");
          for (int i = 0; i < sizeof(payload1); i++) {
            Serial.print(payload1[i]);
            Serial.print(" ");
          }
          Serial.println();
          response1[currentByte] = 255;//payload marker
          currentByte++;
          for (int i = 0; i < sizeof(payload1); i++) {
            response1[currentByte] = payload1[i];
            currentByte++;
          }
          Serial.println();
          for (int i = 0; i < sizeof(response1); i++) {
            Serial.print(response1[i]);
            Serial.print(" ");
          }
          Serial.println();
        
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response1, sizeof(response1));
        Udp.endPacket();
      }

    }



    if (cPacket.code == 3) {
      Serial.println("PUT");
      if (isEqual("zbior", cPacket.cOption[0].optionValue, 5)) {
        int payloadSize = cPacket.cOption[1].optionValue;
        for (int i = 0; i < sizeof(cPacket.payload); i++) {
          Serial.print("|");
          Serial.print(cPacket.payload[i]);
          Serial.print(" ");
          Serial.print(i);
        }
        Serial.println();
        uint8_t response[100];
        response[0] = cPacket.coapVersion << 6 | 4 << 4 | cPacket.tokenlen;
        int zasobLen = 0;
        bool isSpaceFlag = false;
        while (this->zasob[zasobLen] != NULL) {
          zasobLen++  ;
        }
        if (zasobLen < 4) {
          response[1] = 68;
          isSpaceFlag = true;
        } else {
          response[1] = 140;
        }
        response[2] = cPacket.messageId >> 8;
        response[3] = cPacket.messageId;
        int currentByte = 4;
        if (cPacket.tokenlen > 0) {
          for (int i = 0; i < cPacket.tokenlen; i++) {
            response[currentByte] = cPacket.token[i];
            currentByte++;
          }
        }
        //adding to array
        if (isSpaceFlag) {
          int newNumber;
          newNumber = cPacket.payload[0];
          Serial.print("new numer");
          newNumber -= 48;
          Serial.println((int)newNumber);
          Serial.println();
          this->addNumber(newNumber, zasobLen);
        }
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, sizeof(response));
        Udp.endPacket();
      }
    }

  }


}
