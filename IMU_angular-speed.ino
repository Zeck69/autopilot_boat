#include <Wire.h>
#include <MPU6050.h>


MPU6050 mpu;

double speed;
double previous;
double present; 

void setup() 
{
  mpu.calibrateGyro();
  mpu.setThreshold(3);
  
  speed = 0.0;
}

int angular_speed()
{
  Vector gyro = mpu.readNormalizeGyro();
  return gyro.ZAxis;
}

int linear_speed() 
{
  Vector accel = mpu.readNormalizeAccel();
  return accel.XAxis * get_time()  + speed;
}

int get_time() {
  previous = present;
  present = millis();
  return (present-previous);
}

void loop() {
  
}
