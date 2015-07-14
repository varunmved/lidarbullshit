#include <Stepper.h>
#include <NewPing.h>

#define TRIGGER_PIN  8 // Arduino pin tied  to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     7  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#include <I2C.h>
#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.
#define    RegisterHighLowB    0x8f          // Register to get both High and Low bytes in 1 call.

//Global Variables
int counter = 0; // loop variable 
int steploc = 0; //Step # for distance i.e. 2200 for infinity, 780 for 2.
float objdist_ft = 0; //distance from sensor
float objdist_in = 0;
int oldsteploc = 0;
int stepcount = 0;

Stepper stepper(200, 5, 4);

void setup() {
  stepper.setSpeed(300);
  Serial.begin(9600);
  I2c.begin(); // Opens & joins the irc bus as master
  delay(100); // Waits to make sure everything is powered up before sending or receiving data
  I2c.timeOut(50); // Sets a timeout to ensure no locking up of sketch if I2C communication fails
  delay(4000);

}

void loop() {
  int distance = 0;
  for (int i = 0; i < 10; i++) {
    // Write 0x04 to register 0x00
    uint8_t nackack = 100; // Setup variable to hold ACK/NACK resopnses
    while (nackack != 0) { // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
      nackack = I2c.write(LIDARLite_ADDRESS, RegisterMeasure, MeasureValue); // Write 0x04 to 0x00
      delay(1); // Wait 1 ms to prevent overpolling
    }

    byte distanceArray[2]; // array to store distance bytes from read function

    // Read 2byte distance from register 0x8f
    nackack = 100; // Setup variable to hold ACK/NACK resopnses
    while (nackack != 0) { // While NACK keep going (i.e. continue polling until sucess message (ACK) is received )
      nackack = I2c.read(LIDARLite_ADDRESS, RegisterHighLowB, 2, distanceArray); // Read 2 Bytes from LIDAR-Lite Address and store in array
      delay(1); // Wait 1 ms to prevent overpolling
    }
    distance += (distanceArray[0] << 8) + distanceArray[1];  // Shift high byte [0] 8 tothe left and add low byte [1] to create 16-bit int
    printf("distance: %d\n", distance);
  }

  // convert to feet
  distance = distance / 30.48 - 39;

  steploc = findLoc(distance);
  stepcount = steploc - oldsteploc;

  //Step the motor
  if (abs(stepcount) > 11)
  {
    stepper.step((stepcount));
  }

  //Serial Output of step parameters
  Serial.print("Object distance: "); Serial.println(distance);
  Serial.print("Step Count: "); Serial.println(stepcount);
  Serial.print("Current Location: "); Serial.println(steploc);
  Serial.print("Previous Location: "); Serial.println(oldsteploc);

  //Set relative origin
  oldsteploc = abs(steploc);


}

/*
int findLoc(float dist)
{
  int newloc;

  if (dist <= 3)
  {
    if (dist > 1.375) {
      newloc = (int) - 421.02 * (dist) * (dist) + 2642.3 * (dist) - 2840.2;
    }
    else {
      newloc = 0;
    }
  }
  else if (dist > 3 && dist <= 7)
  {
    newloc = (int) 591.91 * log(dist) + 687.45;
  }
  else if (dist > 7)
  {
    newloc = 1837.7;
  }
  return newloc;
}
*/
int findLoc(float dist)
{
	int newLoc;
	if (dist < 1.375) newLoc = (int) (-421.02*(dist)*(dist) + 2642.3*(dist) - 2840.2);
	else if (dist > 1.375 && dist < 7) newLoc = (int) ( 591.91 * log(dist) +687.45);
	else newLoc = 1837.7;

	return newLoc;
}







