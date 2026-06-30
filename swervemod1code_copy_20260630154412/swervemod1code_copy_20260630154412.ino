#include <Arduino.h> 
#include <Wire.h> 
#include <SPI.h> 

#define MPUscl 3 
#define MPUsda 17 
#define PLEXsda 12 
#define PLEXscl 11 

#define mod1_DRIVE_IN4 37 
#define mod1_DRIVE_IN3 38 
#define mod1_STEER_IN2 39 
#define mod1_STEER_IN1 40 

#define R_REGISTER 0x00 
#define W_REGISTER 0x20 
#define R_RX_PAYLOAD 0x61 
#define FLUSH_RX 0xE2 

#define STATUS_REG 0x07 
#define CONFIG_REG 0x00 
#define RF_SETUP_REG 0x06 
#define EN_AA_REG 0x01 
#define FIFO_STATUS_REG 0x17 
#define RX_ADDR_P0_REG 0x0A 
#define RX_PW_P0_REG 0x11 

const int CE_PIN = 13; 
const int CSN_PIN = 10; 
const int SCK_PIN = 15; 
const int MOSI_PIN = 16; 
const int MISO_PIN = 9;  

float DjoyXval = 0; 
float DjoyYval = 0; 
float SjoyXval = 0; 
float SjoyYval = 0; 
float GYx = 0; 
float GYy = 0; 
float GYz = 0; 
float angENC1 = 0; 
float cubicY = 0; 
float cubicX = 0; 

unsigned long sensortime = 0; 
unsigned long printtime = 0; 

struct telemetry { 
  float DjoyX; 
  float DjoyY; 
  float SjoyX; 
  float SjoyY; 
}; 
telemetry joystickdata; 

void MPUfetchGY(){ 
  Wire.beginTransmission(0x68); 
  Wire.write(0x43); 
  Wire.endTransmission(false); 
  if (Wire.requestFrom(0x68, 6) == 6) { 
    uint8_t xHI = Wire.read(); uint8_t xLO = Wire.read(); 
    uint8_t yHI = Wire.read(); uint8_t yLO = Wire.read(); 
    uint8_t zHI = Wire.read(); uint8_t zLO = Wire.read(); 
    int rawX = (xHI << 8) | xLO; 
    int rawY = (yHI << 8) | yLO; 
    int rawZ = (zHI << 8) | zLO; 
    GYx = rawX / 131.0; 
    GYy = rawY / 131.0; 
    GYz = rawZ / 131.0; 
  } 
} 

float PLEXfetch(int channel){ 
  delayMicroseconds(10); 
  if (channel <= 4){ 
    Wire1.beginTransmission(0x70); 
    Wire1.write(1 << channel); 
    Wire1.endTransmission(); 
    delay(2); 
    Wire1.beginTransmission(0x36); 
    Wire1.write(0x0E); 
    Wire1.endTransmission(false); 
    if (Wire1.requestFrom(0x36, 2) == 2) { 
      uint8_t angHI = Wire1.read(); 
      uint8_t angLO = Wire1.read(); 
      int rawANG = (angHI << 8) | angLO; 
      float ANG = (rawANG * 360.0) / 4096.0; 
      return ANG; 
    } 
    return -1.0; 
  } else { 
    return 0; 
  } 
} 

void MCUwake(){ 
  Wire.beginTransmission(0x68); 
  Wire.write(0x6B); 
  Wire.write(0x00); 
  Wire.endTransmission(); 
} 

void DRIVEmo(){ 
  if(DjoyYval > 2048){ 
    cubicY = (DjoyYval - 2048.0) / (4095.0 - 2048.0); 
    int speed = (cubicY * cubicY * cubicY ) * 4095.0; 
    analogWrite(mod1_DRIVE_IN3, speed); 
    digitalWrite(mod1_DRIVE_IN4, LOW); 
  } else if (DjoyYval < 2048) { 
    cubicY = (2048.0 - DjoyYval) / 2048.0; 
    int speed = (cubicY * cubicY * cubicY ) * 4095.0; 
    digitalWrite(mod1_DRIVE_IN3, LOW); 
    analogWrite(mod1_DRIVE_IN4, speed); 
  } else { 
    digitalWrite(mod1_DRIVE_IN3, LOW); 
    digitalWrite(mod1_DRIVE_IN4, LOW); 
  } 
} 

void STEERmo(){ 
  if(SjoyXval > 2048){ 
    float cubicX = (SjoyXval - 2048.0) / (4095.0 - 2048.0); 
    int speed = (cubicX * cubicX * cubicX ) * 4095.0; 
    analogWrite(mod1_STEER_IN1, speed); 
    digitalWrite(mod1_STEER_IN2, LOW); 
  } else if (SjoyXval < 2048) { 
    cubicX = (2048.0 - SjoyXval) / 2048.0; 
    int speed = (cubicX * cubicX * cubicX ) * 4095.0; 
    digitalWrite(mod1_STEER_IN1, LOW); 
    analogWrite(mod1_STEER_IN2, speed); 
  } else { 
    digitalWrite(mod1_STEER_IN1, LOW); 
    digitalWrite(mod1_STEER_IN2, LOW); 
  } 
} 

void writeRegister(uint8_t REG, uint8_t value) { 
  digitalWrite(CSN_PIN, LOW); 
  SPI.transfer(W_REGISTER | REG); 
  SPI.transfer(value); 
  digitalWrite(CSN_PIN, HIGH); 
} 

