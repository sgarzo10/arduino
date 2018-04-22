#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "Home.h"
#include "indexl.h"

HOME H;
//const char* ssid = "Infostrada-EB3118";
const char* ssid = "RoomWifi";
//const char* password = "G7LLFX7R7G";
const char* password = "roomwifi3553";
const char* fingerprint = "‎19 6D CC D2 2E B2 0E 9E B5 48 5e 93 A5 F5 14 DA AD A0 52 7C";
ESP8266WebServer server(80);
struct pacchetto{
  String header;
  String cmdClient;
  String valClient;
  String cmdInviato;
  String checkWrite;
  String segnaleRicevuto;
  String messaggio;};

void setup(void){
  /*IPAddress ip(192, 168, 1, 5);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);*/
  WiFi.begin(ssid, password);
  //PIN RESET
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
  pinMode(16,OUTPUT);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
  MDNS.begin("esp8266");
  server.on("/", handleRoot);
  server.on("/style.css", handlerCss);
  server.on("/index.js", handlerJs);
  server.on("/comando", handlerComando);
  server.onNotFound(handleNotFound);
  server.begin();
  Wire.begin();
  Serial.begin(115200);
  Serial.print("\nHTTP server started with IP ");
  Serial.println(WiFi.localIP());
}

void loop(void){
  server.handleClient();
  requestI2C(8,4);
  SERIAL_READ();    
}

void SERIAL_READ(){
  String SERIAL_BUFFER="";
  while(Serial.available() > 0) 
  SERIAL_BUFFER=Serial.readString();
  if (SERIAL_BUFFER.length() >0 && Serial.available() < 1){
    scritturaWireStr(SERIAL_BUFFER);
    SERIAL_BUFFER="";
  }   
}

void scritturaWireStr(String cmd){ 
  char c[4]="--";
  cmd.toCharArray(c,4);
  Wire.beginTransmission(8);
  Wire.write(c);
  Wire.endTransmission();
}

String requestI2C(int dest, int dim){
  delay(1000);
  digitalWrite(16,0);
  String IN_BUFFER="";
  Wire.requestFrom(dest, dim);
  while (Wire.available()){ 
    char c = Wire.read();
    IN_BUFFER+=c;
  }
  if(IN_BUFFER!=""){
    digitalWrite(16,HIGH);
    Serial.print("Buff:");
    Serial.println(IN_BUFFER);
    H.decodificaNRF(IN_BUFFER);
    if (H.flag_PRESENZA==1){
      H.vULTIMA_PRESENZA=getRequest();
      Serial.println(H.vULTIMA_PRESENZA);
      H.flag_PRESENZA=0;
    }  
  }
  return IN_BUFFER;
}

bool controlOnOff(String cmd){
  if(cmd!="on" && cmd!="off")
    return false;
  else
    return true;
}

bool controlPirCommand(String cmd){
  if (controlOnOff(cmd) || cmd=="buzzOn" || cmd=="buzzOff" || cmd=="stato")
    return true;
  else
    return false;
}

bool controlTvCommand(String cmd){
  if (controlOnOff(cmd) || cmd=="temp" || cmd=="hum" || cmd=="vol+" || cmd=="vol-" || cmd=="ch+" || cmd=="ch-")
    return true;
  else
    return false;
}

bool controlPresaCommand(String cmd){
  if (controlOnOff(cmd) || cmd=="stato")
    return true;
  else
    return false;
}

