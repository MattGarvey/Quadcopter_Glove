#include<Wire.h>

const int levelPin = 2;
const int ledPin =  13;
const int flexPin = A0;
const int throtle = 6;      // Pins for throtle, forward, and right
const int fwd = 10;
const int right = 5;
const int MPU_addr = 0x68;  // I2C address of the MPU-6050
  
int16_t GyXraw, GyYraw, GyZraw;
int GyX, GyY, GyZ;          // Used
int GyXoff = 0;             // Offsets to normalize the gyroscope values
int GyYoff = 0;
int GyZoff = 0;
int flexoff = 0;            // Offset to normalize flex sensors
int flex, flexraw;

int fwdDrive = 0;           // Values 'written' to the analog controller
int throtleDrive = 0;
int rightDrive = 0;
int rdoff = 75;             // Offsets for right and fwd movement (so it hovers)
int fdoff = 30;

int steeringTH = 6000;      // Threshold for gyroscope values

int x = 0;  // variable to be updated by the interrupt
int y = 0;  // second interrupt value

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

////// First gyroscope read ///////////
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
////////////////////////////////////////

  attachInterrupt(digitalPinToInterrupt(2), increment, RISING);   // Interrupt init
  attachInterrupt(digitalPinToInterrupt(3), incrementY, RISING);  //
  Serial.begin(9600);                                 //turn on serial communication
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
  while (x == 0 && y % 2 == 0) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
    stablize();
    flexoff = flexraw;
    analogWrite(throtle, 150);
    analogWrite(fwd, 75);
    analogWrite(right, 75);
  }

  if (x % 2 == 0 && y % 2 == 0 && x != 0) {
    digitalWrite(13, HIGH);

    pilot();

  }

  if (x % 2 == 1 && y % 2 == 0) {
    digitalWrite(13, LOW);
    stablize();
    delay(100);
    x++;
  }

  if (y % 2 == 1) {
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    delay(250);
    ring();
  }

}

// Interrupt service routines
void increment() {
  x++;
}

void incrementY() {
  y++;
}

// Helper Methods
void stablize() {
  GyXoff = GyXraw;
  GyYoff = GyYraw;
  GyZoff = GyZraw;
}

void ring() {
  analogWrite(throtle, 0);
  for (int i = 0; i < 153; i++) {
       analogWrite(right, i+rdoff);
       analogWrite(fwd, 153-i+fdoff);
       delay(5);
  }
    for (int i = 0; i < 153; i++) {
       analogWrite(fwd, i+fdoff);
       analogWrite(right, 153-i+rdoff);
       delay(5);
  }
  y++;
}

void pilot() {
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
  lift();
  analogWrite(throtle, throtleDrive);
  analogWrite(fwd, fwdDrive);
  analogWrite(right, rightDrive);
}

void lift() {
  // 1.5/5/255 = 75, so no left or right motion
  // Throtle ranges from 255 to 0, 255 is off, 0 is full
  //
  if (flex > 0)
    throtleDrive = 255 - flex * 1.15;
  else
    throtleDrive = 255;

  if (GyY > steeringTH)
    fwdDrive = 0;
  else if (GyY < -steeringTH)
    fwdDrive = 255;
  else
    fwdDrive = 75+fdoff;

  if (GyX > steeringTH)
    rightDrive = 0;
  else if (GyX < -steeringTH)
    rightDrive = 255;
  else
    rightDrive = 75+rdoff;
}

