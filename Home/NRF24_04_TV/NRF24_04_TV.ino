#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h> 
#include <IRremote.h>
#include <dht11.h>

#define DHT11_PIN 8
#define TV 3
#define CENTRZ 0

dht11 DHT;
IRsend irsend;
RF24 radio(9,10);
RF24Network network(radio);
unsigned long packets_sent;
struct payload_t{
  uint8_t id; 
  unsigned long ms;
  unsigned long counter;
};

void setup(void){
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ TV);
  /*Serial.begin(9600);
  Serial.println("RF24Network/node_rx_03/TV");*/
}

void loop(void){  
  network.update();
  while ( network.available() ){
    unsigned long ms;
    bool ok=false;
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    /*Serial.print("RX #");
    Serial.print(payload.counter);
    Serial.print(" da ");
    Serial.print(payload.id);
    Serial.print(" val: ");
    Serial.println(payload.ms);*/ 
    if ( payload.ms==1){
      int chk = DHT.read(DHT11_PIN);
      ms=100+DHT.temperature;
    }
    if ( payload.ms==2){
      int chk = DHT.read(DHT11_PIN);
      ms=200+DHT.humidity;
    }
    if ( payload.ms!=2 && payload.ms!=1){
      uint16_t cmdTV=writeTV(payload.ms);
      if (cmdTV != 0x0){
        for (int i = 0; i < 5; i++){
          irsend.sendRC5(cmdTV, 12); // RC5 TV power code
          delay(40);
        }
        ms=0;
      }
      else
        ms=9;
    }
    /*Serial.print("TX: #");
    Serial.print(packets_sent);
    Serial.print(" a ");
    Serial.print(CENTRZ);
    Serial.print("valore: ");
    Serial.println(TEMP);*/
    payload_t payloadI= { TV, ms, packets_sent++};
    RF24NetworkHeader headerI(/*to node*/ CENTRZ);
    ok=network.write(headerI,&payloadI,sizeof(payloadI));
    /*if (ok)
     Serial.println("ok."); 
    else
     Serial.println("failed.");*/
  }  
}

uint16_t writeTV(unsigned long cmd){
    uint16_t cmdTV=0x0; 
    if (cmd == 0)
      cmdTV = 0xC;
    if (cmd == 4) //VOL +
      cmdTV = 0x10;      
    if (cmd == 5) //VOL -
      cmdTV = 0x11;      
    if (cmd == 6) //CH+
      cmdTV = 0x20;      
    if (cmd == 7) //CH-
      cmdTV = 0x21;
    return cmdTV;
}
