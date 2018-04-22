// MPU-6050 Short Example Sketch
#include<Wire.h>

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t Ac[3],Gr[3],Tmp,GyX,GyY,GyZ,clic;

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  clic =0;
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT); 
}
void loop(){
  digitalWrite(6, HIGH); 
  digitalWrite(7, HIGH); 
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
  int16_t sx = analogRead(2);    // read the input pin
  int16_t dx = analogRead(3);
  Ac[0]=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  Ac[1]=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  Ac[2]=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  Gr[0]=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  Gr[1]=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  Gr[2]=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  int i;
  for(i=0;i<3;i++)
    Ac[i]=(Ac[i]*9.81)/16384;
  for(i=0;i<3;i++)
    Gr[i]=Gr[i]/131;
  if (sx != 1023 and dx != 1023)
    clic = 0;
  else
  {
    if (sx == 1023)
    {
      for(i=0;sx == 1023;i++)
        sx = analogRead(2);
      if (i >= 600)
       clic = 1;
    }
    else
    {
      for(i=0;dx == 1023;i++)
        dx = analogRead(3);
      if (i >= 800)
        clic = 2;
    }
  }
  /*Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(Ac[0]);
  Serial.print(" | Y = "); Serial.print(Ac[1]);
  Serial.print(" | Z = "); Serial.println(Ac[2]);*/
  Serial.print("S");
  Serial.print(Ac[0]);
  Serial.print(",");
  Serial.print(Ac[1]);
  Serial.print(",");
  Serial.print(clic);
  Serial.print("E");
  delay(200);
  /*Serial.print("Temperature: "); Serial.print(Tmp/340.00+36.53); Serial.println(" C "); 
  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(GyX);
  Serial.print(" | Y = "); Serial.print(GyY);
  Serial.print(" | Z = "); Serial.println(GyZ);
  Serial.println(" ");*/
}
