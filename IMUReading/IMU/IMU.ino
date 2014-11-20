//========================= IMU Sensor Processing ==========================
/* RoboHAZMAT Senior Design Team
 * Motion Control Group
 * Gerardo Bledt
 * November 19, 2014
 *
 * Makes use of the open source IMU sensor library written for the MPU 6050
 * and the MPU 9150. Communicates over I2C with the arduino to read in all of
 * the necessary values. Calibrates the sensor measurements to correct the 
 * offsets of the sensor. Reads the values, converts them to SI units, does
 * direct integration on the gyroscope data, runs the data through a Kalman
 * Filter to estimate the orientation in 3D space, and prints the results.
 * 
 * 1. Parameter Setup: Defines and initializes all of the necessary 
 *    variables, libraries, and the IMU sensor class.
 *
 * 2. Functions: Contains all of the user created functions to help process
 *    the signals from the IMU.
 *    - Calibrate
 *    - printIMU
 *    - KalmanFilter
 *
 * 3. Main Loop: 
 * 
 *    *** TO DO ***
 *     - Optimize code for speed.
 *     - Kalman Filter method
 *     - Possibly need to use quaternions
 */
 
//============================ Parameter Setup =============================
/* Defines and initializes all of the necessary libraries, variables, and 
 * the IMU class.
 */

// Arduino Wire library is required for the I2C communication
#include <Wire.h>
#include <I2Cdev.h>

// Libraries added to get IMU specific data
#include <helper_3dmath.h>
#include <MPU6050_6Axis_MotionApps20.h>
//#include <MPU6050.h>

// Initializes MPU6050 class (default I2C address is 0x68)
MPU6050 IMU;

// Sets up time history variable
int timePrev = 0;
  
// Readings for the accelerometer, gyroscope, and magnetometer
int16_t acc[3] = {0,0,0};
int16_t gyro[3] = {0,0,0};
int16_t mag[3] = {0,0,0};

// Direct integration for gyroscopic angle
int16_t gyroAngle[3] = {0,0,0};

// Readings for the accelerometer, gyroscope, and magnetometer
int16_t accCali[3] = {0,0,0};  // may have to add the -g 
int16_t gyroCali[3] = {0,0,0};
int16_t magCali[3] = {0,0,0};

void setup()
{
  // Joins the I2C bus
  Wire.begin();
  
  // Initialize serial communication
  Serial.begin(38400);
  
  // Initialize the IMU
  Serial.println("Initializing IMU...");
  IMU.initialize();
  
  // Test connection to IMU
  Serial.println(IMU.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // Calibrates the IMU 
  boolean calibrated = false;
  while (!calibrated) {
    calibrated = Calibrate();
  }
}

// ========================================================================

/**
 * Calibration method for the IMU. The method assumes the user will keep 
 * the IMU steady throughout calibration. Takes a preset amount of readings
 * and uses those to find an average and estimate the natural offset of 
 * each of the sensor readings.
 */
boolean Calibrate() {
  // Number of readings to take during calibration
  int caliReadings = 5000;
  
  // History vectors for the readings
  int16_t accHist[3] = {0,0,0};
  int16_t gyroHist[3] = {0,0,0};
  //int16_t magHist[3] = {0,0,0};
  
  // Find total readings for the number of readings specified
  for (int i = 0; i < caliReadings; i++) {
    IMU.getMotion9(&acc[0], &acc[1], &acc[2], &gyro[0], &gyro[1], &gyro[2], &mag[0], &mag[1], &mag[2]);
    for (int x = 0; x < 3; x++) {
      accHist[x] += acc[x];
      gyroHist[x] += gyro[x];
      // magHist[x] += mag[x];
    }
  }
  
  // Take the average of the readings to determine sensor offsets
  for (int x = 0; x < 3; x++) {
    // NOTE: gyro and mag mag not need cali stuff added maybe
    accCali[x] += accHist[x] / caliReadings;
    gyroCali[x] += gyroHist[x] / caliReadings;
    //magCali[x] += magHist[x] / caliReadings;
  }
  
  // Successfull calibration if it finds an offset for the acc and gyro
  if ((accCali[0] > 0 || accCali[0] < 0) && (gyroCali[0] > 0 || gyroCali[0] < 0)) {
    return true;
  } 
  return false;
}

/**
 * Reads the IMU raw data using the MPU6050 library. Uses the resolutions 
 * of +/- 2g for the accelerometer, +/- 250 deg/s for the gyroscope, and 
 * +/- 1200 microT for the magnetometer to convert into usable SI units.
 */
void ReadIMU() {
  // MPU6050 function to parse data
  IMU.getMotion9(&acc[0], &acc[1], &acc[2], &gyro[0], &gyro[1], &gyro[2], &mag[0], &mag[1], &mag[2]);
  
  // Convert each measurement into SI units
  for (int x = 0; x < 3; x++) {
    acc[x] = (acc[x] - accCali[x]) * 9.81 / 16384;    // m/s^2
    gyro[x] = (gyro[x] - gyroCali[x]) * 250 / 32768;  // degrees/s
    mag[x] = (mag[x] - magCali[x]) * 1200 / 32768;    // microT
  }
}

/**
 * Implements a standard Kalman Filter to integrate acc and gyro data.
 */
void KalmanFilter() {
  // ***** TO DO *****
}

/**
 * Prints off the calibrated and converted data for the IMU.
 */ 
void printIMU() {
  // Prints accelerometer data
  Serial.print("Acc:\t");
  Serial.print(acc[0]); Serial.print("\t");
  Serial.print(acc[1]); Serial.print("\t");
  Serial.print(acc[2]); Serial.print("\t");
  
  // Prints gyroscope angular velocity data
  Serial.print("|\tGyro:\t");
  Serial.print(gyro[0]); Serial.print("\t");
  Serial.print(gyro[1]); Serial.print("\t");
  Serial.print(gyro[2]); Serial.print("\t");
  
  // Prints gyroscope angular position data
  Serial.print("|\tGyro Angle:\t");
  Serial.print(gyroAngle[0]); Serial.print("\t");
  Serial.print(gyroAngle[1]); Serial.print("\t");
  Serial.print(gyroAngle[2]); Serial.print("\t");
  
  // Prints magnetometer data
  Serial.print("|\tMag:\t");
  Serial.print(mag[0]); Serial.print("\t");
  Serial.print(mag[1]); Serial.print("\t");
  Serial.print(mag[2]); Serial.print("\t");
  
  // Prints the time measurements were taken at
  Serial.println(timePrev);
}



// =======================================================================


/**
 * The main program loop that reads in the converted IMU data, integrates
 * the gyroscope data to find the orientation angles of the IMU. Then the 
 * data is run through a Kalman filter to cancel out the IMU drift.
 */
void loop()
{
  // Read the IMU sensor values and convert to SI units
  ReadIMU();
  
  // Finds the time since last loop (dt)
  int timeCurr = millis();
  int dt = timeCurr - timePrev; // ms
  timePrev = timeCurr;
  
  // Integration for the gyroscope data to find the gyroscope angle
  for (int x = 0; x < 3; x++) {
    gyroAngle[x] += gyro[x]*dt/1000; // degrees
  }
  
  // Prints the converted IMU data
  printIMU();
  
  /*
  // Kalman Filter on the IMU data
  KalmanFilter();
  printIMU();
  */
}
