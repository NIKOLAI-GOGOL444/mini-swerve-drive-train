#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// Standard non-clashing I2C hardware pin mappings
#define MPUscl 3
#define MPUsda 17
#define PLEXsda 12
#define PLEXscl 11

// Encoder Pins
#define mod1_Genc 41
#define mod1_Yenc 42
#define mod2_Genc 2
#define mod2_Yenc 1
#define mod3_Genc 14
#define mod3_Yenc 13
#define mod4_Genc 36
#define mod4_Yenc 35

// Motor Driver Pins
#define mod1_DRIVE_IN4 37
#define mod1_DRIVE_IN3 38
#define mod1_STEER_IN2 39
#define mod1_STEER_IN1 40

#define mod2_DRIVE_IN4 43
#define mod2_DRIVE_IN3 21
#define mod2_STEER_IN2 4
#define mod2_STEER_IN1 5

#define mod3_DRIVE_IN4 6
#define mod3_DRIVE_IN3 7
#define mod3_STEER_IN2 37
#define mod3_STEER_IN1 48

#define mod4_DRIVE_IN4 0
#define mod4_DRIVE_IN3 45
#define mod4_STEER_IN2 8
#define mod4_STEER_IN1 18

// Raw Register Command Sets
#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define R_RX_PAYLOAD 0x61
#define FLUSH_RX 0xE2

// nRF24L01 Memory Addresses
#define STATUS_REG 0x07
#define CONFIG_REG 0x00
#define RF_SETUP_REG 0x06
#define EN_AA_REG 0x01
#define FIFO_STATUS_REG 0x17
#define RX_ADDR_P0_REG 0x0A
#define RX_PW_P0_REG 0x11

double prevAngENC1 = 0, prevAngENC2 = 0, prevAngENC3 = 0, prevAngENC4 = 0;
double DriveComp1 = 0, DriveComp2 = 0, DriveComp3 = 0, DriveComp4 = 0;
unsigned long lastCompTime = 0;

// Hardware SPI Pins (Official hardware-optimized ESP32-S3 pins)
const int CE_PIN = 44;
const int CSN_PIN = 10;
const int SCK_PIN = 15;
const int MOSI_PIN = 16;
const int MISO_PIN = 9;

// Volatile Encoder Counters
volatile long Mod1 = 0;
volatile long Mod2 = 0;
volatile long Mod3 = 0;
volatile long Mod4 = 0;

volatile long Ticks1 = 0;
volatile long Ticks2 = 0;
volatile long Ticks3 = 0;
volatile long Ticks4 = 0;

// Telemetry & Control Floats
float RawDriveX = 2048;
float RawDriveY = 2048;
float RawSteerX = 2048;
float RawSteerY = 2048;
float GYx = 0;
float GYy = 0;
float GYz = 0;
float angENC1 = 0;
float angENC2 = 0;
float angENC3 = 0;
float angENC4 = 0;

unsigned long sensortime = 0;
unsigned long printtime = 0;

// Motor State History Counters
int driveIN3_lastMod1 = 0; int driveIN4_lastMod1 = 0; int steerIN1_lastMod1 = 0; int steerIN2_lastMod1 = 0;
int driveIN3_lastMod2 = 0; int driveIN4_lastMod2 = 0; int steerIN1_lastMod2 = 0; int steerIN2_lastMod2 = 0;
int driveIN3_lastMod3 = 0; int driveIN4_lastMod3 = 0; int steerIN1_lastMod3 = 0; int steerIN2_lastMod3 = 0;
int driveIN3_lastMod4 = 0; int driveIN4_lastMod4 = 0; int steerIN1_lastMod4 = 0; int steerIN2_lastMod4 = 0;

float Isum[5] = {0, 0, 0, 0, 0};
float lastERR[5] = {0, 0, 0, 0, 0};

float P1, I1, D1, P2, I2, D2, P3, I3, D3, P4, I4, D4;

float minERR1 = 999, maxERR1 = -999;// TEMPOWARY
float minERR2 = 999, maxERR2 = -999;
float minERR3 = 999, maxERR3 = -999;
float minERR4 = 999, maxERR4 = -999;
unsigned long oscTime = 0;

double kp1 = 1.5;
double ki1 = 0.05;
double kd1 = 0.4;

double kp2 = 1.5;
double ki2 = 0.05;
double kd2 = 0.4;

double kp3 = 1.5;
double ki3 = 0.05;
double kd3 = 0.4;

double kp4 = 1.5;
double ki4 = 0.05;
double kd4 = 0.4;

// FIX #4: OUT1-4 now declared here, and these are the ONLY output variables
// PID() writes to these, and STEERmo() reads these -- names now match.
double OUT1 = 0;
double OUT2 = 0;
double OUT3 = 0;
double OUT4 = 0;

