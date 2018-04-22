#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>
#include <SPI.h>
#include <SD.h>

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define MAX_Y 916
#define MIN_Y 80
#define MAX_X 913
#define MIN_X 124 //*/

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

TouchScreen ts = TouchScreen(8, A3, A2, 9, 300);
Adafruit_TFTLCD tft(A3, A2, A1, A0, A4);
uint8_t ySerial,page=0;
struct button {
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t heigth;
  uint16_t radius;
  String text;
  boolean visible;
  };
button b1,b2,indietro,b_presenze,b_temp;
void setup(void) {
  Serial.begin(250000);
  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());
  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  //SD  
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  if (!SD.begin(4))
    Serial.println("initialization failed!");
  else
    Serial.println("initialization done.");  
  //PAG. MENU
  b1=createButton(50,50,100,50,20,"SERIAL",true);
  b2=createButton(170,50,100,50,10,"FILE",true);
  //PAG.FILE
  b_presenze=createButton(30,50,200,50,10,"PRESENZE",true);
  b_temp=createButton(30,120,200,50,10,"TEMPERATURE",true);
  indietro=createButton(20,190,40,40,20,"BCK",true);
  drawButton(&b1);
  drawButton(&b2);
}

void loop() {
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  if (p.z > 10 && p.z < 1000)
  {
    uint16_t x,y;
    p.x = map(p.x,120,920,240,0);
    p.y = map(p.y,90,900,320,0);
    x=p.y;
    y=p.x;
    Serial.print("X:");
    Serial.println(x); 
    Serial.print("Y:");
    Serial.println(y);
    if (ceckButton(b1,x,y))
    {
      Serial.println("Premuto Button1");
      hideButton(&b1);
      hideButton(&b2);
      drawButton(&indietro);
      ySerial=10;
      tft.setCursor(10, ySerial);
      tft.setTextColor(WHITE);  
      tft.setTextSize(1);
      page=1;
    }
    if (ceckButton(b2,x,y))
    {
      Serial.println("Premuto Button2");
      hideButton(&b1);
      hideButton(&b2);
      drawButton(&b_presenze);
      drawButton(&b_temp);
      drawButton(&indietro);
      page=2;
    }
    if(ceckButton(indietro,x,y))
    {
      Serial.println("Premuto Indietro");
      if (page == 1 || page == 2 )
      {
        tft.fillScreen(BLACK);
        drawButton(&b1);
        drawButton(&b2);
        hideButton(&indietro);
        page=0;
      }
      if (page == 3)
      {
        tft.fillScreen(BLACK);
        drawButton(&b_presenze);
        drawButton(&b_temp);
        drawButton(&indietro);
        page=2; 
      }
     }

    if (ceckButton(b_temp,x,y))
    {
      Serial.println("Premuto TEMP");
      hideButton(&b_presenze);
      hideButton(&b_temp);
      drawButton(&indietro);
      leggiFile("js.txt");
      page=3;
    }     

  }
  if(page == 1)
  {
    char c='\n';
    uint8_t i = 0;
    while(Serial.available())
    {
       c = Serial.read();
       tft.print(c);
       i++;
       if ( i == 51)
       {
        ySerial=ySerial+8;
        tft.setCursor(10,ySerial);
        i=0;
       }
    }
    if (c != '\n')
    {
      ySerial=ySerial+8;
      tft.setCursor(10,ySerial);
    }
    if (ySerial >= 170)
    {
      tft.fillScreen(BLACK);
      drawButton(&indietro);
      ySerial=10;
    }
  }
}

button createButton(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t r,String t,boolean v){
  button b;
  b.x=x;
  b.y=y;
  b.width=w;
  b.heigth=h;
  b.text=t;
  b.radius=r;
  b.visible=v;
  return b;
  }
void drawButton(button *b)
{
  tft.drawRoundRect(b->x,b->y,b->width,b->heigth,b->radius, tft.color565(0, 255, 0));
  tft.setTextSize(2);
  tft.setCursor((b->x+b->width/5),(b->y+b->heigth/3));
  tft.setTextColor(WHITE);
  tft.println(b->text);
  b->visible=true;
}

void hideButton(button *b){
  tft.drawRoundRect(b->x,b->y,b->width,b->heigth,b->radius, tft.color565(0, 0, 0));
  tft.setTextSize(2);
  tft.setCursor((b->x+b->width/5),(b->y+b->heigth/3));
  tft.setTextColor(BLACK);
  tft.println(b->text);
  b->visible=false;
  }
boolean ceckButton(button b, uint16_t x, uint16_t y){
   if ( x > b.x && x < b.x+b.width && y > b.y && y < b.y+b.heigth && b.visible == true)
    return true;
   else
    return false;
  }

void leggiFile(String nomeFile)
{
  File myFile;
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE);  
  tft.setTextSize(1);
  tft.println("LETTURA:");
  myFile = SD.open(nomeFile);
  Serial.print('@');
  boolean attesa = true;
  char c = 'q';
  while(attesa)
  {
    if(Serial.available() > 0) 
    {
      c=Serial.read();
      if(c == '@')
        attesa=false;
    }
  }
  if (myFile) {
    int riga=0;
    while (myFile.available()) {
      char c = char(myFile.read());
      //tft.print(c);
      Serial.print(c);
      if (c == '\n')
        riga++;
      if (riga==20)
      {
        tft.fillScreen(BLACK);
        riga=0;
        tft.setCursor(0,0);
      }
      boolean attesa = true;
      while(attesa)
      {
        if(Serial.available() > 0) 
        {
          c=Serial.read();
          tft.print(c);
          attesa=false;
        }
      }
    }
    myFile.close();
    Serial.print('€');
    boolean attesa = true;
    char c='a';
    while(attesa)
    {
      if(Serial.available() > 0) 
      {
        c=Serial.read();
        if(c == '€')
          attesa=false;
      }
    }
  } else {
    Serial.println("error opening: "+ nomeFile );
    tft.println("error opening: "+ nomeFile );
  }   
}

