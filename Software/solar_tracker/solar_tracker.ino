/*
 * Our names go here
 */
 
#include <ModbusSerial.h>
#include <Modbus.h>

//Flags
boolean rs485 = false;
boolean balanced = false;
boolean azimuth = false;
boolean az_48 = false;
boolean el_48 = false;

const int AZRTE = 12;
const int ELRTE = 13;

const int ISO_PWR_DET = 7;
const int TXRX_ENABLE = 6;

const int EL_BL = 
//Modbus Register Offsets (0-9999)

//Modbus Input Registers (Master Read-Only)
const int SENSOR_AZ_IREG = 1001;
const int SENSOR_EL_IREG = 1111;

//Modbus Holding Registers (Master Read/Write)
const int OVERRIDE_HREG = 111;

//ModbusSerial object
ModbusSerial mb;

void setup() {
  //Modbus Setup
  //Config Modbus Serial (port, speed, byte format)
  mb.config(&Serial, 38400, SERIAL_8N1);
  //Set the Slave ID (1-247)
  mb.setSlaveId(212);
  //Add Input Registers
  mb.addIreg(SENSOR_AZ_IREG);
  mb.addIreg(SENSOR_EL_IREG);
  //Add Holding Registers
  mb.addHreg(OVERRIDE_HREG, 0);
}

void loop() {
  // put your main code here, to run repeatedly:

}
