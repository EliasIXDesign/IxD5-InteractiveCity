//***************Includes, definitions and prototypes***************
#include <Eventually.h>

//Determines at which value photoresistors register that a ball is placed on them
#define PHOTO_RES_THRESHOLD 300

//Determines the number of miliseconds before the board resets
//It is set to 10 minutes
#define INACTIVITY_RESET_DELAY 60000

//Input
#define PHOTO_RES1 A0
#define PHOTO_RES2 A1

#define BUTTON1 13

//Output
#define LED_STRIP1 3
#define LED_STRIP2 4
#define DIODE1 5
#define DIODE2 6
#define COLOR_OUT 7

#define SERVO1 8
#define SERVO2 9

//Used as parameters when communicating color to the light controller
enum colors {yellow, green};

bool getPhotoSensorState(int);


//***************Event manager and listeners***************
//This is our manager
//He allows us to create listeners, which are objects that trigger under certain curcumstances
//When they trigger, they call a specific function
//The manager itself regulates which listeners to trigger and when
EvtManager mgr;

//Custom listener, which triggers it's action when a photosensor on the designated pin reaches its opposite state (HIGH or LOW)
class EvtPhotoPinListener : public EvtListener{
  public:
    int pin;
    int threshold;
    bool currentSensorState;
    bool pinTriggerState;
  
    EvtPhotoPinListener(int, bool, EvtAction);
    void setupListener();
    bool isEventTriggered();
};

//Constructor
EvtPhotoPinListener::EvtPhotoPinListener(int conPin, bool conPinTriggerState, EvtAction action) {
  pin = conPin;
  triggerAction = action;
  pinTriggerState = conPinTriggerState;
  delay(40);
};

//Called when the listener is added to the manager
void EvtPhotoPinListener::setupListener() {
};

//Checks to see if this listener should fire it's event
//This is continually called
bool EvtPhotoPinListener::isEventTriggered() {
  
  currentSensorState = getPhotoSensorState(pin);

  //Prints the current values of each photoresistor
  //We use this to determine the appropriate photosensor threshold
  /*Serial.print(pinTriggerState);
  Serial.print(" ");
  Serial.print(getPhotoSensorState(pin));
  Serial.print(" ");
  Serial.print(analogRead(PHOTO_RES1));
  Serial.print("\t");
  Serial.print("\t");
  Serial.print(pinTriggerState);
  Serial.print(" ");
  Serial.print(getPhotoSensorState(pin));
  Serial.print(" ");
  Serial.print(analogRead(PHOTO_RES2));
  Serial.println();*/

  if(currentSensorState == pinTriggerState){
    return true;
  }else{
    return false;
  }
};


//***************Quadrant struct definition***************
//Struct representing a region on the board
//Matches the photoresistor of each region with the output pins pertaining to that region
//These output pins send signals to servos and LED strips
struct region {
  char baseTitle[20];
  int photoSensorPin;
  int LEDPin;
  int diodePin;
  int servoPin;
  };

region region1 = {
  "base1", 
  region1.photoSensorPin = PHOTO_RES1, 
  region1.LEDPin = LED_STRIP1,
  region1.diodePin = DIODE1,
  region1.servoPin = SERVO1,
};
  
region region2 = {
  "base2",
  region2.photoSensorPin = PHOTO_RES2,
  region2.LEDPin = LED_STRIP2,
  region2.diodePin = DIODE2,
  region2.servoPin = SERVO2,
};

//This array contains pointers, which point at the respective regions
//Allows us to access regions and their members in iteration
region* regions[2] = {&region1, &region2};


//***************Main code***************
int maxIndexOfRegionArray = (sizeof(regions) / sizeof(region*)) - 1;

//This is a pointer which points to the region, in which the ball is placed
//Allows us to light and actuate the currently selected region, while keeping other regions idle
region* ballLocation;

void setup() {
  Serial.begin(9600);

  pinMode(region1.LEDPin, OUTPUT);
  pinMode(region2.LEDPin, OUTPUT);
  pinMode(region1.diodePin, OUTPUT);
  pinMode(region2.diodePin, OUTPUT);
  pinMode(COLOR_OUT, OUTPUT);

  pinMode(region1.servoPin, OUTPUT);
  pinMode(region2.servoPin, OUTPUT);

  digitalWrite(region1.LEDPin, LOW);
  digitalWrite(region2.LEDPin, LOW);

  analogWrite(region1.servoPin, LOW);
  analogWrite(region2.servoPin, LOW);
  
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(PHOTO_RES1, INPUT);
  pinMode(PHOTO_RES2, INPUT);

  //Signals to the Raspberry Pi which state we start in
  Serial.println("awaitingPickup");

  //Finds the ball's starting location
  ballLocation = updateBallLocation();

  //Creates an initial listener
  //We assume that the ball start in a ball holder
  mgr.addListener(new EvtPhotoPinListener(ballLocation->photoSensorPin, HIGH, (EvtAction)ballPickedUp));

  lightOnlyCurrentRegionsBase();
  setLightColor(yellow);
}

//Specifies that we use the Eventually loop instead of the normal loop
USE_EVENTUALLY_LOOP(mgr)


//***************Events***************
//One of the following functions/events are called when a listener triggers its specific event
//Each event updates the current state of the artifact
//Additionally, each event deletes all listeners and creates listeners corresponding to the possible actions from the current state
//Each listener corresponds to one possible action from the current state

