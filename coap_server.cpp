#include "coap_server.h"
#include <math.h>
#define PACKET_BUFFER_LENGTH        100
#define UDP_SERVER_PORT 1234

byte MAC[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a
unsigned int localPort = UDP_SERVER_PORT;

bool coapServer::addNumber(int newNumber, int currentSize) {//metoda przypisujaca otrzymany numer do zasobow serwera, przyjmuje nowa liczbe oraz miejsce zasobu
  this->zasob[currentSize] = newNumber; //przypisanie do zasobow
}

bool isEqual(char* text1, char* text2, int textSize) {//funkcja porownmujaca dwie tablice charow  
  for (int i = 0; i < textSize; i++) {
    if (text1[i] != text2[i]) {
      return false;
    }
    return true;
  }

}
int gcd(int a, int b) //funkcja wyliczajca warotosc nwd dla 2 liczb
{ 
    if (a == 0) 
        return b; 
    return gcd(b % a, a); 
}

int findlcm(int arr[], int n) // funkcja wyliczajaca nww dla tablicy intow
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

int findGCD(int arr[], int n) // funkcja wyliczajaca nwd dla tablicy intow
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
char * toArray(unsigned int number) { //funkcja pomocnicza zmianiajaca zmienna int na tablice znakow
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
bool coapServer::start() { // funkcja inicjalizujaca polaczenie udp

  ObirEthernet.begin(MAC);
  Serial.print(F("My IP address: "));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(ObirEthernet.localIP()[thisByte], DEC); Serial.print(F("."));
  }
  Serial.println();
  Udp.begin(localPort);
}
bool coapServer::loop() { // glowna petla serwera coap
  int packetSize = Udp.parsePacket(); //sprawdzenie czy otrzymano pakiet udp
  if (packetSize > 0) { // jezeli odebrano pakiet udp
    int len = Udp.read(packetBuffer, PACKET_BUFFER_LENGTH); // odczytywanie pakietu
    Serial.print("Recieved: "); //wypisywanie otrzymanego pakietu
    packetBuffer[len] = '\0';
    packetMessage[len] = '\0';
    Serial.println((char*)packetBuffer);
    coapPacket cPacket; //tworzenie obiektu klasy coapPacket
    cPacket.coapVersion = packetBuffer[0] & 196 >> 6; //odczytywanie wersji coapa z wiadomosci
    cPacket.type = packetBuffer[0] & 48 >> 4;//odczytywanie typu otrzymanej wiadomosci 
    cPacket.tokenlen = packetBuffer[0] & 15;//odczytywanie TKL wiadomosci
    cPacket.code = packetBuffer[1]; //odczytanie kodu otrzymanej wiadomosci
    cPacket.messageId = (packetBuffer[2] << 8) | packetBuffer[3];//łaczenie dwóch bajtów
    //obsluga tokena
    if (cPacket.tokenlen > 0) {
      uint8_t token = new uint8_t[cPacket.tokenlen];
    } else {
      uint8_t token = NULL;
    }
    for (int i = 0; i < cPacket.tokenlen; i++) {
      cPacket.token[i] = packetBuffer[4 + i];
    }
    //obsługa opcji w zalozeniu ograniczamy liczbe opcji oraz tworzymy tablice je przechowujaca
    int optionNumber = 0;// liczba porzadkowa opcji pomocna w operacjach na tablicach
    bool isNextOption = true; // flaga sprawdzajaca istnienie kolejnej opcji do odczytania
    int currentByte = 4 + cPacket.tokenlen; // przypisanie aktualnie zapisywanego bitu, zabieg konieczny przy obslugiwaniu kilku opcji
    while (isNextOption) {
      cPacket.cOption[optionNumber].delta = (packetBuffer[currentByte] & 244 ) >> 4;//przypisanie delty/length w option
      cPacket.cOption[optionNumber].optionLength = packetBuffer[currentByte] & 15; 
      //obsluga rozszerzonych opcji i rozszerzonej delty ( extended )
      if (cPacket.cOption[optionNumber].delta == 13) {
        currentByte++;
        cPacket.cOption[optionNumber].delta += packetBuffer[currentByte];
      }
      if (cPacket.cOption[optionNumber].optionLength == 13) {
        currentByte++;
        cPacket.cOption[optionNumber].optionLength += packetBuffer[currentByte];
      }
      currentByte++;
      //zapisanie wartosci opcji
      if (cPacket.cOption[optionNumber].optionLength > 0) {
        cPacket.cOption[optionNumber].optionValue = new uint8_t[cPacket.cOption[optionNumber].optionLength];
        for (int i = 0 ; i < cPacket.cOption[optionNumber].optionLength; i++) {
          cPacket.cOption[optionNumber].optionValue[i] = packetBuffer[currentByte];
          currentByte++;
        }
      }
      //sprawdzenie czy nie ma wiecej opcji
      if (packetBuffer[currentByte] == NULL) {
        isNextOption = false;
      }
      if (packetBuffer[currentByte] == 255) {
        isNextOption = false;
      }
      optionNumber++;
    }

    //obsluga payload
    uint8_t payloadMarker = 255; // przypisanie payloadmarkera 
    if (packetBuffer[currentByte] == payloadMarker) {
      int nextByte = currentByte;
      while (packetBuffer[nextByte] != NULL) {
        nextByte++; // wyliczenie dlugosci payloadu na podstawie dlugosci pakietu udp i aktualnie odczytywanego bitu
      }
      cPacket.payload = new uint8_t[nextByte - currentByte - 1]; // zadeklarowanie dlugosci payloadu na podtawie utworzonej wczesniej dlugosci
      for (int i = 0; i < (nextByte - currentByte - 1); i++) {
        cPacket.payload[i] = packetBuffer[i + currentByte + 1];//zapisanie payloadu

      }



    }

    //obsluga otrzymanych pakietow i odpowiedz
    if (cPacket.code == 1) {
      if (isEqual(".well-known", cPacket.cOption[0].optionValue, 11)) {
        if (isEqual("core", cPacket.cOption[1].optionValue, 4)) {//przypadek w ktorym otrzymujemy uri-path - ./well-known/core
          cPacket.response.coapVersion = cPacket.coapVersion;
          uint8_t response[100] = {32};
          response[0] = cPacket.coapVersion << 6 | 4 << 4 | cPacket.tokenlen; //przypisanie wersji z oczytanego pakietu
          response[1] = 69; //response code 2.05
          response[2] = cPacket.messageId >> 8;
          response[3] = cPacket.messageId;
          int currentByte = 4;
          response[currentByte] = 194;//przypisanie wartosci delta i lenght wysylanej opcji
          currentByte++;
          response[currentByte] = 0;//przypisanie wartosci opcji
          currentByte++;
          response[currentByte] = 40;//przypisanie wartosci opcji
          currentByte++;
          response[currentByte] = 255; // payload marker
          char payload[] = "</zbior>;if=zbior;</metryka1>;if=metryka1;</metryka2>;if=metryka2;</metryka3>;if=metryka3;";//zasob well-known/core

          currentByte++;
          for (int i = 0; i < sizeof(payload); i++) {
            response[currentByte] = payload[i];//przypisanie payloadu do wysylanej odpowiedzi
            currentByte++;
          }
          
          Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()); //odczytanie ip i portu do ktorego wysylamy odpowiedz
          Udp.write(response, sizeof(response)); //wysylanie odpowiedzi udp
          Udp.endPacket(); //zakonczenie sesji udp
        }
      }

      if (isEqual("zbior", cPacket.cOption[0].optionValue, 5)) { //uripath = zbior
        //get zbior
        Serial.println("zbior");
        byte response1[100];
        response1[0] = cPacket.coapVersion << 6 | 4 << 4 | cPacket.tokenlen; //przypisywanie wartosci zgodnie z poprzednim schematem
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

        char payload1[20];
        Serial.println(cPacket.cOption[1].delta); //sprawdzenie nww i nwd przed wyslaniem(do naszego wgladu w konsoli)
        if (cPacket.cOption[1].delta == 6) {
          Serial.println("NWW NWD");
          int zasobLen = 0;
            while (this->zasob[zasobLen] != NULL) {//jesli element zasobu jest NULL - > zwieksz
              zasobLen++  ;
            }
          int nwd = findGCD(this->zasob,zasobLen); //obliczanie nwd na podstawie elementow tablicy zasob
          Serial.print("nwd");
          Serial.println(nwd);
          int nww = findlcm(this->zasob,zasobLen); //obliczanie nww j.w.
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
          char *nwwchar = toArray(nww); //funkcja zamieniajaca int na char
          Serial.print("NWW");
          Serial.println(nww);
          for(int i=0;i<sizeof(nwwchar);i++){
            payload1[9+i]=nwwchar[sizeof(nwwchar)-i-1]+48; //przypisanie odpowiedzi do payloadu
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



    if (cPacket.code == 3) { //obsluga  zadania PUT
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
        //dodawanie do tablicy
        if (isSpaceFlag) {
          int newNumber;
          newNumber = cPacket.payload[0];
          Serial.print("new numer");
          newNumber -= 48;
          Serial.println((int)newNumber);
          Serial.println();
          this->addNumber(newNumber, zasobLen);
        }
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()); //przetworzenie i wysylka pakietu udp
        Udp.write(response, sizeof(response));
        Udp.endPacket();
      }
    }

  }


}
