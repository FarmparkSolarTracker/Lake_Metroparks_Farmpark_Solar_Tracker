
/*
 * Our names go here
 */
 
#include <ModbusSerial.h>
#include <Modbus.h>
#include <Servo.h>

//Flags
boolean rs485 = false;
boolean balanced = false;
boolean azimuth = false;
boolean night = false;

int az_scale = 1;
int el_scale = 1;
int pin_state = 0;
int command = 0;
int el_cp = 0;
int az_cp = 0;
int v = 0;

const int THRESH_L = 100;
const int THRESH_H = 200;

const int AZRTE = 12;
const int ELRTE = 13;

const int ISO_PWR_DET = 7;
const int TXRX_ENABLE = 6;

const int EL_BL = 8;
const int EL_BH = 9;
const int EL_FL = 4;
const int EL_FH = 5;
const int EL_SENSE = A6;

const int AZ_BL = 10;
const int AZ_BH = 11;
const int AZ_FL = 2;
const int AZ_FH = 3;
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
  mb.config(& Serial, 38400, SERIAL_8N1); //Check on how to enable enable pin
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

  //Wait until isolated section has fully powered on
  pin_state = digitalRead(ISO_PWR_DET);
  while (pin_state == HIGH){
    pin_state = digitalRead(ISO_PWR_DET);
  }
}

void loop() {
  //Input multiplexer////////////////////
  if (rs485 == true){
    command = v;
  }else{
    if(azimuth == true){
      command = az_cp;
    }else{
      command = el_cp;
    }
  }

  //Output demultiplexer////////////////
  if (azimuth == true){
    //el velocity = 0
    //az velocity = command
  }else{
    //az velocity = 0;
    //el velocity = command
  }

  //Sensor//////////////////////////////
  mb.Ireg(SENSOR_AZ_IREG, analogRead(AZ_SENSE));
  mb.Ireg(SENSOR_EL_IREG, analogRead(EL_SENSE));
  if((mb.Ireg(SENSOR_AZ_IREG) >= THRESH_L) && (mb.Ireg(SENSOR_AZ_IREG) < THRESH_H)){
     azimuth = false;
    if((mb.Ireg(SENSOR_AZ_IREG) >= THRESH_L) && (mb.Ireg(SENSOR_AZ_IREG) < THRESH_H)){
      balanced = true;
      //ctrl_pos(1,0);
      //ctrl_pos(2,0);
    }else{
      balanced = false;
      //ctrl_pos(1,0);
      //ctrl_pos(2,T-el);
    }
  }else{
    balanced = false;
    //ctrl_pos(2,0);
    //ctrl_pos(1,T-az)
    azimuth = true;
  }
}
