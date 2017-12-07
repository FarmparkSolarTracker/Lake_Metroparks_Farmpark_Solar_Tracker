/*
 * Farmpark Solar Tracker
 * © William Nourse, Darshan G. Parikh, Paula Van Rooy 2017
 */

#include <Adafruit_PWMServoDriver.h>
#include <ModbusSerial.h>
#include <Modbus.h>
#include <Servo.h>
#include <Wire.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
//Flags
boolean rs485 = false;    //has rs485 data been received?
boolean balanced = false; //is the system lined up with the sun?
boolean azimuth = false;  //does the azimuth need adjusted?
boolean night = false;    //is it nighttime?

byte override = 0x00; //register for button presses(bang-bang)

//Variables
int az_scale = 1;   //scaling factor for azimuth PWM
int el_scale = 1;   //scaling factor for elevation PWM
int pin_state = 0;  //holder variable for digitalRead operations
int pin_state1 = 0; //holder variable for digitalRead operations (bang-bang)
int pin_state2 = 0; //holder variable for digitalRead operations (bang-bang)
int pin_state3 = 0; //holder variable for digitalRead operations (bang-bang)
int pin_state4 = 0; //holder variable for digitalRead operations (bang-bang)
int command = 0;    //velocity which will be sent to the motor
int el_cp = 0;      //output of elevation positional controller
int az_cp = 0;      //output of azimuth positional controller
int v = 0;          //override duty cycle
int az_store = 0;   //holder variable for azimuth sensor reading (bang-bang)
int el_store = 0;   //holder variable for elevation senseor reading (bang-bang)
int wait = 0;       //counter for pausing after an override
unsigned int idle = 0;  //counter for determining if it is nighttime

//Balance thresholds
const int THRESH_L = 100;
const int THRESH_H = 200;

//Configuration jumpers
const int AZRTE = 12;
const int ELRTE = 13;

//GPIO Pins
const int C0 = 14;
const int C1 = 15;
const int C2 = 16;
const int C3 = 17;

//Isolated Interface
const int ISO_PWR_DET = 7;
const int TXRX_ENABLE = 6;

//Motor Pins
const int EL_BL = 8;
const int EL_BH = 9;
const int EL_FL = 4;
const int EL_FH = 5;
const int AZ_BL = 10;
const int AZ_BH = 11;
const int AZ_FL = 2;
const int AZ_FH = 3;

//Sensor Pins
const int EL_SENSE = A6;
const int AZ_SENSE = A7;


//Modbus Register Offsets (0-9999)

//Modbus Input Registers (Master Read-Only)
const int SENSOR_AZ_IREG = 1001;
const int SENSOR_EL_IREG = 1111;

//Modbus Holding Registers (Master Read/Write)
const int OVERRIDE_HREG = 111;

//ModbusSerial object
ModbusSerial mb;

void setup() {
  
  //Modbus Setup//////////////////////////

  //Config Modbus Serial (port, speed, byte format)
  mb.config(& Serial, 38400, TXRX_ENABLE); //Check on how to enable enable pin
  //Set the Slave ID (1-247)
  mb.setSlaveId(212);

  //Add Input Registers
  mb.addIreg(SENSOR_AZ_IREG);
  mb.addIreg(SENSOR_EL_IREG);
  //Add Holding Registers
  mb.addHreg(OVERRIDE_HREG, 0);

  //Pinmodes/////////////////////////////
  pinMode(TXRX_ENABLE, OUTPUT);
  pinMode(EL_BL, OUTPUT);
  pinMode(EL_BH, OUTPUT);
  pinMode(EL_FL, OUTPUT);
  pinMode(EL_FH, OUTPUT);
  pinMode(AZ_BL, OUTPUT);
  pinMode(AZ_BH, OUTPUT);
  pinMode(AZ_FL, OUTPUT);
  pinMode(AZ_FH, OUTPUT);
  
  pinMode(AZRTE, INPUT);
  pinMode(ELRTE, INPUT);
  pinMode(ISO_PWR_DET, INPUT);
  pinMode(C0, INPUT);
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);  

  //Serial.begin(9600);
  //Initialization/////////////////////

  //Set scaling factors
  pin_state = digitalRead(AZRTE);
  if (pin_state == LOW){
    az_scale = 2;
  }
  pin_state = digitalRead(ELRTE);
  if (pin_state == LOW){
    el_scale = 2;
  }

  //Wait until isolated section has fully powered on before starting
  pin_state = digitalRead(ISO_PWR_DET);
  while (pin_state == HIGH){
    pin_state = digitalRead(ISO_PWR_DET);
  }

  pwm.begin();

  pwm.setPWMFreq(60);

  yield();
}

