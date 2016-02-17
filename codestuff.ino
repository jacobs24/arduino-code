/*
Control software for an Arduino based flight computer using a GY 80 inertial navigation system.

Original author: Stewart Jacobs    (armchairengineer@gmail.com)
Last author: Stewart Jacobs        (armchairengineer@gmail.com)
Code version: In Development

This software controls the pyrotechnic outputs on a small launch vehicle based on vehicle
acceleration, rotation, barometric pressure, and compass headings. The flight computer determines
its location by numeric integration and logs real time flight data to an SD card.

The system runs the following instruments in its current configuration:
  -ADXL345     (http://www.analog.com/media/en/technical-documentation/data-sheets/ADXL345.pdf)
  -L3G4200D    (http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/CD00265057.pdf)
  -HMC5883L    (http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/HMC5883L_3-Axis_Digital_Compass_IC.pdf)
  -BMP085      (http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Pressure/BST-BMP085-DS000-06.pdf)
*/

//Libraries to include in file. Do not remove these
#include <Wire.h>     //Arduino I2C library
#include <SD.h>       //Arduino SD card library
#include <Tone.h>     //Speaker handler library
#inlcude <SPI.h>      //Arduino SPI library

//Device addresses. Do not change these values unless the device is changed
#define GYR 0x68        //Gyroscope I2C device address
#define ACC 0x53        //Accelerometer I2C device address
#define COMP 0x1E       //Digital compass I2C device address

//Accelerometer data registers
//=====================================================
#define ACCXLSB 0x32    //X-axis least significant bit
#define ACCXMSB 0x33    //X-axis most significant bit
#define ACCYLSB 0x34    //Y-axis least significant bit
#define ACCYMSB 0x35    //Y-axis most significant bit
#define ACCZLSB 0x36    //Z-axis least significant bit
#define ACCZMSB 0x37    //Z-axis most significant bit
//====================================================

//Gyroscope data registers
//=====================================================
#define GYRXLSB 0x28    //X-axis least significant bit
#define GYRXLSB 0x29    //X-axis least significant bit
#define GYRXLSB 0x2A    //Y-axis least significant bit
#define GYRXLSB 0x2B    //Y-axis least significant bit
#define GYRXLSB 0x2C    //Z-axis least significant bit
#define GYRXLSB 0x2D    //Z-axis least significant bit
//=====================================================

#define CHIPSELECT 4    //The pin of the arduino connected to the chip select line of the SD card.
#define CALNUM 1024     //Number of times to read a device during a callibration loop. Set to 1024 by default. Increase for greater accuracy.
#define TIMEOUT 5000    //How long the flight computer is supposed to wait until it assumes that no serial connection is present. Set to 5 seconds by default.

boolean inSafeMode;     //If true, the flight computer will only report values back to the serial monitor and will disarm any active pyros. This gets set to true if a serial connection is established.
boolean gyrIsNominal;   //Checks if the gyroscope is operational
boolean accIsNominal;   //Checks if the accelerometer is operational
double accCalX;         //X-axis acceleration read during calibration
double accCalY;         //Y-axis acceleration read during calibration
double accCalZ;         //Z-axis acceleration read during calibration

double gyrSens;        //Conversion factor for converting gyroscope data to degrees per second
unsigned long time;    //How long the flight computer has been powered on

void setup() {
  Wire.Begin();
  Serial.begin(9600);
  
  while (1) {
    int timeout = millis();
    
    //If the flight computer runs for more than 5 seconds without establishing a serial connection, shut off the serial port and run normally.
    if (timeout >= TIMEOUT) {
      Serial.end();
      break;
    }
    
    //If the flight computer detects a serial connection, run in safe mode and print values to the serial monitor.
    if (Serial) {
      inSafeMode = true;
      break;
    }
  }
  
  checkForNominal();
  configure();
  calibrate();
  
  //Only run the SD card if not operating in safe mode
  if (!inSafeMode) {
    //Check if SD card is present and card reader is working
    if (!SD.begin(CHIPSELECT)) {
      
    } else {
      //Attempt to start writing to the SD card. If successful, the file writes the headers and closes.
      File flightData = SD.open("flight_data.txt", FILE_WRITE);
      
      if(flightData) {
        flightData.println("VEL\tX\tY\tZ");
        flightData.close();
      } else {
        
      }
    }
  }
}

