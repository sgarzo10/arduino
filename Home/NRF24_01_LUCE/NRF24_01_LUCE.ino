/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network 
 *
 * TRANSMITTER NODE
 * Every 2 seconds, send a payload to the receiver node.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

#define pin_rele 5

RF24 radio(9,10);                    // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);          // Network uses that radio

const uint16_t this_node = 01;        // Address of our node in Octal format
const uint16_t other_node = 00;       // Address of the other node in Octal format

const unsigned long interval = 810; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already
unsigned long ms;                    // MS


struct payload_t {                  // Structure of our payload
  unsigned long ms;
  unsigned long counter;
};

void setup(void)
{
  pinMode(pin_rele,OUTPUT);
  
  Serial.begin(57600);
  Serial.print("RF24Network/NODE_01-LUCE: ");
 
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  Serial.println("START");
  Serial.println("##################"); 
}

void loop(void)
{ 
  unsigned long res=9999; 
  network.update();                  // Check the network regularly
  while ( network.available() )
  {     // Is there anything ready for us?  
    RF24NetworkHeader header;        // If so, grab it and print it out
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    printRX(header.from_node,payload.ms);
    res=doCommand(payload.ms);
    sendTX(other_node,res);
  }
  delay(50);
  
}

void printRX(uint16_t node,unsigned long ms)
{
    Serial.print("<< RX ");
    Serial.print("NODO: ");
    Serial.print(node);
    Serial.print(" MEX: ");
    Serial.println(ms);
    Serial.println("##################");  
    delay(50);
    
}
unsigned long doCommand(unsigned long ms)
{
  unsigned long res=9;
  if (ms == 1 || ms == 0)
  {
    if (ms == 1)
      digitalWrite(pin_rele,not(digitalRead(pin_rele)));
    if (digitalRead(pin_rele)==HIGH)
      res=0; //Serial.println("ON");
    else
      res=1; //Serial.println("OFF");
  }
  return res;
}

bool sendTX(uint16_t node,unsigned long ms)
{
  bool ok=false;
  int count=0;
  while (count !=5 )
  {
    network.update();                          // Check the network regularly

    //unsigned long now = millis();              // If it's time to send a message, send it!
    //if ( now - last_sent >= interval  )
    //{
      //last_sent = now;
      Serial.print("Sending...");
      payload_t payload = { ms, packets_sent++ };
      RF24NetworkHeader header(/*to node*/ node,'t');
      ok = network.write(header,&payload,sizeof(payload));
      if (ok)
      {
        Serial.println("ok.");
        count=5;
      }  
      else
      {
        Serial.println("failed.");
        count++;
      }  
    //}
  }
  return ok;
}
