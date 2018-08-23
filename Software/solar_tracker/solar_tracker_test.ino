#include <Wire.h>

/*
 * Farmpark Solar Tracker
 * © William Nourse, Darshan G. Parikh, Paula Van Rooy 2017
 */
#include <Adafruit_PWMServoDriver.h>

//declare pwm driver object
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//expander pin numbers of motor drivers
int AZ_FL = 0;
int AZ_FH = 1;
int AZ_BL = 6;
int AZ_BH = 7;
int EL_FL = 2;
int EL_FH = 3;
int EL_BL = 4;
int EL_BH = 5;

//expander pin numbers of LEDs
int LED_ELB = 8;
int LED_ELF = 9;
int LED_AZB = 10;
int LED_AZF = 11;
int LED_ST1 = 12;
int LED_ST2 = 13;
int LED_ST3 = 14;
int LED_ST4 = 15;

//config switch pins
int SEL0 = 8;
int SEL1 = 9;
int SEL2 = 10;
int SEL3 = 11;
int SEL4 = 12;
int SEL5 = 13;

//speed temp variables
int az36 = 0;
int az48 = 0;
int el36 = 0;
int el48 = 0;

int azSpeed = 2048;
int azNeg = 2048;
int elSpeed = 2048;
int elNeg = 2048;

void setup() {
  //initialize expander communication
  pwm.begin();
  pwm.setPWMFreq(60); //Hz

  //Pinmodes
  pinMode(SEL0, INPUT);
  pinMode(SEL1, INPUT);
  pinMode(SEL2, INPUT);
  pinMode(SEL3, INPUT);
  pinMode(SEL4, INPUT);
  pinMode(SEL5, INPUT);

  az36 = digitalRead(SEL0);
  az48 = digitalRead(SEL1);
  el36 = digitalRead(SEL2);
  el48 = digitalRead(SEL4);

  //Set azimuth motor scaling from switches
  if (az48 == HIGH){
    azSpeed = 4096;
    pwm.setPWM(LED_ST1,4096,0);
  }else if (az36 == HIGH){
    azSpeed = 3072;
    pwm.setPWM(LED_ST2,4096,0);
  }else{
    azSpeed = 2048;
  }
  //Set elevation motor scaling from switches
  if (el48 == HIGH){
    elSpeed = 4096;
    pwm.setPWM(LED_ST3,4096,0);
  }else if (el36 == HIGH){
    elSpeed = 3072;
    pwm.setPWM(LED_ST4,4096,0);
  }else{
    elSpeed = 2048;
  }
  azNeg = 4096 - azSpeed;
  elNeg = 4096 - elSpeed;

}

void loop() {
  ctrlMotor(false,true);  //el_fw light, jack shit
  ctrlMotor(true,true);  //az_fw light, jack shit
  delay(1000);
  //freezeMotor();
  //delay(1000);
  //ctrlMotor(true,false);  //az_bw light, az_bw drive
  //ctrlMotor(false,false); //el_bw light, el_bw drive
  //delay(1000);
  freezeMotor();
  //freezeMotor();
  delay(1000);
}

void ctrlMotor(bool az, bool fwd){
  if (az == true){
    if (fwd == true){
      pwm.setPWM(AZ_FH,azSpeed,azNeg);
      pwm.setPWM(LED_AZF,azSpeed,azNeg);
      pwm.setPWM(AZ_FL,4096,0);
      pwm.setPWM(AZ_BH,0,4096);
      pwm.setPWM(AZ_BL,0,4096);
    }else{
      pwm.setPWM(AZ_FH,0,4096);
      pwm.setPWM(AZ_FL,0,4096);
      pwm.setPWM(AZ_BH,azSpeed,azNeg);
      pwm.setPWM(LED_AZB,azSpeed,azNeg);
      pwm.setPWM(AZ_BL,4096,0);
    }
  }else{
    if (fwd == true){
      pwm.setPWM(EL_FH,elSpeed,elNeg);
      pwm.setPWM(LED_ELF,elSpeed,elNeg);
      pwm.setPWM(EL_FL,4096,0);
      pwm.setPWM(EL_BH,0,4096);
      pwm.setPWM(EL_BL,0,4096);
    }else{
      pwm.setPWM(EL_FH,0,4096);
      pwm.setPWM(EL_FL,0,4096);
      pwm.setPWM(EL_BH,elSpeed,elNeg);
      pwm.setPWM(LED_ELB,elSpeed,elNeg);
      pwm.setPWM(EL_BL,4096,0);
    }
  }
}

void freezeMotor(){
    pwm.setPWM(AZ_FH,0,4096);
    pwm.setPWM(AZ_FL,0,4096);
    pwm.setPWM(AZ_BH,0,4096);
    pwm.setPWM(AZ_BL,0,4096);
    pwm.setPWM(LED_AZF,0,4096);
    pwm.setPWM(LED_AZB,0,4096);
    pwm.setPWM(EL_FH,0,4096);
    pwm.setPWM(EL_FL,0,4096);
    pwm.setPWM(EL_BH,0,4096);
    pwm.setPWM(EL_BL,0,4096);
    pwm.setPWM(LED_ELF,0,4096);
    pwm.setPWM(LED_ELB,0,4096);
}