void loop() {
  //Check 485////////////////////////
  //mb.task();
  //Read buttons (bang-bang)
  pin_state1 = digitalRead(C0);
  pin_state2 = digitalRead(C1);
  pin_state3 = digitalRead(C2);
  pin_state4 = digitalRead(C3);
  if (pin_state1 == HIGH){
    override = B00000001;
    // Serial.print("Override = ");
    //Serial.println(override);
  }else if (pin_state2 == HIGH){
    override = B00000010;
    // Serial.print("Override = ");
    //Serial.println(override);
  }else if (pin_state3 == HIGH){
    override = B00000100;
    //Serial.print("Override = ");
    //Serial.println(override);
  }else if (pin_state4 == HIGH){
    override = B00001000;
     //Serial.print("Override = ");
    //Serial.println(override);
  }else{
    override = 0x00;
     //Serial.print("Override = ");
    //Serial.println(override);
  }
  
  //Check override register(bang-bang)
  if (override == 0){
    if (rs485 == true){
      if (wait == 255){
        wait = 0;
        rs485 = false;
      }else{
        wait++;
        //Serial.print("Wait = ");
        //Serial.println(wait);
      }
    }
  }else{
    rs485 = true;
    wait = 0;
    switch(override){
      case B00000001:
        azimuth = false;
        v = 4095;
        break;
      case B00000010:
        azimuth = false;
        v = -4095;
        break;
      case B00000100:
        azimuth = true;
        v = 4095;
        break;
      case B00001000:
        azimuth = true;
        v = -4095;
        break;
      default:
        azimuth = false;
        v = 0;
        break;
    }
  }
/*
  //Determine if night//////////////////
  if(balanced == true){
    if(idle == 65500){
      if(night == false){
        night = true;
        //stop elevation motor
        digitalWrite(EL_BH, LOW);
        digitalWrite(EL_FH, LOW);
        digitalWrite(EL_BL, LOW);
        digitalWrite(EL_FL, LOW);
        //move azimuth fast east
        digitalWrite(AZ_BH, LOW);
        digitalWrite(AZ_FL,HIGH);
        digitalWrite(AZ_BL,LOW);
        analogWrite(AZ_FH,255/az_scale);
        delay(15000); //delay 15 seconds
        //stop azimuth
        digitalWrite(AZ_BH, LOW);
        digitalWrite(AZ_FH, LOW);
        digitalWrite(AZ_BL, LOW);
        digitalWrite(AZ_FL, LOW);
      }
    }
  }else{
    idle = 0;
    night = false;
  }
*/
  //control loop///////////////////////////
  for(int i=0; i<100; i++){
  //Input multiplexer////////////////////
    if (rs485 == true){
      command = v;
      //Serial.println("comm = v");
    }else{
      command = 0;
      //Serial.println("comm = 0");
    }

    /*if (rs485 == true){
      command = v;
      //Serial.println("comm = v");
    }else{
      if(azimuth == true){
        command = az_cp;
      }else{
        command = el_cp;
      }
      command = 0;
      //Serial.println("comm = 0");
    }*/

    //Sensor//////////////////////////////
    mb.Ireg(SENSOR_AZ_IREG, analogRead(AZ_SENSE));
    mb.Ireg(SENSOR_EL_IREG, analogRead(EL_SENSE));
    if((mb.Ireg(SENSOR_AZ_IREG) >= THRESH_L) && (mb.Ireg(SENSOR_AZ_IREG) < THRESH_H)){
       azimuth = false;
      if((mb.Ireg(SENSOR_EL_IREG) >= THRESH_L) && (mb.Ireg(SENSOR_EL_IREG) < THRESH_H)){
        balanced = true;
        az_cp = 0;
        el_cp = 0;
      }else{
        balanced = false;
        az_cp = 0;
        //ctrl_pos(2,T-el);
      }
    
    /*
    az_store = analogRead(AZ_SENSE);
    el_store = analogRead(EL_SENSE);
    if((az_store >= THRESH_L) && (az_store < THRESH_H)){
      azimuth = false;
      if((el_store >= THRESH_L) && (el_store < THRESH_H)){
        balanced = true;
        //ctrl_pos(1,0);
        //ctrl_pos(2,0);
      }else{
        balanced = false;
        //ctrl_pos(1,0);
        //ctrl_pos(2,T-el)
      }
    }else{
      balanced = false;
      //ctrl_pos(2,0);
      //ctrl_pos(1,T-az);
      azimuth = true;
    }*/
    }else{
      balanced = false;
      el_cp = 0;
      //az_cp = something;
      azimuth = true;
    }
  }
  
  //Output demultiplexer//////////////
  if(azimuth == true){
    //stop elevation
    digitalWrite(EL_BH, LOW);
    digitalWrite(EL_FH, LOW);
    digitalWrite(EL_BL, LOW);
    digitalWrite(EL_FL, LOW);
    //Serial.println("stop el");
    if(command >= 0){
      //move azimuth east
      digitalWrite(AZ_BH, LOW);
      digitalWrite(AZ_FL,HIGH);
      digitalWrite(AZ_BL,LOW);
      analogWrite(AZ_FH,command/az_scale);
      
      //Serial.println("az east");
      //Serial.print("comm = ");
      //Serial.println(command);
    }else{
      //move azimuth west
      digitalWrite(AZ_FH,LOW);
      digitalWrite(AZ_BL,HIGH);
      digitalWrite(AZ_FL,LOW);
      analogWrite(AZ_BH,abs(command)/az_scale);
    
      //Serial.println("az west");
      //Serial.print("comm = ");
      //Serial.println(command);
    }
  }else{
    //stop azimuth
    digitalWrite(AZ_BH, LOW);
    digitalWrite(AZ_FH, LOW);
    digitalWrite(AZ_BL, LOW);
    digitalWrite(AZ_FL, LOW);
    //Serial.println("stop az");
    if(command >= 0){
      //move elevation up
      digitalWrite(EL_BH, LOW);
      digitalWrite(EL_FL,HIGH);
      digitalWrite(EL_BL,LOW);
      analogWrite(EL_FH,command/el_scale);
      //Serial.println("el up");
      //Serial.print("comm = ");
      //Serial.println(command);
    }else{
      //move elevation down
      digitalWrite(EL_FH,LOW);
      digitalWrite(EL_BL,HIGH);
      digitalWrite(EL_FL,LOW);
      analogWrite(EL_BH,abs(command)/el_scale);
      //Serial.println("el down");
      //Serial.print("comm = ");
      //Serial.println(command);
    }
  }
}