bool ballPickedUp(){
  //Deletes listeners
  mgr.resetContext();

  //Creates listeners
  //From this state the user can: place the ball in any region
  mgr.addListener(new EvtPhotoPinListener(region1.photoSensorPin, LOW, (EvtAction)ballPlaced));
  mgr.addListener(new EvtPhotoPinListener(region2.photoSensorPin, LOW, (EvtAction)ballPlaced));

  //Makes all bases light yellow, turns off diodes, and disengages servos
  lightAllBases();
  setLightColor(yellow);
  turnOffDiodes();
  disengageServo(region1.servoPin);
  disengageServo(region2.servoPin);

  delay(100);

  //Signals to the Raspberry Pi we're in awaitingBall state
  Serial.println("awaitingBall");
  return true;
}

bool ballPlaced(){
  ballLocation = updateBallLocation();

  //Deletes listeners
  mgr.resetContext();

  //From here the user can: Pick up the ball, press "reveal answer", or be timed out
  mgr.addListener(new EvtPhotoPinListener(ballLocation->photoSensorPin, HIGH, (EvtAction)ballPickedUp));
  mgr.addListener(new EvtPinListener(BUTTON1, 100, (EvtAction)answerChosen));
  mgr.addListener(new EvtTimeListener(INACTIVITY_RESET_DELAY, false, (EvtAction)oneMinutePassedWithoutAction));

  //Turns on only the LED strip and diode of the selected region
  //LED strip turns green
  lightOnlyCurrentRegionsBase();
  setLightColor(green);
  turnOnCurrentRegionsDiodes();

  delay(100);

  //Signals to the Raspberry Pi where the ball is placed
  //Lets it find an appropriate question
  Serial.println(ballLocation->baseTitle);
  return true;
}

bool answerChosen(){
  mgr.resetContext();

  //From here the user can: Pick up the ball, press continue, or be timed out
  mgr.addListener(new EvtPhotoPinListener(ballLocation->photoSensorPin, HIGH, (EvtAction)ballPickedUp));
  mgr.addListener(new EvtPinListener(BUTTON1, 100, (EvtAction)finishChosen));
  mgr.addListener(new EvtTimeListener(INACTIVITY_RESET_DELAY, false, (EvtAction)oneMinutePassedWithoutAction));

  //Engages servo of the current region
  engageServo(ballLocation->servoPin);

  Serial.println("reveal");
  return true;
}

bool finishChosen(){
  mgr.resetContext();

  //From here the user can: Pick up the ball 
  mgr.addListener(new EvtPhotoPinListener(ballLocation->photoSensorPin, HIGH, (EvtAction)ballPickedUp));

  //Resets the board
  //Turns off all diodes, and disengages all servos
  //Only current base lights up, and in yellow
  lightOnlyCurrentRegionsBase();
  setLightColor(yellow);
  turnOffDiodes(); 
  disengageServo(region1.servoPin);
  disengageServo(region2.servoPin);

  delay(100);

  Serial.println("awaitingPickup");
  return true;
}

bool oneMinutePassedWithoutAction(){
  mgr.resetContext();

  //From here the user can: Pick up the ball 
  mgr.addListener(new EvtPhotoPinListener(ballLocation->photoSensorPin, HIGH, (EvtAction)ballPickedUp));

  //Resets the board
  lightOnlyCurrentRegionsBase();
  setLightColor(yellow);
  turnOffDiodes();
  disengageServo(region1.servoPin);
  disengageServo(region2.servoPin);

  Serial.println("awaitingPickup");

  return true;
}


//***************Utility functions***************
//Reads a photosensor and returns whether or not the sensor is considered HIGH or LOW
//in relation to the PHOTO_RES_THRESHOLD
bool getPhotoSensorState(int pin){
  bool sensorState;
  int currentVal = analogRead(pin);
    
    if(currentVal >= PHOTO_RES_THRESHOLD){
      sensorState = HIGH;
    }else{
      sensorState = LOW;
    }
  return sensorState;
}

//Runs through all regions and returns the region with the ball placed
//Note: If somehow multiple regions detect a ball, the function return the first noted region
region* updateBallLocation(){
  region* returnRegion;
  bool quadrantSensorState;
  int i = 0;

  do{
    delay(5);
    quadrantSensorState = getPhotoSensorState(regions[i]->photoSensorPin);
    
    if(quadrantSensorState == LOW){
      returnRegion = regions[i];
    }
    i++;
  }while(quadrantSensorState == HIGH && i <= maxIndexOfRegionArray);
  
  return returnRegion;
}


//These functions signal which ball holders should light up
void lightAllBases(){
  for(int i = 0; i <= maxIndexOfRegionArray; i++){
    digitalWrite(regions[i]->LEDPin, HIGH);
  }
}

void lightOnlyCurrentRegionsBase(){
  for(int i = 0; i <= maxIndexOfRegionArray; i++){
    if(regions[i] == ballLocation){
      digitalWrite(regions[i]->LEDPin, HIGH);
    }else{
      digitalWrite(regions[i]->LEDPin, LOW);
    }
  }
}

//These functions signal which diodes should light up
void turnOnCurrentRegionsDiodes(){
  digitalWrite(ballLocation->diodePin, HIGH);
}

void turnOffDiodes(){   
  for(int i = 0; i <= maxIndexOfRegionArray; i++){ 
    digitalWrite(regions[i]->diodePin, LOW);
  } 
}

//Signals to the light controller which color to give LED strips
void setLightColor(colors color){
  if(color == yellow){
    digitalWrite(COLOR_OUT, LOW);
  }else if(color == green){
    digitalWrite(COLOR_OUT, HIGH);
  }else{
  }
}

//These functions signal which servos to engage or disengage
void engageServo(int outputPin){
  analogWrite(outputPin, 500);
}

void disengageServo(int outputPin){
  analogWrite(outputPin, 0);
}
