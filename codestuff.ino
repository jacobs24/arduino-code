#include <Wire.h>
#include <SD.h>
#include <Tone.h>
#inlcude <SPI.h>

//Device and register addresses. Do not change these values unless the device is changed
#define GYR 0x68        //Gyroscope I2C device address
#define ACC 0x53        //Accelerometer I2C device address
#define COMP 0x1E       //Digital compass I2C device address
#define BAR 0x77

#define CALNUM 1024     //Number of times to read a device during a callibration loop. Set to 1024 by default. Increase for greater accuracy.

bool hasReachedApogee;  //Has the vehicle reached apogee yet
bool hasIgnition;       //Has the first stage engine ingnited yet
bool gyrIsNominal;      //Checks if the gyroscope is operational
bool accIsNominal;      //Checks if the accelerometer is operational
double accCalX;         //X-axis acceleration read during calibration
double accCalY;         //Y-axis acceleration read during calibration
double accCalZ;         //Z-axis acceleration read during calibration
File flightData;        //Text file containing flight position data

void setup() {
  Wire.Begin();
  checkForNominal();
  configure();
  calibrate();
}

void loop() {
//read acceleration
//read gyro
//read compass

}

/*
Selects which pyro relays to close. Relay options are defined as follows:
1 - Drogue parachute relay
2 - Main parachute relay
3 - 2nd stage ignition relay
This method fires only once when called. Call it again if more than one relay needs to be opened at once.
 */
void detonate(int relayNumber) {
  switch (relayNumber) {
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    default:
      break;
  }
}

/*
Check the I2C devices for response by checking the device IDs. 
 */
void checkForNominal() {
  //Request gyroscope device identity. If the response is incorrect, the device has a problem.
  Wire.beginTransmission(GYR);
  Wire.write(0x0F)
  Wire.endTransmission();
  Wire.requestFrom(GYR,1);
  if (Wire.read() == 0xD3) {
    gyrIsNominal = true;
  }
  
  //Request accelerometer device identity. If the response is incorrect, the device has a problem.
  Wire.beginTransmission(ACC);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(ACC,1);
  if (Wire.read() = 0xE5) {
    accIsNominal = true;
  }
}

void configure() {
  //Gyroscope calibration
  Wire.beginTransmission(GYR);
  Wire.write(0x20); //Access control register 1
  Wire.write(0xAF); //Set the data rate 400 Hz, disable power-down mode, enable all three axes
  Wire.endTransmission();
}

void calibrate() {
  for (int i = 0; i < CALNUM; i++) {

  }

}

double getGyroValues () {
  
}

double getAccValues() {
  Wire.beginTransmission(ACC);
  Wire.write(0x32);
  Wire.endTransmission();
  Wire.requestFrom(ACC,1);
  byte LSBx = Wire.read();

  Wire.beginTransmission(ACC);
  Wire.write(0x33);
  Wire.endTransmission();
  Wire.requestFrom(ACC,1);
  byte MSBx = Wire.read();
}
