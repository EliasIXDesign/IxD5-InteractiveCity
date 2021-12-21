//***************Includes, definitions and prototypes***************
#include <Servo.h>

//Changing actuation increment or delay allows us to quickly try a new actuation speed
#define ACTUATION_INCREMENT 1
#define ACTUATION_DELAY 20
#define MAX_ACTUATION_DEGREE 360

//Input
#define INPUT1 A0
#define INPUT2 A1

//Output
#define SERVO1 6
#define SERVO2 9

//***************Servos***************
//Struct representing a servo
//Matches the input pin (getting signal from the main Arduino) and output pin of each servo
//Also tracks the current position of each servo
struct sServo {
  int inputPin;
  int servoPin;
  int pos;
  Servo servo;
  };

sServo servo1 = {
  servo1.inputPin = INPUT1,
  servo1.servoPin = SERVO1,
  servo1.pos = 0
  };

sServo servo2 = {
  servo2.inputPin = INPUT2,
  servo2.servoPin = SERVO2,
  servo2.pos = 0
  };

//Array of each servo
//Allows iteration across all servos
sServo* servos[2] = {&servo1, &servo2};

//***************Main code***************
bool arduinoIsRunning = true;
int maxIndexOfServoArray = (sizeof(servos) / sizeof(sServo*)) - 1;

void setup() {
  Serial.begin(9600);

  pinMode(INPUT1, INPUT);
  pinMode(INPUT2, INPUT);
  
  servo1.servo.attach(servo1.servoPin);
  servo2.servo.attach(servo2.servoPin);

  servo1.servo.write(0);
  servo2.servo.write(0);
}

//Continually updates all servos' postition
void loop() {
  do{
    updateServoPositions();
    delay(ACTUATION_DELAY);
  }while(arduinoIsRunning);
}


//***************Utility functions***************
//Runs through all servos, checks for each if they ought to be engaged,
//which is determined by the signal received from the main Arduino (HIGH or LOW) in that servos input pin.
//Each servo increments or decrements its position gradually.
//Multiple servos can increment and decrement simultaneously and independently.
void updateServoPositions(){
  int pos;

  for(int i = 0; i <= maxIndexOfServoArray; i++){
    pos = servos[i]->pos;

    //Engages servo, if it ought to be
    if(signalReceived(servos[i]->inputPin) && pos < MAX_ACTUATION_DEGREE){
      pos = pos + ACTUATION_INCREMENT;
      servos[i]->servo.write(pos);
      servos[i]->pos = pos;
      
    //Disengages servo, if it ought to be
    }else if(!signalReceived(servos[i]->inputPin) && pos > 0){
      pos = pos - ACTUATION_INCREMENT;
      servos[i]->servo.write(pos);
      servos[i]->pos = pos;

    }else{
    }
      
    Serial.print(pos);
    Serial.print("\t");
  }

  Serial.println();
}

//Returns whether or not signal is received in an input pin
bool signalReceived(int inputPin){
  if(analogRead(inputPin) >= 500){
    return true;
  }else{
    return false;
  }
}
