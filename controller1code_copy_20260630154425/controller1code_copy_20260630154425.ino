#include <Arduino.h> 
#include <Wire.h> 
#include <SPI.h> 

#define DjoyXpin A0  
#define DjoyYpin A1  
#define SjoyXpin A3  
#define SjoyYpin A4  

#define W_REGISTER 0x20 
#define W_TX_PAYLOAD 0xA0 
#define FLUSH_TX 0xE1 

#define CONFIG_REG 0x00 
#define EN_AA_REG 0x01 
#define RF_SETUP_REG 0x06 
#define STATUS_REG 0x07 
#define TX_ADDR_REG 0x10 

const int SCK_PIN = 12; 
const int MOSI_PIN = 11; 
const int MISO_PIN = 10; 
const int CE_PIN = 9; 
const int CSN_PIN = 8; 

float DjoyXval = 0; 
float DjoyYval = 0; 
float SjoyXval = 0; 
float SjoyYval = 0; 
unsigned long printtime = 0; 

struct telemetry { 
  float DjoyX; 
  float DjoyY; 
  float SjoyX; 
  float SjoyY; 
}; 
telemetry joystickdata; 

void writeCommand(uint8_t COM) { 
  digitalWrite(CSN_PIN, LOW); 
  SPI.transfer(COM); 
  digitalWrite(CSN_PIN, HIGH); 
} 

void writeRegister(uint8_t REG, uint8_t value) { 
  digitalWrite(CSN_PIN, LOW); 
  SPI.transfer(W_REGISTER | REG); 
  SPI.transfer(value); 
  digitalWrite(CSN_PIN, HIGH); 
} 

void writeAddress(uint8_t REG, uint8_t* addr, int size) {
  digitalWrite(CSN_PIN, LOW);
  SPI.transfer(W_REGISTER | REG);
  for(int x = 0; x < size; x++){
    SPI.transfer(addr[x]);
  }
  digitalWrite(CSN_PIN, HIGH);
}

void DRIVEfetch(){ 
  DjoyXval = analogRead(DjoyXpin); 
  DjoyYval = analogRead(DjoyYpin); 
  if (DjoyXval >= 1832.5 && DjoyXval <= 2262.5) { DjoyXval = 2048; } 
  if (DjoyYval >= 1897.5 && DjoyYval <= 2197.5) { DjoyYval = 2048; } 
} 

void STEERfetch(){ 
  SjoyXval = analogRead(SjoyXpin); 
  SjoyYval = analogRead(SjoyYpin); 
  if (SjoyXval >= 1900.0 && SjoyXval <= 1980.0) { SjoyXval = 2048; } 
  if (SjoyYval >= 1950.0 && SjoyYval <= 2020.0) { SjoyYval = 2048; } 
} 

void setup() { 
  Serial.begin(115200); 
  delay(1000); 
  Serial.println("--- S3 Native Mapping Enforced ---"); 
  
  pinMode(DjoyXpin, INPUT); 
  pinMode(DjoyYpin, INPUT); 
  pinMode(SjoyXpin, INPUT); 
  pinMode(SjoyYpin, INPUT); 
  
  pinMode(CE_PIN, OUTPUT); 
  pinMode(CSN_PIN, OUTPUT); 
  digitalWrite(CE_PIN, LOW); 
  digitalWrite(CSN_PIN, HIGH); 
  
  analogReadResolution(12); 
  
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1); 
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); 
  
  writeRegister(CONFIG_REG, 0x0E); 
  writeRegister(EN_AA_REG, 0x00);  
  writeRegister(0x03, 0x03);       
  writeRegister(0x04, 0x00);       
  writeRegister(0x05, 0x4C);       
  writeRegister(RF_SETUP_REG, 0x06); 
  
  uint8_t tx_addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
  writeAddress(TX_ADDR_REG, tx_addr, 5); 
  
  Serial.println("--- nRF24L01 Transmitter Active ---"); 
} 

void loop() { 
  DRIVEfetch(); 
  STEERfetch(); 
  
  joystickdata.DjoyX = DjoyXval; 
  joystickdata.DjoyY = DjoyYval; 
  joystickdata.SjoyX = SjoyXval; 
  joystickdata.SjoyY = SjoyYval; 
  
  writeCommand(FLUSH_TX); 
  
  digitalWrite(CSN_PIN, LOW); 
  SPI.transfer(W_TX_PAYLOAD); 
  SPI.writeBytes((uint8_t*)&joystickdata, sizeof(joystickdata)); 
  digitalWrite(CSN_PIN, HIGH); 
  
  digitalWrite(CE_PIN, HIGH); 
  delayMicroseconds(130); 
  digitalWrite(CE_PIN, LOW); 
  
  writeRegister(STATUS_REG, 0x70); 

  if (millis() - printtime >= 250) { 
    printtime = millis(); 
    Serial.println("=== BROADCAST TELEMETRY METRICS ==="); 
    Serial.print("Drive Joystick X: "); Serial.println(joystickdata.DjoyX); 
    Serial.print("Drive Joystick Y: "); Serial.println(joystickdata.DjoyY); 
    Serial.print("Steer Joystick X: "); Serial.println(joystickdata.SjoyX); 
    Serial.print("Steer Joystick Y: "); Serial.println(joystickdata.SjoyY); 
    Serial.println("------------------------------------"); 
  } 
  delay(10); 
}