int Disp1X = -54;
int Disp1Y = 54;
int Disp2X = -54;
int Disp2Y = -54;
int Disp3X = 54;
int Disp3Y = 54;
int Disp4X = 54;
int Disp4Y = -54;

double TargSPEED1 = 0;
double TargSPEED2 = 0;
double TargSPEED3 = 0;
double TargSPEED4 = 0;

double TargANG1 = 0;
double TargANG2 = 0;
double TargANG3 = 0;
double TargANG4 = 0;

double ERR1 = 0;
double ERR2 = 0;
double ERR3 = 0;
double ERR4 = 0;

struct telemetry {
  float DjoyX;
  float DjoyY;
  float SjoyX;
  float SjoyY;
};
telemetry joystickdata;


// --- INTERRUPT SERVICE ROUTINES (ISRs) ---

void IRAM_ATTR INTERmod1() {
  if (digitalRead(mod1_Genc) == LOW) Mod1++; else Mod1--;
}

void IRAM_ATTR INTERmod2() {
  if (digitalRead(mod2_Genc) == LOW) Mod2++; else Mod2--;
}

void IRAM_ATTR INTERmod3() {
  if (digitalRead(mod3_Genc) == LOW) Mod3++; else Mod3--;
}

void IRAM_ATTR INTERmod4() {
  if (digitalRead(mod4_Genc) == LOW) Mod4++; else Mod4--;
}
void CompensateDrive(int module) {
  double deltaTime = (millis() - lastCompTime) / 1000.0;   // seconds since last loop
  if (deltaTime <= 0) deltaTime = 0.001;  // guard against divide-by-zero on the very first call

  double currentAng, prevAng, targSpeed;
  switch (module) {
    case 1: currentAng = angENC1; prevAng = prevAngENC1; targSpeed = TargSPEED1; break;
    case 2: currentAng = angENC2; prevAng = prevAngENC2; targSpeed = TargSPEED2; break;
    case 3: currentAng = angENC3; prevAng = prevAngENC3; targSpeed = TargSPEED3; break;
    case 4: currentAng = angENC4; prevAng = prevAngENC4; targSpeed = TargSPEED4; break;
    default: return;
  }

  double deltaAngle = currentAng - prevAng;
  if (deltaAngle > 180) deltaAngle -= 360;      // wraparound-safe, same idea as ERR()
  if (deltaAngle < -180) deltaAngle += 360;

  double steerHousingSpeed = deltaAngle / deltaTime;   // deg/sec
  double compensated = (targSpeed - 0.389 * steerHousingSpeed) / 0.611;
  compensated = constrain(compensated, -4095, 4095);

  switch (module) {
    case 1: DriveComp1 = compensated; prevAngENC1 = currentAng; break;
    case 2: DriveComp2 = compensated; prevAngENC2 = currentAng; break;
    case 3: DriveComp3 = compensated; prevAngENC3 = currentAng; break;
    case 4: DriveComp4 = compensated; prevAngENC4 = currentAng; break;
  }
}

// --- HARDWARE SENSOR INTEGRATIONS ---

