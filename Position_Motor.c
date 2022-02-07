#include <stdio.h>
#include<math.h>
#define PWMR 5
#define PWML 4
#define ENCA 14
#define ENCB 12

long prevT = 0;
int32_t posPrev = 0;

float v1Filt = 0;
float v1Prev = 0;

int32_t Pos = 0;
float a = 0.8;
float eintegral = 0;

//const uint8_t interruptPin = 14;
//volatile byte interruptCounter = 0;
//int numberOfInterrupts = 0;




void ICACHE_RAM_ATTR readEncoder1();

void setup() {

  Serial.begin(115200);
  //pinMode(interruptPin, INPUT);
  pinMode(PWMR, OUTPUT);
  pinMode(PWML,OUTPUT);
   pinMode(ENCA, INPUT);
  pinMode(ENCB,INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder1, RISING);

}

//void handleInterrupt() {
//  interruptCounter++;
//}

void loop() {

// Serial.println(Pos);
//
  long currT = micros();
  float deltaT = ((float) (currT-prevT))/1.0e6;
  float velocity1 =((float) (Pos - posPrev))/deltaT;
  posPrev = Pos;
  prevT = currT;


    // Convert count/s to RPM
  float v1 = velocity1/315.0*60.0;



  // Low-pass filter (25 Hz cutoff)
 v1Filt = 0.99*v1Filt + 0.01*v1 ;//+ 0.0728*v1Prev;
// v1Prev = v1;
// Set a target
  float vt = 200;   // Set a target

   // Compute the control signal u
  float kp = 5.0;
  float ki = 0.1;
  float e = vt-abs(v1Filt);
  eintegral = eintegral + e*deltaT;
  
  float u = kp*e + ki*eintegral;
//
//
//  
//  // Set the motor speed and direction
  int dir = 1;
  if (u<0){
    dir = -1;
  }
  int pwr = (int) fabs(u);
  if(pwr > 255){
    pwr = 255;
  }

  setMotor(dir,pwr,PWMR,PWML);
 // Serial.println(v1);
  //Serial.print(",");
  Serial.println(v1Filt);
//  Serial.print(",");
//  Serial.print(vt);
//  Serial.print(",");
//  Serial.println(e);
//        
//  //Serial.print("e = ");
// // Serial.println(e);


}




void setMotor(int dir, int pwmVal, int in1, int in2){
  //analogWrite(pwm,pwmVal); // Motor speed
  if(dir == 1){ 
    // Turn one way
    analogWrite(in1,pwmVal);
    digitalWrite(in2,LOW);
  }
  else if(dir == -1){
    // Turn the other way
    digitalWrite(in1,LOW);
    analogWrite(in2,pwmVal);
  }
  else{
    // Or dont turn
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);    
  }
}

void readEncoder1(){
  int b = digitalRead(ENCB);
  if(b>0){
    Pos++;
  }
  else{
    Pos--;
  }
}
