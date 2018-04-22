#include <dht11.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DS3232RTC.h>
#include <EEPROM.h>

uint8_t ArrGG[7]={0,0,0,0,0,0,0};
uint8_t ArrHH[7]={0,0,0,0,0,0,0};
uint8_t ArrMM[7]={0,0,0,0,0,0,0};
uint8_t g=0;
uint8_t LED_TEST=3;
uint8_t BUZZ=13;
uint8_t BTN_LCD=6;
uint8_t pagina=0;
uint8_t hum=0;
//BLUETOOTH: PIN TXD 4, PIN RXD 2
String BLUETOOTH_BUFFER;
String MESSAGGIO="";
String strData;
String strOra;
boolean premuto=false;
boolean acceso=false;
boolean suonato=false;
SoftwareSerial bluetooth(4, 2);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
dht11 DHT;
#define DHT11_PIN 5

void setup() 
{
  lcd.begin(16,2);
  pinMode(LED_TEST,OUTPUT);
  pinMode(BUZZ,OUTPUT);
  pinMode(BTN_LCD,INPUT);
  bluetooth.begin(9600); 
  Serial.begin(9600);
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    Serial.println("NOT SYNC");
  else
    Serial.println("SYNC");
  delay(100);
  DHT_CHECK();
  LCD_PAGE(); 
  START();
}

void loop() 
{ 
  SHOW_DATE();
  LCD_PAGE();
  BTN_READ(true);
  BLUETOOTH_READ();
  SVEGLIA_CHECK();
  delay(100);
}

void START()
{
  //BLINK LED TEST
  uint8_t i,j=0;
  for (i=0;i<6;i++) 
  { 
    digitalWrite(LED_TEST,!digitalRead(LED_TEST));
    delay (200);
  }
  //LETTURA MESSAGGIO
  boolean t=false;
  for(i=0;t==false;i++)
  {
    if(EEPROM.read(i)!=0)
      MESSAGGIO+=char(EEPROM.read(i));
    else
      t=true;
  }
  //LETTURA SVEGLIE
  for(i=17;i<24;i++)
  {
    ArrGG[j]=EEPROM.read(i);
    j++;
  }
  j=0;
  for(i=24;i<31;i++)
  {
    ArrHH[j]=EEPROM.read(i);
    j++;
  }
  j=0;
  for(i=31;i<38;i++)
  {
    ArrMM[j]=EEPROM.read(i);
    j++;
  }
}

void BLUETOOTH_READ()
{
  while (bluetooth.available())
  {
    Serial.println("Arrivano dati");
    BLUETOOTH_BUFFER+=(char)bluetooth.read();
  }
  if(!bluetooth.available() && BLUETOOTH_BUFFER!="")
  {
    Serial.print("Messagio ricevuto: ");
    Serial.println(BLUETOOTH_BUFFER);
    bluetooth.println("Messaggio ricevuto!"+BLUETOOTH_BUFFER);
    BLUETOOTH_COMMAND();
    BLUETOOTH_BUFFER="";
  }
}

void BLUETOOTH_COMMAND()
{
  if (BLUETOOTH_BUFFER=="led1 ON")
    digitalWrite(LED_TEST,HIGH);
  if (BLUETOOTH_BUFFER=="led1 OFF")
    digitalWrite(LED_TEST,LOW);
  if (BLUETOOTH_BUFFER.substring(0,4)=="set ")
    SET_MEX();
  if (BLUETOOTH_BUFFER.substring(0,5)=="time ")
    SET_DATE(BLUETOOTH_BUFFER.substring(5,BLUETOOTH_BUFFER.length()));
  if (BLUETOOTH_BUFFER.substring(0,8)=="sveglia ")
    SET_SVEGLIA(BLUETOOTH_BUFFER.substring(8,BLUETOOTH_BUFFER.length()));
  if (BLUETOOTH_BUFFER=="reset")
    RESET_EEPROM(); 
}

void RESET_EEPROM()
{
  for(int i=0;i<EEPROM.length();i++)
    EEPROM.write(i,0);
}

void SET_MEX()
{
  Serial.println("SET MEX");
  MESSAGGIO=BLUETOOTH_BUFFER.substring(4,BLUETOOTH_BUFFER.length());
  uint8_t i;
  for(i=0;i<MESSAGGIO.length();i++)
  {
    char c=MESSAGGIO.charAt(i);
    int m=c;
    EEPROM.write(i,m);
  }
  EEPROM.write(i,0);
  LCD_PAGE();  
}