void MPUfetchGY() {
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

float PLEXfetch(int module) {
  int channelMap[5] = {0, 7, 6, 5, 2};  // module 2 currently on channel 6 (see conversation history)
  int channel = channelMap[module];

  delayMicroseconds(10);
  if (module >= 1 && module <= 4) {
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
  }
  return 0;
}

void MCUwake() {
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
}


// --- MOTOR DRIVE CONTROLLERS ---

void DRIVEmo(int modNumD) {
  if (modNumD == 1) {
    if (DriveComp1  > 0) {
      ledcDetach(mod1_DRIVE_IN4);
      pinMode(mod1_DRIVE_IN4, OUTPUT); digitalWrite(mod1_DRIVE_IN4, LOW);
      analogWrite(mod1_DRIVE_IN3, (int)DriveComp1);
      driveIN3_lastMod1 = (int)DriveComp1; driveIN4_lastMod1 = 0;
    } else if (DriveComp1  < 0) {
      ledcDetach(mod1_DRIVE_IN3);
      pinMode(mod1_DRIVE_IN3, OUTPUT); digitalWrite(mod1_DRIVE_IN3, LOW);
      analogWrite(mod1_DRIVE_IN4, (int)(-DriveComp1 ));
      driveIN3_lastMod1 = 0; driveIN4_lastMod1 = (int)(-DriveComp1);
    } else if(DriveComp1 == 0){
      ledcDetach(mod1_DRIVE_IN3);
      ledcDetach(mod1_DRIVE_IN4);
      pinMode(mod1_DRIVE_IN3, OUTPUT); digitalWrite(mod1_DRIVE_IN3, LOW);
      pinMode(mod1_DRIVE_IN4, OUTPUT); digitalWrite(mod1_DRIVE_IN4, LOW);
      driveIN3_lastMod1 = 0; driveIN4_lastMod1 = 0;
    }
  } 
  else if (modNumD == 2){ 
      if (DriveComp2 > 0) {
      ledcDetach(mod2_DRIVE_IN4);
      pinMode(mod2_DRIVE_IN4, OUTPUT); digitalWrite(mod2_DRIVE_IN4, LOW);
      analogWrite(mod2_DRIVE_IN3, (int)DriveComp2);
      driveIN3_lastMod2 = (int)DriveComp2; driveIN4_lastMod2 = 0;
    } else if (DriveComp2 < 0) {
      ledcDetach(mod2_DRIVE_IN3);
      pinMode(mod2_DRIVE_IN3, OUTPUT); digitalWrite(mod2_DRIVE_IN3, LOW);
      analogWrite(mod2_DRIVE_IN4, (int)(-DriveComp2));
      driveIN3_lastMod2 = 0; driveIN4_lastMod2 = (int)(-DriveComp2);
    } else if(DriveComp2 == 0) {
      ledcDetach(mod2_DRIVE_IN3);
      ledcDetach(mod2_DRIVE_IN4);
      pinMode(mod2_DRIVE_IN3, OUTPUT); digitalWrite(mod2_DRIVE_IN3, LOW);
      pinMode(mod2_DRIVE_IN4, OUTPUT); digitalWrite(mod2_DRIVE_IN4, LOW);
      driveIN3_lastMod2 = 0; driveIN4_lastMod2 = 0;
    }
  }
  else if (modNumD == 3) {
    if (DriveComp3 > 0) {
      ledcDetach(mod3_DRIVE_IN4);
      pinMode(mod3_DRIVE_IN4, OUTPUT); digitalWrite(mod3_DRIVE_IN4, LOW);
      analogWrite(mod3_DRIVE_IN3, (int)DriveComp3);
      driveIN3_lastMod3 = (int)DriveComp3; driveIN4_lastMod3 = 0;
    } else if (DriveComp3 < 0) {
      ledcDetach(mod3_DRIVE_IN3);
      pinMode(mod3_DRIVE_IN3, OUTPUT); digitalWrite(mod3_DRIVE_IN3, LOW);
      analogWrite(mod3_DRIVE_IN4, (int)(-DriveComp3));
      driveIN3_lastMod3 = 0; driveIN4_lastMod3 = (int)(-DriveComp3);
    } else if (DriveComp3 == 0) {
      ledcDetach(mod3_DRIVE_IN3);
      ledcDetach(mod3_DRIVE_IN4);
      pinMode(mod3_DRIVE_IN3, OUTPUT); digitalWrite(mod3_DRIVE_IN3, LOW);
      pinMode(mod3_DRIVE_IN4, OUTPUT); digitalWrite(mod3_DRIVE_IN4, LOW);
      driveIN3_lastMod3 = 0; driveIN4_lastMod3 = 0;
    }
  }
  else if (modNumD == 4) {
    if (DriveComp4 > 0) {
      ledcDetach(mod4_DRIVE_IN4);
      pinMode(mod4_DRIVE_IN4, OUTPUT); digitalWrite(mod4_DRIVE_IN4, LOW);
      analogWrite(mod4_DRIVE_IN3, (int)DriveComp4);
      driveIN3_lastMod4 = (int)DriveComp4; driveIN4_lastMod4 = 0;
    } else if (DriveComp4 < 0) {
      ledcDetach(mod4_DRIVE_IN3);
      pinMode(mod4_DRIVE_IN3, OUTPUT); digitalWrite(mod4_DRIVE_IN3, LOW);
      analogWrite(mod4_DRIVE_IN4, (int)(-DriveComp4));
      driveIN3_lastMod4 = 0; driveIN4_lastMod4 = (int)(-DriveComp4);
    } else if (DriveComp4 == 0) {
      ledcDetach(mod4_DRIVE_IN3);
      ledcDetach(mod4_DRIVE_IN4);
      pinMode(mod4_DRIVE_IN3, OUTPUT); digitalWrite(mod4_DRIVE_IN3, LOW);
      pinMode(mod4_DRIVE_IN4, OUTPUT); digitalWrite(mod4_DRIVE_IN4, LOW);
      driveIN3_lastMod4 = 0; driveIN4_lastMod4 = 0;
    }
  }
}


void STEERmo(int modNumS) {
  if (modNumS == 1) {
    if (OUT1 > 0) {
      ledcDetach(mod1_STEER_IN1);
      pinMode(mod1_STEER_IN1, OUTPUT); digitalWrite(mod1_STEER_IN1, LOW);
      analogWrite(mod1_STEER_IN2, (int)OUT1);
      steerIN1_lastMod1 = 0; steerIN2_lastMod1 = (int)OUT1;
    } else if (OUT1 < 0) {
      ledcDetach(mod1_STEER_IN2);
      pinMode(mod1_STEER_IN2, OUTPUT); digitalWrite(mod1_STEER_IN2, LOW);
      analogWrite(mod1_STEER_IN1, (int)(-OUT1));
      steerIN1_lastMod1 = (int)(-OUT1); steerIN2_lastMod1 = 0;
    } else {
      ledcDetach(mod1_STEER_IN1);
      ledcDetach(mod1_STEER_IN2);
      pinMode(mod1_STEER_IN1, OUTPUT); digitalWrite(mod1_STEER_IN1, LOW);
      pinMode(mod1_STEER_IN2, OUTPUT); digitalWrite(mod1_STEER_IN2, LOW);
      steerIN1_lastMod1 = 0; steerIN2_lastMod1 = 0;
    }
  }
  else if (modNumS == 2) {
    if (OUT2 > 0) {
      ledcDetach(mod2_STEER_IN1);
      pinMode(mod2_STEER_IN1, OUTPUT); digitalWrite(mod2_STEER_IN1, LOW);
      analogWrite(mod2_STEER_IN2, (int)OUT2);
      steerIN1_lastMod2 = 0; steerIN2_lastMod2 = (int)OUT2;
    } else if (OUT2 < 0) {
      ledcDetach(mod2_STEER_IN2);
      pinMode(mod2_STEER_IN2, OUTPUT); digitalWrite(mod2_STEER_IN2, LOW);
      analogWrite(mod2_STEER_IN1, (int)(-OUT2));
      steerIN1_lastMod2 = (int)(-OUT2); steerIN2_lastMod2 = 0;
    } else {
      ledcDetach(mod2_STEER_IN1);
      ledcDetach(mod2_STEER_IN2);
      pinMode(mod2_STEER_IN1, OUTPUT); digitalWrite(mod2_STEER_IN1, LOW);
      pinMode(mod2_STEER_IN2, OUTPUT); digitalWrite(mod2_STEER_IN2, LOW);
      steerIN1_lastMod2 = 0; steerIN2_lastMod2 = 0;
    }
  }
  else if (modNumS == 3) {
    if (OUT3 > 0) {
      ledcDetach(mod3_STEER_IN1);
      pinMode(mod3_STEER_IN1, OUTPUT); digitalWrite(mod3_STEER_IN1, LOW);
      analogWrite(mod3_STEER_IN2, (int)OUT3);
      steerIN1_lastMod3 = 0; steerIN2_lastMod3 = (int)OUT3;
    } else if (OUT3 < 0) {
      ledcDetach(mod3_STEER_IN2);
      pinMode(mod3_STEER_IN2, OUTPUT); digitalWrite(mod3_STEER_IN2, LOW);
      analogWrite(mod3_STEER_IN1, (int)(-OUT3));
      steerIN1_lastMod3 = (int)(-OUT3); steerIN2_lastMod3 = 0;
    } else {
      ledcDetach(mod3_STEER_IN1);
      ledcDetach(mod3_STEER_IN2);
      pinMode(mod3_STEER_IN1, OUTPUT); digitalWrite(mod3_STEER_IN1, LOW);
      pinMode(mod3_STEER_IN2, OUTPUT); digitalWrite(mod3_STEER_IN2, LOW);
      steerIN1_lastMod3 = 0; steerIN2_lastMod3 = 0;
    }
  }
  else if (modNumS == 4) {
    if (OUT4 > 0) {
      ledcDetach(mod4_STEER_IN1);
      pinMode(mod4_STEER_IN1, OUTPUT); digitalWrite(mod4_STEER_IN1, LOW);
      analogWrite(mod4_STEER_IN2, (int)OUT4);
      steerIN1_lastMod4 = 0; steerIN2_lastMod4 = (int)OUT4;
    } else if (OUT4 < 0) {
      ledcDetach(mod4_STEER_IN2);
      pinMode(mod4_STEER_IN2, OUTPUT); digitalWrite(mod4_STEER_IN2, LOW);
      analogWrite(mod4_STEER_IN1, (int)(-OUT4));
      steerIN1_lastMod4 = (int)(-OUT4); steerIN2_lastMod4 = 0;
    } else {
      ledcDetach(mod4_STEER_IN1);
      ledcDetach(mod4_STEER_IN2);
      pinMode(mod4_STEER_IN1, OUTPUT); digitalWrite(mod4_STEER_IN1, LOW);
      pinMode(mod4_STEER_IN2, OUTPUT); digitalWrite(mod4_STEER_IN2, LOW);
      steerIN1_lastMod4 = 0; steerIN2_lastMod4 = 0;
    }
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

void printMotorState(int MODNUM){
  if (MODNUM == 1){
    Serial.println("MOTOR OUTPUT STATE MODULE 1:");
    Serial.print("  DRIVE_IN3: "); Serial.print(digitalRead(mod1_DRIVE_IN3) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN3_lastMod1); Serial.println(")");
    Serial.print("  DRIVE_IN4: "); Serial.print(digitalRead(mod1_DRIVE_IN4) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN4_lastMod1); Serial.println(")");
    Serial.print("  STEER_IN1: "); Serial.print(digitalRead(mod1_STEER_IN1) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN1_lastMod1); Serial.println(")");
    Serial.print("  STEER_IN2: "); Serial.print(digitalRead(mod1_STEER_IN2) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN2_lastMod1); Serial.println(")");
  } else if (MODNUM == 2){
    Serial.println("MOTOR OUTPUT STATE MODULE 2:");
    Serial.print("  DRIVE_IN3: "); Serial.print(digitalRead(mod2_DRIVE_IN3) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN3_lastMod2); Serial.println(")");
    Serial.print("  DRIVE_IN4: "); Serial.print(digitalRead(mod2_DRIVE_IN4) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN4_lastMod2); Serial.println(")");
    Serial.print("  STEER_IN1: "); Serial.print(digitalRead(mod2_STEER_IN1) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN1_lastMod2); Serial.println(")");
    Serial.print("  STEER_IN2: "); Serial.print(digitalRead(mod2_STEER_IN2) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN2_lastMod2); Serial.println(")");
  } else if (MODNUM == 3){
    Serial.println("MOTOR OUTPUT STATE MODULE 3:");
    Serial.print("  DRIVE_IN3: "); Serial.print(digitalRead(mod3_DRIVE_IN3) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN3_lastMod3); Serial.println(")");
    Serial.print("  DRIVE_IN4: "); Serial.print(digitalRead(mod3_DRIVE_IN4) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN4_lastMod3); Serial.println(")");
    Serial.print("  STEER_IN1: "); Serial.print(digitalRead(mod3_STEER_IN1) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN1_lastMod3); Serial.println(")");
    Serial.print("  STEER_IN2: "); Serial.print(digitalRead(mod3_STEER_IN2) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN2_lastMod3); Serial.println(")");
  } else if (MODNUM == 4){
    Serial.println("MOTOR OUTPUT STATE MODULE 4:");
    Serial.print("  DRIVE_IN3: "); Serial.print(digitalRead(mod4_DRIVE_IN3) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN3_lastMod4); Serial.println(")");
    Serial.print("  DRIVE_IN4: "); Serial.print(digitalRead(mod4_DRIVE_IN4) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(driveIN4_lastMod4); Serial.println(")");
    Serial.print("  STEER_IN1: "); Serial.print(digitalRead(mod4_STEER_IN1) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN1_lastMod4); Serial.println(")");
    Serial.print("  STEER_IN2: "); Serial.print(digitalRead(mod4_STEER_IN2) ? "HIGH" : "LOW");
    Serial.print("  (PWM last set: "); Serial.print(steerIN2_lastMod4); Serial.println(")");
  }
}

void VectorMath(){
  double TargANGraw1 = atan2(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp1X)), ((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp1Y))) * 180.0 / PI;
  double TargSPEEDraw1 = sqrt(pow(((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp1Y)), 2) + pow(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp1X)), 2));
  double TargANGraw2 = atan2(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp2X)), ((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp2Y))) * 180.0 / PI;
  double TargSPEEDraw2 = sqrt(pow(((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp2Y)), 2) + pow(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp2X)), 2));
  double TargANGraw3 = atan2(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp3X)), ((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp3Y))) * 180.0 / PI;
  double TargSPEEDraw3 = sqrt(pow(((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp3Y)), 2) + pow(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp3X)), 2));
  double TargANGraw4 = atan2(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp4X)), ((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp4Y))) * 180.0 / PI;
  double TargSPEEDraw4 = sqrt(pow(((RawDriveX - 2048) - ((RawSteerX - 2048) * Disp4Y)), 2) + pow(((RawDriveY - 2048) + ((RawSteerX - 2048) * Disp4X)), 2));

  double greatestSPEED = max(TargSPEEDraw1, max(TargSPEEDraw2, max(TargSPEEDraw3, TargSPEEDraw4)));

  if(greatestSPEED > 4095){
    TargSPEED1 = TargSPEEDraw1 * (4095 / greatestSPEED);
    TargSPEED2 = TargSPEEDraw2 * (4095 / greatestSPEED);
    TargSPEED3 = TargSPEEDraw3 * (4095 / greatestSPEED);
    TargSPEED4 = TargSPEEDraw4 * (4095 / greatestSPEED);
  } else {
    TargSPEED1 = TargSPEEDraw1;
    TargSPEED2 = TargSPEEDraw2;
    TargSPEED3 = TargSPEEDraw3;
    TargSPEED4 = TargSPEEDraw4;
  }

  // Convert raw targets to 0-360 BEFORE comparing to current encoder angle
  double t1 = TargANGraw1; if (t1 < 0) t1 += 360;
  double t2 = TargANGraw2; if (t2 < 0) t2 += 360;
  double t3 = TargANGraw3; if (t3 < 0) t3 += 360;
  double t4 = TargANGraw4; if (t4 < 0) t4 += 360;

  // Flip decision now compares against the module's ACTUAL current position, not zero
  double e1 = t1 - angENC1; while (e1 > 180) e1 -= 360; while (e1 < -180) e1 += 360;
  if (fabs(e1) > 92) { t1 += 180; if (t1 >= 360) t1 -= 360; TargSPEED1 = -TargSPEED1; }
  TargANG1 = t1;

  double e2 = t2 - angENC2; while (e2 > 180) e2 -= 360; while (e2 < -180) e2 += 360;
  if (fabs(e2) > 92) { t2 += 180; if (t2 >= 360) t2 -= 360; TargSPEED2 = -TargSPEED2; }
  TargANG2 = t2;

  double e3 = t3 - angENC3; while (e3 > 180) e3 -= 360; while (e3 < -180) e3 += 360;
  if (fabs(e3) > 92) { t3 += 180; if (t3 >= 360) t3 -= 360; TargSPEED3 = -TargSPEED3; }
  TargANG3 = t3;

  double e4 = t4 - angENC4; while (e4 > 180) e4 -= 360; while (e4 < -180) e4 += 360;
  if (fabs(e4) > 92) { t4 += 180; if (t4 >= 360) t4 -= 360; TargSPEED4 = -TargSPEED4; }
  TargANG4 = t4;
}
void ERR(int module){
  if (module == 1){
    double ERR1raw = (TargANG1 - angENC1);
    if (ERR1raw > 180){
      ERR1 = ERR1raw - 360;
    } else if (ERR1raw < -180){
      ERR1 = ERR1raw + 360;
    } else {
      ERR1 = ERR1raw;
    }
  }else if (module == 2){
    double ERR2raw = (TargANG2 - angENC2);
    if (ERR2raw > 180){
      ERR2 = ERR2raw - 360;
    } else if (ERR2raw < -180){
      ERR2 = ERR2raw + 360;
    } else {
      ERR2 = ERR2raw;
    }
  }else if (module == 3){
    double ERR3raw = (TargANG3 - angENC3);
    if (ERR3raw > 180){
      ERR3 = ERR3raw - 360;
    } else if (ERR3raw < -180){
      ERR3 = ERR3raw + 360;
    } else {
      ERR3 = ERR3raw;
    }
  }else if (module == 4){
    double ERR4raw = (TargANG4 - angENC4);
    if (ERR4raw > 180){
      ERR4 = ERR4raw - 360;
    } else if (ERR4raw < -180){
      ERR4 = ERR4raw + 360;
    } else {
      ERR4 = ERR4raw;
    }
  }
}

