#include<Wire.h>

const int levelPin = 2;
const int ledPin =  13;
const int flexPin = A0;
const int slc = A1;
const int sda = A2;
const int MPU_addr = 0x68; // I2C address of the MPU-6050

int16_t GyXraw, GyYraw, GyZraw;
int GyX, GyY, GyZ;
int GyXoff = 0;
int GyYoff = 0;
int GyZoff = 0;
int flexoff = 0;
int flex, flexraw;

int x = 0;  // variable to be updated by the interrupt
int y = 0;  // landing button

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(levelPin, INPUT);
  pinMode(flexPin, INPUT);
  pinMode(A0, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A1, OUTPUT);

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  flexraw = analogRead(A0);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  GyXraw = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyYraw = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZraw = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  attachInterrupt(digitalPinToInterrupt(2), increment, RISING);
  attachInterrupt(digitalPinToInterrupt(3), incrementY, RISING);
  Serial.begin(9600);  //turn on serial communication
  Serial.print(" | GyX = "); Serial.print(GyXraw);
  Serial.print(" | GyY = "); Serial.print(GyYraw);
  Serial.print(" | GyZ = "); Serial.print(GyZraw);
  Serial.print(" | Flex = "); Serial.println(flexraw);
  GyX = GyXraw;
  GyY = GyYraw;
  GyZ = GyZraw;
  flex = flexraw;
}

void loop() {
  
  while (x == 0 && y%2 == 0) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
    stablize();
    flexoff = flexraw;
  }
  
  if (x%2 == 1 && y%2 == 0) {
    digitalWrite(13, LOW);
    stablize();
    x++;
  }
  
  if (x%2 == 0 && y%2 == 0) {
    digitalWrite(13, HIGH);
    pilot();
    delay(1000);
  }
  if(y%2 == 1){
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    delay(250);
    land();
  }
  
}

// Interrupt service routine for interrupt 0
void increment() {
  x++;
}

void incrementY(){
  y++;
}

void stablize() {
  GyXoff = GyXraw;
  GyYoff = GyYraw;
  GyZoff = GyZraw;
}

void land(){
  
}

void pilot(){
    flexraw = analogRead(A0);
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
    GyXraw = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyYraw = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZraw = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    GyX = GyXraw - GyXoff;
    GyY = GyYraw - GyYoff;
    GyZ = GyZraw - GyZoff;
    flex = flexraw - flexoff;
    Serial.print(" | GyX = "); Serial.print(GyX);
    Serial.print(" | GyY = "); Serial.print(GyY);
    Serial.print(" | GyZ = "); Serial.print(GyZ);
    Serial.print(" | Flex = "); Serial.println(flex);
}

