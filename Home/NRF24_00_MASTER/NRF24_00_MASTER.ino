/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */
 
#include <Wire.h>
#include <String.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


RF24 radio(9,10);                // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);      // Network uses that radio
const uint16_t this_node = 00;    // Address of our node in Octal format ( 04,031, etc)
const uint16_t other_node = 01;   // Address of the other node in Octal format

struct payload_t {                 // Structure of our payload
  unsigned long ms;
  unsigned long counter;
};

const unsigned long interval = 500;   //ms  // How often to send 'hello world to the other unit
unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already

const unsigned long interval_read = 1000;   //ms  // How often to send 'hello world to the other unit
unsigned long last_read;             // When did we last send?

bool DEBUG=true;  //true=ENABLE false=DISABLE
char NRF24_BUFFER[5];

String BUFFER_OUT[10];
int INDEX_OUT = 0;
String MEX_OUT="";

String BUFFER_IN[10];
int INDEX_IN = 0;

void init_IN()
{
  for(int i = 0; i < 10; i++)
  {
    String s="";
    BUFFER_IN[i] = s;
  }
}

void init_OUT()
{
  for(int i = 0; i < 10; i++)
  {
    String s="";
    BUFFER_OUT[i] = s;
  }
}
 
void add_IN(String mex)
{
  if (INDEX_IN==9)
    INDEX_IN=0;
  BUFFER_IN[INDEX_IN]=mex;
  if (DEBUG)
  {
    Serial.print("ADD IN: ");  
    Serial.print(BUFFER_IN[INDEX_IN]);
    Serial.print(" POS: ");
    Serial.println(INDEX_IN);
  }
  INDEX_IN++;
  delay(50);
}

void add_OUT(String mex)
{
  if (INDEX_OUT==9)
    INDEX_OUT=0;
  BUFFER_OUT[INDEX_OUT]=mex;
  if (DEBUG)
  {
    Serial.print("ADD OUT: ");  
    Serial.print(BUFFER_OUT[INDEX_OUT]);
    Serial.print(" POS: ");
    Serial.println(INDEX_OUT);
  }
  INDEX_OUT++;
  delay(50);
}

void returnCurrent_IN()
{
  String res="";
  res=BUFFER_IN[0];
  res.toCharArray(NRF24_BUFFER, 5);
  for (int i=1;i<10;i++)
    BUFFER_IN[i-1]=BUFFER_IN[i];
  BUFFER_IN[9]="";  
  if (INDEX_IN>0)
    INDEX_IN--;
}

String returnCurrent_OUT()
{
  String res="";
  res=BUFFER_OUT[0];
  for (int i=1;i<10;i++)
    BUFFER_OUT[i-1]=BUFFER_OUT[i];
  if (INDEX_OUT>0)
    INDEX_OUT--;
  return res;  
}

void setup(void)
{
  Serial.begin(57600);
  Serial.print("RF24Network/MASTER: ");

  init_IN();
  init_OUT();
  
  Wire.begin(8);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  add_IN("00");

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  if (DEBUG) Serial.println("START");
}

void loop(void)
{
  SERIAL_READ();
  unsigned long now = millis();              // If it's time to send a message, send it!
  if ( now - last_read >= interval_read  )
  {
    last_read = now;    
    network.update();                  // Check the network regularly
    while ( network.available() )
    { // Is there anything ready for us?
      RF24NetworkHeader header;        // If so, grab it and print it out
      payload_t payload;
      network.read(header,&payload,sizeof(payload));
      printRX(header.from_node,payload.ms);
      String s=(String)header.from_node;
      s+=(String)payload.ms;
      add_IN(s);
    }
  }
  else  
    sendNRFCommand();
  delay(50);  
}