void PID (int module){
  if (module == 1){
    ERR(module);

    Isum[module] = Isum[module] + ERR1;
    Isum[module] = constrain(Isum[module], -200, 200);

    P1 = kp1 * ERR1;
    I1 = ki1 * Isum[module];
    D1 = kd1 * (ERR1 - lastERR[module]);

    lastERR[module] = ERR1;

    OUT1 = constrain(P1 + I1 + D1, -4095, 4095);

  }else if (module == 2){
    ERR(module);

    Isum[module] = Isum[module] + ERR2;
    Isum[module] = constrain(Isum[module], -200, 200);

    P2 = kp2 * ERR2;
    I2 = ki2 * Isum[module];
    D2 = kd2 * (ERR2 - lastERR[module]);

    lastERR[module] = ERR2;

    OUT2 = constrain(P2 + I2 + D2, -4095, 4095);

  }else if (module == 3){
    ERR(module);

    Isum[module] = Isum[module] + ERR3;
    Isum[module] = constrain(Isum[module], -200, 200);

    P3 = kp3 * ERR3;
    I3 = ki3 * Isum[module];
    D3 = kd3 * (ERR3 - lastERR[module]);

    lastERR[module] = ERR3;

    OUT3 = constrain(P3 + I3 + D3, -4095, 4095);

  }else if (module == 4){
    ERR(module);

    Isum[module] = Isum[module] + ERR4;
    Isum[module] = constrain(Isum[module], -200, 200);

    P4 = kp4 * ERR4;
    I4 = ki4 * Isum[module];
    D4 = kd4 * (ERR4 - lastERR[module]);

    lastERR[module] = ERR4;

    OUT4 = constrain(P4 + I4 + D4, -4095, 4095);
  }
}

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000)) {
    delay(10);
  }
  Serial.println("Serial Connected Successfully!");
  analogReadResolution(12);
  delay(100);

  pinMode(mod1_Yenc, INPUT_PULLUP);
  pinMode(mod1_Genc, INPUT_PULLUP);
  pinMode(mod2_Yenc, INPUT_PULLUP);
  pinMode(mod2_Genc, INPUT_PULLUP);
  pinMode(mod3_Yenc, INPUT_PULLUP);
  pinMode(mod3_Genc, INPUT_PULLUP);
  pinMode(mod4_Yenc, INPUT_PULLUP);
  pinMode(mod4_Genc, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(mod1_Yenc), INTERmod1, RISING);
  attachInterrupt(digitalPinToInterrupt(mod2_Yenc), INTERmod2, RISING);
  attachInterrupt(digitalPinToInterrupt(mod3_Yenc), INTERmod3, RISING);
  attachInterrupt(digitalPinToInterrupt(mod4_Yenc), INTERmod4, RISING);

  pinMode(mod1_DRIVE_IN3, OUTPUT);
  pinMode(mod1_DRIVE_IN4, OUTPUT);
  pinMode(mod1_STEER_IN1, OUTPUT);
  pinMode(mod1_STEER_IN2, OUTPUT);

  pinMode(mod2_DRIVE_IN3, OUTPUT);
  pinMode(mod2_DRIVE_IN4, OUTPUT);
  pinMode(mod2_STEER_IN1, OUTPUT);
  pinMode(mod2_STEER_IN2, OUTPUT);

  pinMode(mod3_DRIVE_IN3, OUTPUT);
  pinMode(mod3_DRIVE_IN4, OUTPUT);
  pinMode(mod3_STEER_IN1, OUTPUT);
  pinMode(mod3_STEER_IN2, OUTPUT);

  pinMode(mod4_DRIVE_IN3, OUTPUT);
  pinMode(mod4_DRIVE_IN4, OUTPUT);
  pinMode(mod4_STEER_IN1, OUTPUT);
  pinMode(mod4_STEER_IN2, OUTPUT);

  // --- Radio CE/CSN pins configured BEFORE any SPI activity ---
  pinMode(CE_PIN, OUTPUT);
  pinMode(CSN_PIN, OUTPUT);

  analogWrite(mod1_DRIVE_IN3, 0);
  analogWrite(mod1_DRIVE_IN4, 0);
  analogWrite(mod1_STEER_IN1, 0);
  analogWrite(mod1_STEER_IN2, 0);

  analogWriteFrequency(mod1_DRIVE_IN3, 2000);
  analogWriteFrequency(mod1_DRIVE_IN4, 2000);
  analogWriteFrequency(mod1_STEER_IN1, 2000);
  analogWriteFrequency(mod1_STEER_IN2, 2000);

  digitalWrite(CE_PIN, LOW);
  digitalWrite(CSN_PIN, HIGH);   // idle CSN high (deselected) before SPI starts

  // --- SPI begins BEFORE any register access ---
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1);
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  uint8_t rxProbe = readRegister(STATUS_REG);
  Serial.print("RX PRE-INIT STATUS (should be 0x0E): 0x");
  Serial.println(rxProbe, HEX);

  writeRegister(CONFIG_REG, 0x0F);
  writeRegister(EN_AA_REG, 0x00);
  writeRegister(0x02, 0x01);
  writeRegister(0x03, 0x03);
  writeRegister(0x05, 0x4C);         // Channel 76, matches TX
  writeRegister(RF_SETUP_REG, 0x06);

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

  noInterrupts();
  Ticks1 = Mod1;
  Ticks2 = Mod2;
  Ticks3 = Mod3;
  Ticks4 = Mod4;
  interrupts();

  uint8_t FIFO = readRegister(FIFO_STATUS_REG);
  if ((FIFO & 0x01) == 0){
    digitalWrite(CSN_PIN, LOW);
    SPI.transfer(R_RX_PAYLOAD);
    SPI.transferBytes(NULL, (uint8_t*)&joystickdata, sizeof(joystickdata));
    digitalWrite(CSN_PIN, HIGH);

    writeRegister(STATUS_REG, 0x70);
    writeCommand(FLUSH_RX);

    RawDriveX = joystickdata.DjoyX;
    RawDriveY = joystickdata.DjoyY;
    RawSteerX = joystickdata.SjoyX;
    RawSteerY = joystickdata.SjoyY;
  }

    angENC1 = PLEXfetch(1);
    angENC2 = PLEXfetch(2);
    angENC3 = PLEXfetch(3);
    angENC4 = PLEXfetch(4);

    VectorMath();
    PID(1); PID(2); PID(3); PID(4);

    //TEMP
