#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

#define CIABATTA 4
#define CENTRZ 0

RF24 radio(9,10);
RF24Network network(radio);
unsigned long packets_sent;
const uint8_t pin_rele[] = {6,8,5,7,3,4};
struct payload_t{
  uint8_t id;
  unsigned long ms;
  unsigned long counter;
};

void setup(void){
  for (int i=0; i<sizeof(pin_rele); i++){
    pinMode(pin_rele[i],OUTPUT);
    digitalWrite(pin_rele[i],HIGH);
  } 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ CIABATTA);
  /*Serial.begin(9600);
  Serial.println("RF24Network/node_rx_04/Ciabatta");*/
}

void loop(void){
  network.update();
  while ( network.available() ){
    String ms;
    bool ok=false;
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    /*Serial.print("RX #");
    Serial.print(payload.counter);
    Serial.print(" da ");
    Serial.print(payload.id);
    Serial.print(" valore ");
    Serial.println(payload.ms);*/
    String num = String(payload.ms).substring(0,1);
    String val = String(payload.ms).substring(1,2);
    if ((val == "1" || val == "0") && (num == "1" || num == "2" || num == "3" || num == "4" || num == "5" || num == "6")){
      if (val == "1")
        digitalWrite(pin_rele[num.toInt()-1],not(digitalRead(pin_rele[num.toInt()-1])));
      if (digitalRead(pin_rele[num.toInt()-1])==HIGH)
        ms=num+"0"; //Serial.println("ON");
      else
        ms=num+"1"; //Serial.println("OFF");
    }
    else
      ms="9";
    /*Serial.print("TX: #");
    Serial.print(packets_sent);
    Serial.print(" a ");
    Serial.print(CENTRZ);
    Serial.print("valore: ");
    Serial.println(ms);*/
    payload_t payloadI = {CIABATTA, ms.toInt(), packets_sent++};
    RF24NetworkHeader headerI(/*to node*/ CENTRZ);
    ok=network.write(headerI,&payloadI,sizeof(payloadI));
    /*if (ok) 
     Serial.println("ok.");
    else
     Serial.println("failed.");*/
  } 
}