void SET_DATE(String s)
{
  Serial.println("SET TIME");
  tmElements_t tm;
  time_t t;
  tm.Year=y2kYearToTm(s.substring(0,2).toInt());
  tm.Month=s.substring(2,4).toInt();
  tm.Day=s.substring(4,6).toInt();
  tm.Hour=s.substring(6,8).toInt();
  tm.Minute=s.substring(8,10).toInt();
  tm.Second=s.substring(10,12).toInt();
  t=makeTime(tm);
  RTC.set(t);
  setTime(t);
}

void SET_SVEGLIA(String s)
{
  uint8_t d=s.substring(0,1).toInt();
  uint8_t h=s.substring(1,3).toInt();
  uint8_t m=s.substring(3,5).toInt();
  ArrGG[d]=1;
  ArrHH[d]=h;
  ArrMM[d]=m;
  uint8_t i,j=0;
  for(i=17;i<24;i++)
  {
    EEPROM.write(i,ArrGG[j]);
    j++;
  }
  j=0;
  for(i=24;i<31;i++)
  {
    EEPROM.write(i,ArrHH[j]);
    j++;
  }
  j=0;
  for(i=31;i<38;i++)
  {
    EEPROM.write(i,ArrMM[j]);
    j++;
  }
}

void SVEGLIA_CHECK()
{
  g=weekday()-1;
  if (ArrGG[g]==1 && ArrHH[g]==hour() && ArrMM[g]==minute())
  {
    for (int i=0;i<1000 && suonato==false;i++)
    {
      digitalWrite(BUZZ,HIGH);
      delay(1);
      digitalWrite(BUZZ,LOW);
      delay(1);
      if(BTN_READ(false))
        suonato=true;
    }
  }
  else
     suonato=false;
  Serial.println(suonato);
}
 
void DHT_CHECK()
{
  int chk;
  chk=DHT.read(DHT11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      Serial.println("OK\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println("Timeout Error\t");
      break;
    default:
      Serial.println("Unkown Error\t");
      break;    
  }
  if(DHT.humidity<100)
    hum=DHT.humidity;
}

void DHT_PRINT()
{ 
  lcd.print("TEMP: ");
  lcd.print(RTC.temperature()/4.0);
  lcd.setCursor(0, 1);
  lcd.print("HUM: ");
  lcd.print(hum);
  lcd.print(" %");
}

void MEX_PRINT()
{
  uint8_t pos=0;
  lcd.setCursor(6,0);
  lcd.print("MEX:");
  pos=(16-MESSAGGIO.length())/2;
  lcd.setCursor(pos,1);
  lcd.print(MESSAGGIO);
}

void DATA_PRINT()
{
  lcd.print("ORA:"+strOra);
  lcd.setCursor(0,1);
  lcd.print("DATA:"+strData);
}

void SVEGLIA_PRINT()
{
  lcd.print("DLMMGVS");
  lcd.setCursor(0,1);
  for(uint8_t i=0;i<7;i++)
  {
    if (ArrGG[i]==1)
      lcd.print("X");
    else
      lcd.print("O");
  }
  lcd.setCursor(9,0);
  lcd.print("ORA:");
  lcd.setCursor(8,1);
  lcd.print(ArrHH[g]);
  lcd.print(returnDigit(ArrMM[g]));
  lcd.setCursor(g,1);
  lcd.cursor();
}

boolean BTN_READ(boolean cambioPag)
{
  uint32_t i=0;
  boolean res=false;
  while (digitalRead(BTN_LCD)==0)
  {
    i++;
    if(i>150000 && acceso==false)
    {
      digitalWrite(LED_TEST,!digitalRead(LED_TEST));
      acceso=true;
      cambioPag=false;
    }
    premuto=true;
  }
  if(premuto)
  {
    if(cambioPag==true)
    {
      if (pagina+1>3)
        pagina=0;
      else
        pagina=pagina+1;
      LCD_PAGE();
    }
    res=true;
    acceso=false;
    premuto=false;
  }
  return res;
}

void SHOW_DATE()
{
  strData="";
  strOra="";
  strData=strData+day()+" ";
  strData=strData+month()+" ";
  strData=strData+year();
  strOra=hour();
  strOra=strOra+returnDigit(minute());
  strOra=strOra+returnDigit(second());
  String s = returnDigit(second());
  if (s == ":30" || s == ":00" )
    DHT_CHECK();
}

String returnDigit(int digits)
{
  String s;
  s=":";
  if (digits<10)
    s=s+"0";
  s=s+digits;  
  return s;
}

void LCD_PAGE()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.noCursor();
  switch (pagina)
  {
    case 0:
      MEX_PRINT();
      break;
    case 1:
      DATA_PRINT();
      break;
    case 2:
      DHT_PRINT();
      break;
    case 3:
      SVEGLIA_PRINT();
      break;
  }
}