void loop() {
  double accVals[] = getAccValues();
  double gyrVals[] = getGyroValues();
  time = millis();
}

/*
Selects which pyro relays to close. Relay options are defined as follows:
  1 - Drogue parachute relay
  2 - Main parachute relay
  3 - 2nd stage ignition relay
This method fires only once when called. Call it again if more than one relay needs to be opened at once.
*/
/*
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
*/

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

/*
Configure the devices before flight and set conversion constants (for converting bytes to real data).
Method only runs once during the startup sequence.
*/
void configure() {
  //Gyroscope configuration
  Wire.beginTransmission(GYR);
  Wire.write(0x20); //Access control register 1
  Wire.write(0xAF); //Set the data rate 400 Hz, disable power-down mode, enable all three axes
  Wire.endTransmission();
  
  Wire.beginTransmission(GYR);
  Wire.write(0x23);  //Access control register 3
  Wire.write(0x00);  //Set the sensitivity to 250 dps
  Wire.endTransmission();
  gyrSens = 0.00875; //Set the gyro digit sensitivity in degrees per second
  
  //Accelerometer configuration
  
  //Compass configuration
  
  //Barometer configuration
}

/*
Obtains a set of readings which are used as a reference value for when the vehicle is at rest.
*/
void calibrate() {
  
  
  for (int i = 0; i < CALNUM; i++) {
    
  }

}

double getGyroValues () {
  byte MSBx, LSBx, MSBy, LSBy, MSBz, LSBz;
  double vals[3];
  
  //Retrieve bits from the gyroscope data registers
  LSBx = readI2C(GYR,GYRLSBX);
  MSBx = readI2C(GYR,GYRMSBX);
  LSBy = readI2C(GYR,GYRLSBY);
  MSBy = readI2C(GYR,GYRMSBY);
  LSBz = readI2C(GYR,GYRLSBZ);
  MSBz = readI2C(GYR,GYRMSBZ);
  
  //Join bits, convert to rotation values, return values as array
  vals[1] = (((MSBx << 8) | LSBx)) * gyrSens;
  vals[2] = (((MSBy << 8) | LSBy)) * gyrSens;
  vals[3] = (((MSBz << 8) | LSBz)) * gyrSens;
  return vals;
}

double getHeadings() {
  
}

double getPressure() {
  
}

/*
Returns the accelerometer readings along all 3 axes. The accelerometer returns
data in Two's Complement form, then the LSB and MSB are joined to produce each
axial reading.
*/
double[] getAccValues() {
  byte MSBx, LSBx, MSBy, LSBy, MSBz, LSBz;
  double vals[3];
  
  //Retrieve bits from the accelerometer data registers
  LSBx = readI2C(ACC,ACCLSBX);
  MSBx = readI2C(ACC,ACCMSBX);
  LSBy = readI2C(ACC,ACCLSBY);
  MSBy = readI2C(ACC,ACCMSBY);
  LSBz = readI2C(ACC,ACCLSBZ);
  MSBz = readI2C(ACC,ACCMSBZ);
  
  //Join bits, convert to acceleration values, return values as array
  vals[1] = (((MSBx << 8) | LSBx)) * 1;
  vals[2] = (((MSBy << 8) | LSBy)) * 1;
  vals[3] = (((MSBz << 8) | LSBz)) * 1;
  return vals;
}

//double integrate() {
  
//}

/*
Write flight data to the SD card. Vehicle altitude, heading, and acceleration are
included in this method.
*/
void write2SD() {
  File flightData = SD.open("flight_data.txt", FILE_WRITE);
}

/*
Read a specified registry in an I2C device. This condenses the number of lines
involved in polling a device.
*/
float readI2C(byte devID, byte regAddr) {
  Wire.beginTransmission(devID);
  Wire.write(regAddr);
  Wire.endTransmission();
  Wire.requestFrom(devID,1);
  while (!wire.available()) {}
  return(Wire.read());
}