void SERIAL_READ()
{
  String node="";
  String ms="";
  String SERIAL_BUFFER="";
  while(Serial.available() > 0) 
  SERIAL_BUFFER=Serial.readString();
  if (SERIAL_BUFFER.length() >0 && Serial.available() < 1)
  {
    node=SERIAL_BUFFER.substring(0,1);
    ms=SERIAL_BUFFER.substring(1,2);
    sendTX(node.toInt(),ms.toInt());
    SERIAL_BUFFER="";
  }   
  
  
}

void sendNRFCommand()
{
  //Serial.print("Current len TX: ");
  //Serial.println(BUFFER_OUT[0].length());
  
  if (BUFFER_OUT[0].length()!=0)
  {
    String p=returnCurrent_OUT();
    if (p.length()>=2)
    {
      String node=p.substring(0,1);
      String ms=p.substring(1,2);
      Serial.print("Current: ");
      Serial.println(p);
      Serial.print("node.toInt: ");
      Serial.println(node.toInt());
      Serial.print("ms.toInt(): ");
      Serial.println(ms.toInt());
      //int id=((int)p.charAt(0))-48;
      //unsigned long ms=p.substring(1,4).toInt();
      //bool send_result=sendTX(id,ms);
      bool send_result=sendTX(node.toInt(),ms.toInt());
      if (send_result==false)
      {
        Serial.println("NO SEND");
        network.update();
        add_IN("0");
      }

      //sendTX(node.toInt(),ms.toInt());
    }    
    /*
    if (send_result==false)
    {
      Serial.println("NO SEND");
      //add_IN("0");
    }
    */  
  }  
}

void printRX(uint16_t node,unsigned long ms)
{
    Serial.print("<< RX ");
    Serial.print("HEADER: ");
    Serial.print(node);
    Serial.print(" PAYLOAD: ");
    Serial.println(ms);
    Serial.println("##################");  
}

bool sendTX(uint16_t node,unsigned long ms)
{
  bool ok=false;
  int count=0;
  //while (ok==false  )
  while (count !=7 )
  { 
    network.update();                          // Check the network regularly 
    unsigned long now = millis();              // If it's time to send a message, send it!
    if ( now - last_sent >= interval  )
    {
      last_sent = now;
      Serial.print("Sending to ");
      Serial.print(node);
      Serial.print(" mex ");
      Serial.println(ms);
      payload_t payload = { ms, packets_sent++ };
      RF24NetworkHeader header(/*to node*/ node,'t');
      ok = network.write(header,&payload,sizeof(payload));
      if (ok)
      {
        Serial.println("ok.");
        count=7;
      }  
      else
      {
        Serial.println("failed.");
        count++;
      }  
    }
  }
 Serial.println("##################");
 return ok;
}

/* I2C */
void receiveEvent(int b)
{
  String I2C_OUT_BUFFER="";
  while(Wire.available()){
     char c = Wire.read();
     I2C_OUT_BUFFER+=c;
  }
  if (I2C_OUT_BUFFER!="")
  {
    Serial.print("<< I2C: ");
    Serial.println(I2C_OUT_BUFFER);
    Serial.println("##################");
    Serial.println("<< I2C: "+I2C_OUT_BUFFER);
    add_OUT(I2C_OUT_BUFFER);    
  }  
}
void requestEvent()
{
  returnCurrent_IN();
  Wire.write(NRF24_BUFFER); 
  //Serial.println(">> I2C: ");
  /*
  Serial.println(NRF24_BUFFER);
  Serial.println("##################");
  */
  //delay(50);
}
/*
void requestEvent_2()
{
  String s="";
  Serial.println("request I2C");
  char NRF24_BUFFER[5];
  if (BUFFER_IN[0].length()!=0)
  {
   s=returnCurrent_IN(); 
   s.toCharArray(NRF24_BUFFER, 5);
  }
  else
  {
    s="0";
    s.toCharArray(NRF24_BUFFER, 5);
  }  
  //Wire.write(NRF24_BUFFER);
  Wire.write("hello ");
  Serial.print(">> I2C: ");
  Serial.println(NRF24_BUFFER);
  Serial.println("##################");
  delay(50);
}
*/