bool controlPresa(String cmd){
  if (cmd == "1" || cmd == "2" || cmd == "3" || cmd == "4" || cmd == "5" || cmd == "6")
    return true;
  else
    return false;
}
void handleRoot() {server.send(200, "text/html", fileIndex);}
void handlerCss(){server.send(200,"text/css", fileCss);}
void handlerJs(){server.send(200,"text/javascript", fileJs);}
void handlerComando(){
  String comando = server.arg(0);
  String valore= server.arg(1);
  String response="";
  pacchetto p;
  if (comando=="" || (comando!="luce" && comando!="pir" && comando!="tv"  && comando!="ciabatta" && comando!="refresh" && comando!="reset")){
    p.header="err";
    p.cmdClient=comando;
    p.valClient=valore;
    p.cmdInviato="";
    p.checkWrite="";
    p.segnaleRicevuto="";
    p.messaggio="Comando non riconosciuto";
  }
  else{
    if (valore==""){
      p.header="err";
      p.cmdClient=comando;
      p.valClient=valore;
      p.cmdInviato="";
      p.checkWrite="";
      p.segnaleRicevuto="";
      p.messaggio="Valore nullo";
    }
    else{
        if(comando=="luce"){
          if (controlPresaCommand(valore)){
            String cmd=H.cmdLUCE(valore);
            scritturaWireStr(cmd);
            String check_write = requestI2C(8,4).substring(0,2);
            if(check_write == "01"){
              String rx_luce = requestI2C(8,4);
              String primi=rx_luce.substring(0,2);
              if(primi == "10" || primi == "11" || primi == "19"){
                p.header="ok";
                p.messaggio=H.decodificaNRF(rx_luce);
              }
              else{
                p.header="err";
                p.messaggio="Non ho ricevuto risposta dalla luce (reset Nano)";
              }
              p.segnaleRicevuto=rx_luce;
            }
            else{
              p.header="err";
              p.segnaleRicevuto="";
              p.messaggio="Errore in scrittura verso la luce";
            }
            p.cmdClient=comando;
            p.valClient=valore;
            p.cmdInviato=cmd;
            p.checkWrite=check_write;
          }
          else{
            p.header="err";
            p.cmdClient=comando;
            p.valClient=valore;
            p.cmdInviato="";
            p.checkWrite="";
            p.segnaleRicevuto="";
            p.messaggio="Comando non valido per la luce";
          }
        }
        if(comando=="pir"){
          if (controlPirCommand(valore)){
            String cmd=H.cmdPIR(valore);
            scritturaWireStr(cmd);
            String check_write = requestI2C(8,4).substring(0,2);
            if(check_write == "01"){
              String rx_pir = requestI2C(8,4);
              String primi=rx_pir.substring(0,3);
              if(primi == "211" || primi == "210" || primi == "201" || primi == "200" || primi.substring(0,2) == "29"){
                p.header="ok";
                p.messaggio=H.decodificaNRF(rx_pir);
              }
              else{
                p.header="err";
                p.messaggio="Non ho ricevuto risposta dal PIR (reset Nano)";
              }
              p.segnaleRicevuto=rx_pir;
            }
            else{
              p.header="err";
              p.segnaleRicevuto="";
              p.messaggio="Errore in scrittura verso il PIR";
            }
            p.cmdClient=comando;
            p.valClient=valore;
            p.cmdInviato=cmd;
            p.checkWrite=check_write;
          }
          else{
            p.header="err";
            p.cmdClient=comando;
            p.valClient=valore;
            p.cmdInviato="";
            p.checkWrite="";
            p.segnaleRicevuto="";
            p.messaggio="Comando non valido per il PIR";
          }
        }
        if(comando=="tv"){
          if (controlTvCommand(valore)){
            String cmd=H.cmdTV(valore);
            scritturaWireStr(cmd);
            String check_write = requestI2C(8,4).substring(0,2);
            if(check_write == "01"){
              String rx_tv = requestI2C(8,4);
              String primi = rx_tv.substring(0,2);
              if(primi == "30" || primi == "31" || primi == "32" || primi == "39"){
                p.header="ok";
                p.messaggio=H.decodificaNRF(rx_tv);
              }
              else{
                p.header="err";
                p.messaggio="Non ho ricevuto risposta dalla TV (reset Nano)";
              }
              p.segnaleRicevuto=rx_tv;
            }
            else{
              p.header="err";
              p.segnaleRicevuto="";
              p.messaggio="Errore in scrittura verso la TV";
            }
            p.cmdClient=comando;
            p.valClient=valore;
            p.cmdInviato=cmd;
            p.checkWrite=check_write;
          }
          else{
            p.header="err";
            p.cmdClient=comando;
            p.valClient=valore;
            p.cmdInviato="";
            p.checkWrite="";
            p.segnaleRicevuto="";
            p.messaggio="Comando non valido per la TV";
          }
        }
        if(comando=="ciabatta"){
          String nPresa= server.arg(2);
          if(controlPresaCommand(valore) && controlPresa(nPresa)){
            String cmd=H.cmdCiabatta(nPresa,valore);
            scritturaWireStr(cmd);
            String check_write = requestI2C(8,4).substring(0,2);
            if(check_write == "01"){
              String rx_c = requestI2C(8,4);
              String f = rx_c.substring(0,3); 
              if(f == "410" || f == "411" || f == "420" || f == "421" || f == "430" || f == "431" || f == "440" || f == "441" || f == "450" || f == "451" || f == "460" || f == "461" || f.substring(0,2) == "49"){
                p.header="ok";
                p.messaggio=H.decodificaNRF(rx_c);
              }
              else{
                p.header="err";
                p.messaggio="Non ho ricevuto risposta dalla ciabatta (reset Nano)";
              }
              p.segnaleRicevuto=rx_c;
            }
            else{
              p.header="err";
              p.segnaleRicevuto="";
              p.messaggio="Errore in scrittura verso la ciabatta";
            }
            p.cmdClient=comando;
            p.valClient=valore+nPresa;
            p.cmdInviato=cmd;
            p.checkWrite=check_write;
          }
          else{
            p.header="err";
            p.cmdClient=comando;
            p.valClient=valore+nPresa;
            p.cmdInviato="";
            p.checkWrite="";
            p.segnaleRicevuto="";
            p.messaggio="Comando non valido per la ciabatta";
          }
        }
        if(comando=="reset"){
          String s=String(digitalRead(2));
          p.header="ok";
          p.cmdClient=comando;
          p.valClient=valore;
          p.cmdInviato="";
          p.checkWrite="";
          p.segnaleRicevuto="";
          p.messaggio="Reset lettura "+s;
          digitalWrite(2,LOW);
          delay(5);
          digitalWrite(2,HIGH);
        }
        if(comando=="refresh")
          Serial.println("Refresh");
      }
    }
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["header"] = p.header;
    root["cmdClient"] = p.cmdClient;
    root["valClient"] = p.valClient;
    root["cmdInviato"] = p.cmdInviato;
    root["checkWrite"] = p.checkWrite;
    root["segnaleRicevuto"] = p.segnaleRicevuto;
    root["messaggio"] = p.messaggio;
    root.prettyPrintTo(response);
    server.send(200,"text/plain",response);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

String getRequest(){
    String res="";
    const char* host = "www.time.is";
    WiFiClientSecure client;
    if (!client.connect(host, 443))
      return ""; //Serial.println("connection failed");
    client.verify(fingerprint, host);
    String url = "/";
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line.substring(0,4) == "Expi")
        res=line.substring(14,34);
      if (line == "\n")
        break;
    }
    client.stop();
    return res;
}
