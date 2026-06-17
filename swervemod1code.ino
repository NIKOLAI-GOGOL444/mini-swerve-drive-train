#include <Arduino.h>
#include <Wire.h>

#define MPUscl 3
#define MPUsda 17

#define PLEXsda 12
#define PLEXscl 11

#define DjoyXpin 6
#define DjoyYpin 7

#define SjoyXpin 4
#define SjoyYpin 5

#define mod1_DRIVE_IN4 37
#define mod1_DRIVE_IN3 38

#define mod1_STEER_IN2 39
#define mod1_STEER_IN1 40

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


void DRIVEfetch(){
DjoyXval = analogRead(DjoyXpin);
DjoyYval = analogRead(DjoyYpin);

if (DjoyXval >= 1832.5 && DjoyXval <= 2262.5) {
    DjoyXval = 2048; 
}

if (DjoyYval >= 1897.5 && DjoyYval <= 2197.5) {
    DjoyYval = 2048; 
}

}

void STEERfetch(){
SjoyXval = analogRead(SjoyXpin);
SjoyYval = analogRead(SjoyYpin);

if (SjoyXval >= 1900.0 && SjoyXval <= 1980.0) {
    SjoyXval = 2048; 
}

if (SjoyYval >= 1950.0 && SjoyYval <= 2020.0) {
    SjoyYval = 2048; 
}

}

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

uint8_t angHI = Wire1.read(); uint8_t angLO = Wire1.read();
int rawANG = (angHI << 8) | angLO;
float ANG = (rawANG * 360.0) / 4096.0;

return ANG;
}
return -1.0;
} else{
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
//find ammount joystick is away from center. 
int speed = (cubicY * cubicY * cubicY ) * 4095.0;
//cubic math for motor speed. this makes speed scale like a cubic function! 
//less push makes the motor go slow, while pushing harder increases speed cubically.
//this is done by cubing the precentage of steps that the value is away from the top or bottom parameter(the cubicx/y varriable), depending on direction.
//that number, which is always <= 1, is multiplied by 4095 to be translated back into a motor input the h driver can interpret
//giving us a cubic motor speed!
analogWrite(mod1_DRIVE_IN3, speed);
digitalWrite(mod1_DRIVE_IN4, LOW);
}
else if (DjoyYval < 2048) {
cubicY = (2048.0 - DjoyYval) / 2048.0;
 //because the value of a reverse input would be below the midpoint of 2048, 
//we flip the math to take the cubex/y varriable from 2048. 
//this prevents a negative number, 
//which prevents an output that is either negative, 
//or irreasonable for what we want.

int speed = (cubicY * cubicY * cubicY ) * 4095.0;
digitalWrite(mod1_DRIVE_IN3, LOW);
analogWrite(mod1_DRIVE_IN4, speed);

}
else{
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
}
else if (SjoyXval < 2048) {
cubicX = (2048.0 - SjoyXval) / 2048.0;
int speed = (cubicX * cubicX * cubicX ) * 4095.0;
digitalWrite(mod1_STEER_IN1, LOW);
analogWrite(mod1_STEER_IN2, speed);

}
else{
digitalWrite(mod1_STEER_IN1, LOW);
digitalWrite(mod1_STEER_IN2, LOW);
}
}

void setup() {

Serial.begin(115200);
delay(1000);
analogReadResolution(12);

pinMode(mod1_DRIVE_IN3, OUTPUT);
pinMode(mod1_DRIVE_IN4, OUTPUT);
pinMode(mod1_STEER_IN1, OUTPUT);
pinMode(mod1_STEER_IN2, OUTPUT);

 //prevent motors from overheating
analogWriteFrequency(mod1_DRIVE_IN3, 2000); 
analogWriteFrequency(mod1_DRIVE_IN4, 2000);
analogWriteFrequency(mod1_STEER_IN1, 2000);
analogWriteFrequency(mod1_STEER_IN2, 2000);


Wire.begin(MPUsda, MPUscl);
Wire1.begin(PLEXsda, PLEXscl);
//watchdog times prevent the 12c system from getting stuck on data that wont return after 10 seconds, 
//and forces it to move on to the next input of data.  
Wire.setTimeOut(10); 
Wire1.setTimeOut(10); 

  delay(100);


MCUwake();

}

void loop() {
//code runs in the order its listed, 
//so listing these first will reduce lag between joy and drive. 
DRIVEfetch(); 
DRIVEmo();
STEERfetch();
STEERmo();

if(millis() - sensortime >= 50) {
//sensors only are read every 50 seconds, 
//so the 12c bus doesnt get overwhelmed from input
//that may cause lag for the motors. 
sensortime = millis();
MPUfetchGY();
angENC1 = PLEXfetch(1);

}
if(millis() - printtime >= 250) {
Serial.println("GYRO:");
Serial.print("X val: "); Serial.println(GYx); 
Serial.print("Y val: "); Serial.println(GYy); 
Serial.print("Z val:"); Serial.println(GYz); 

Serial.println("ENCODER:");
Serial.print(angENC1); Serial.println(" degrees");

Serial.println("DRIVE JOY X VAL");
Serial.println(DjoyXval);
Serial.println("DRIVE JOY Y VAL");
Serial.println(DjoyYval);

Serial.println("STEER JOY X VAL");
Serial.println(SjoyXval);
Serial.println("STEER JOY Y VAL");
Serial.println(SjoyYval);
}


}