void writeCommand(uint8_t COM) { 
  digitalWrite(CSN_PIN, LOW); 
  SPI.transfer(COM); 
  digitalWrite(CSN_PIN, HIGH); 
} 

void writeAddress(uint8_t REG, uint8_t* addr, int size) {
  digitalWrite(CSN_PIN, LOW);
  SPI.transfer(W_REGISTER | REG);
  for(int x=0; x<size; x++){
    SPI.transfer(addr[x]);
  }
  digitalWrite(CSN_PIN, HIGH);
}

uint8_t readRegister(uint8_t REG){ 
  digitalWrite(CSN_PIN, LOW); 
  SPI.transfer(R_REGISTER | REG); 
  uint8_t result = SPI.transfer(0x00); 
  digitalWrite(CSN_PIN, HIGH); 
  return result; 
} 

void STEERfetch(){ } 
void DRIVEfetch(){ } 

void setup() { 
  Serial.begin(115200); 
  delay(1000); 
  analogReadResolution(12); 
  
  pinMode(mod1_DRIVE_IN3, OUTPUT); 
  pinMode(mod1_DRIVE_IN4, OUTPUT); 
  pinMode(mod1_STEER_IN1, OUTPUT); 
  pinMode(mod1_STEER_IN2, OUTPUT); 
  pinMode(CE_PIN, OUTPUT); 
  pinMode(CSN_PIN, OUTPUT); 
  
  digitalWrite(mod1_DRIVE_IN3, LOW); 
  digitalWrite(mod1_DRIVE_IN4, LOW); 
  digitalWrite(mod1_STEER_IN1, LOW); 
  digitalWrite(mod1_STEER_IN2, LOW); 
  
  analogWriteFrequency(mod1_DRIVE_IN3, 2000); 
  analogWriteFrequency(mod1_DRIVE_IN4, 2000); 
  analogWriteFrequency(mod1_STEER_IN1, 2000); 
  analogWriteFrequency(mod1_STEER_IN2, 2000); 
  
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1); 
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); 
  
  digitalWrite(CE_PIN, LOW); 
  writeRegister(CONFIG_REG, 0x0F);   
  writeRegister(EN_AA_REG, 0x00);    
  writeRegister(0x02, 0x01);         
  writeRegister(0x03, 0x03);         
  writeRegister(0x05, 0x4C);         
  writeRegister(RF_SETUP_REG, 0x06); 
  
  writeRegister(0x1C, 0x01);         
  writeRegister(0x1D, 0x04);         
  
  writeRegister(RX_PW_P0_REG, sizeof(joystickdata)); 
  
  uint8_t rx_addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  writeAddress(RX_ADDR_P0_REG, rx_addr, 5); 
  
  writeCommand(FLUSH_RX); 
  digitalWrite(CE_PIN, HIGH);        
  
  Wire.begin(MPUsda, MPUscl); 
  Wire1.begin(PLEXsda, PLEXscl); 
  Wire.setTimeOut(10); 
  Wire1.setTimeOut(10); 
  delay(100); 
  MCUwake(); 
} 

void loop() { 
  uint8_t FIFO = readRegister(FIFO_STATUS_REG); 
  
  if ((FIFO & 0x01) == 0){ 
    digitalWrite(CSN_PIN, LOW); 
    SPI.transfer(R_RX_PAYLOAD); 
    SPI.transferBytes(NULL, (uint8_t*)&joystickdata, sizeof(joystickdata)); 
    digitalWrite(CSN_PIN, HIGH); 
    
    writeRegister(STATUS_REG, 0x70); 
    writeCommand(FLUSH_RX);
    
    DjoyXval = joystickdata.DjoyX; 
    DjoyYval = joystickdata.DjoyY; 
    SjoyXval = joystickdata.SjoyX; 
    SjoyYval = joystickdata.SjoyY; 
    
    DRIVEmo(); 
    STEERmo(); 
  } 
  
  if(millis() - sensortime >= 50) { 
    sensortime = millis(); 
    MPUfetchGY(); 
    angENC1 = PLEXfetch(1); 
  } 
  
  if(millis() - printtime >= 250) { 
    Serial.print("  RADIO STATUS REGISTER CHECK: 0x"); Serial.println(readRegister(STATUS_REG), HEX);

    printtime = millis(); 
    Serial.println("=== DECODED RX DATA PACKET ==="); 
    
    Serial.print("  RADIO REG-0x07 STATUS: 0x");
    Serial.println(readRegister(STATUS_REG), HEX);
    
    Serial.println("GYRO:"); 
    Serial.print("  X val: "); Serial.println(GYx); 
    Serial.print("  Y val: "); Serial.println(GYy); 
    Serial.print("  Z val: "); Serial.println(GYz); 
    Serial.println("ENCODER:"); 
    Serial.print("  "); Serial.print(angENC1); Serial.println(" degrees"); 
    Serial.print("DRIVE JOY X VAL: "); Serial.println(DjoyXval); 
    Serial.print("DRIVE JOY Y VAL: "); Serial.println(DjoyYval); 
    Serial.print("STEER JOY X VAL: "); Serial.println(SjoyXval); 
    Serial.print("STEER JOY Y VAL: "); Serial.println(SjoyYval); 
    Serial.println("------------------------------------"); 
  } 
  delayMicroseconds(500); 
}