if (ERR1 < minERR1) minERR1 = ERR1;
if (ERR1 > maxERR1) maxERR1 = ERR1;
if (ERR2 < minERR2) minERR2 = ERR2;
if (ERR2 > maxERR2) maxERR2 = ERR2;
if (ERR3 < minERR3) minERR3 = ERR3;
if (ERR3 > maxERR3) maxERR3 = ERR3;
if (ERR4 < minERR4) minERR4 = ERR4;
if (ERR4 > maxERR4) maxERR4 = ERR4;
//TEMP 

    CompensateDrive(1); CompensateDrive(2); CompensateDrive(3); CompensateDrive(4);
lastCompTime = millis();   // reset for next loop's deltaTime calculation

    DRIVEmo(1); STEERmo(1);
    DRIVEmo(2); STEERmo(2);
    DRIVEmo(3); STEERmo(3);
    DRIVEmo(4); STEERmo(4);

    //temp
    if (millis() - oscTime >= 2000) {   // report every 2 seconds
  oscTime = millis();
  Serial.print("OSC RANGE (deg)  M1:"); Serial.print(maxERR1 - minERR1, 1);
  Serial.print("  M2:"); Serial.print(maxERR2 - minERR2, 1);
  Serial.print("  M3:"); Serial.print(maxERR3 - minERR3, 1);
  Serial.print("  M4:"); Serial.println(maxERR4 - minERR4, 1);

  // reset for the next window
  minERR1 = maxERR1 = ERR1;
  minERR2 = maxERR2 = ERR2;
  minERR3 = maxERR3 = ERR3;
  minERR4 = maxERR4 = ERR4;
}
//temp

  if(millis() - sensortime >= 50) {
    sensortime = millis();
    MPUfetchGY();
  }

  if(millis() - printtime >= 250) {
    printtime = millis();
    Serial.print("RADIO:0x"); Serial.print(readRegister(STATUS_REG), HEX);
    Serial.print("  GYRO X:"); Serial.print(GYx, 1);
    Serial.print(" Y:"); Serial.print(GYy, 1);
    Serial.print(" Z:"); Serial.println(GYz, 1);

    Serial.print("ENC(deg)  M1:"); Serial.print(angENC1, 1);
    Serial.print("  M2:"); Serial.print(angENC2, 1);
    Serial.print("  M3:"); Serial.print(angENC3, 1);
    Serial.print("  M4:"); Serial.println(angENC4, 1);

    Serial.print("TARG ANG  M1:"); Serial.print(TargANG1, 1);
    Serial.print("  M2:"); Serial.print(TargANG2, 1);
    Serial.print("  M3:"); Serial.print(TargANG3, 1);
    Serial.print("  M4:"); Serial.println(TargANG4, 1);

    Serial.print("ERR  M1:"); Serial.print(ERR1, 1);
    Serial.print("  M2:"); Serial.print(ERR2, 1);
    Serial.print("  M3:"); Serial.print(ERR3, 1);
    Serial.print("  M4:"); Serial.println(ERR4, 1);

    Serial.print("STEER OUT  M1:"); Serial.print(OUT1, 0);
    Serial.print("  M2:"); Serial.print(OUT2, 0);
    Serial.print("  M3:"); Serial.print(OUT3, 0);
    Serial.print("  M4:"); Serial.println(OUT4, 0);

    Serial.print("DRIVE SPEED  M1:"); Serial.print(TargSPEED1, 0);
    Serial.print("  M2:"); Serial.print(TargSPEED2, 0);
    Serial.print("  M3:"); Serial.print(TargSPEED3, 0);
    Serial.print("  M4:"); Serial.println(TargSPEED4, 0);

    Serial.print("JOY  Drive(x,y):"); Serial.print(RawDriveX, 0); Serial.print(","); Serial.print(RawDriveY, 0);
    Serial.print("   Steer(x,y):"); Serial.print(RawSteerX, 0); Serial.print(","); Serial.println(RawSteerY, 0);

    printMotorState(1);
    printMotorState(2);
    printMotorState(3);
    printMotorState(4);
    Serial.println("------------------------------------");
  }
  delayMicroseconds(500);
}
