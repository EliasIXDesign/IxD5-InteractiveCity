
/*
 * Todo:
 * ((- Fix positionsproblemet (Fra et script til et andet)))
 * ((- Implementér offset))
 */

//#include "Region1Servo1.h"
//#include "Region2Servo1.h"

#include <Debug.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define RESTART -1

#define SERVO1 0
#define SERVO2 1
#define SERVO3 2
#define SERVO4 3

#define REGION1_IN 2
#define REGION2_IN 3

#define GLOBAL_DELAY 20

#define SERVO1_NOSPEED_VAL 365
#define SERVO2_NOSPEED_VAL 365 /*356? 361?*/
#define SERVO3_NOSPEED_VAL 365 /*361*/
#define SERVO4_NOSPEED_VAL 365

#define SERVO1_SPEED_OFFSET 1
#define SERVO2_SPEED_OFFSET 6 /*2*/
#define SERVO3_SPEED_OFFSET 6
#define SERVO4_SPEED_OFFSET 2

#define MIN_POS 0
#define MAX_POS 1000

#define STATE_PIN_AWAITING_PICKUP 10
#define STATE_PIN_AWAITING_BALL 11
#define STATE_PIN_AWAITING_ANSWER 12
#define STATE_PIN_REVEAL 13

//***************Servos, regions, and pointers***************

struct script {
  int8_t offset;
  int8_t checkpointInterval;
  
  //Note: Disse tager åbentbart ret meget dynamisk hukommelse
  int16_t Time[10];
  int8_t Speed[10];
  };

struct stateScriptPair {
  script chosenScript;
  script idleScript;
  };

struct sctServo {
  int servoDriverPin;
  int16_t noSpeedVal;
  int8_t speedOffset;

  //0 min - 180 max (Also, denne posisiton er ikke pålidelig)
  int16_t currentPos;
  //int16_t scriptPos;

  int currentScriptInterval;
  int currentIntervalStep;

  stateScriptPair awaitingPickupScriptPair;
  stateScriptPair awaitingBallScriptPair;
  stateScriptPair awaitingAnswerScriptPair;
  stateScriptPair revealScriptPair;

  script* currentScript;
  };

struct region {
  //Bruges til at registrere om bolden er placeret her
  int regionPin;

  //Bruges til at assign'e chosen eller idle script
  bool isBallLocation;
  
  sctServo* servos[2];
  };

//Tracker hvor bolden er placeret, så servos kan have forskellige scripts, afhængigt af om bolden er der eller ej
region* ballLocation;

script servo1WheatfieldScript = 
    {
      /*Offset*/0,
      /*Checkpoint*/ 0,
      /*Time*/ {    50,    50,   RESTART},
      /*Speed*/{    20,   -20,   RESTART}
    };

script servo2WheatfieldScript =     
    {
      /*Offset*/0,
      /*Checkpoint*/ 1,
      /*Time*/ {  30,  50,    50,   RESTART},
      /*Speed*/{   0,  20,   -20,   RESTART}
    };

script servo3WheatfieldScript =     
    {
      /*Offset*/0,
      /*Checkpoint*/ 1,
      /*Time*/ {  60,  50,    50,   RESTART},
      /*Speed*/{   0,  20,   -20,   RESTART}
    };

script servo4WheatfieldScript =     
    {
      /*Offset*/0,
      /*Checkpoint*/ 1,
      /*Time*/ {  90,  50,    50,   RESTART},
      /*Speed*/{   0,  20,   -20,   RESTART}
    };




/*script defaultChosenAwaitingBallScript =     
    {
      /*Offset0,
      /*Checkpoint 0,
      /*Time {    20,    20,    10,    10,  RESTART},
      /*Speed{   -30,    30,   -30,    30,  RESTART}
    };*/

script defaultChosenAwaitingAnswerScript =
    {
      /*Offset*/0,
      /*Checkpoint*/ 1,
      /*Time*/ {       50,    200,   RESTART},
      /*Speed*/{       20,      0,   RESTART}
    };

script defaultIdleAwaitingAnswerScript =
    {
      /*Offset*/0,
      /*Checkpoint*/ 1,
      /*Time*/ {        50,    200,  RESTART},
      /*Speed*/{       -20,      0,  RESTART}
    };

script defaultChosenRevealScript =
    {
      /*Offset*/0,
      /*Checkpoint*/ 0,
      /*Time*/ {  10,   10,  RESTART},
      /*Speed*/{  16,  -16,  RESTART}
    };

script defaultIdleScript =     
    {
      /*Offset*/0,
      /*Checkpoint*/ 0,
      /*Time*/ {   200,  RESTART},
      /*Speed*/{     0,  RESTART}
    };



sctServo region1Servo1 = {
  region1Servo1.servoDriverPin = SERVO1,
  region1Servo1.noSpeedVal = SERVO1_NOSPEED_VAL,
  region1Servo1.speedOffset = SERVO1_SPEED_OFFSET,
  region1Servo1.currentPos = 0,
  //region1Servo1.scriptPos = 0,
  region1Servo1.currentScriptInterval = 0,
  region1Servo1.currentIntervalStep = 0,

  region1Servo1.awaitingPickupScriptPair = {
    servo1WheatfieldScript,
    servo1WheatfieldScript
  },

  //NOTE: ballLocation bliver ikke null, når bolden fjernes, så chosen- og idleScript skal være det samme
  region1Servo1.awaitingBallScriptPair = {
    defaultIdleScript,
    defaultIdleScript
  },

  region1Servo1.awaitingAnswerScriptPair = {
    defaultChosenAwaitingAnswerScript,
    defaultIdleAwaitingAnswerScript
  },
  
  region1Servo1.revealScriptPair = {
    defaultChosenRevealScript,
    defaultIdleScript
  },
  
  //Nuværende script
  region1Servo1.currentScript = &(region1Servo1.awaitingPickupScriptPair.idleScript)
};

sctServo region1Servo2 = {
  region1Servo2.servoDriverPin = SERVO2,
  region1Servo2.noSpeedVal = SERVO2_NOSPEED_VAL,
  region1Servo2.speedOffset = SERVO2_SPEED_OFFSET,
  region1Servo2.currentPos = 0,
  //region1Servo1.scriptPos = 0,
  region1Servo2.currentScriptInterval = 0,
  region1Servo2.currentIntervalStep = 0,

  region1Servo2.awaitingPickupScriptPair = {
    servo2WheatfieldScript,
    servo2WheatfieldScript
  },

  //NOTE: ballLocation bliver ikke null, når bolden fjernes, så chosen- og idleScript skal være det samme
  region1Servo2.awaitingBallScriptPair = {
    defaultIdleScript,
    defaultIdleScript
  },

  region1Servo2.awaitingAnswerScriptPair = {
    defaultChosenAwaitingAnswerScript,
    defaultIdleAwaitingAnswerScript
  },
  
  region1Servo2.revealScriptPair = {
    defaultChosenRevealScript,
    defaultIdleScript
  },
  
  //Nuværende script
  region1Servo2.currentScript = &(region1Servo2.awaitingPickupScriptPair.idleScript)
};

sctServo region2Servo1 = {
  region2Servo1.servoDriverPin = SERVO3,
  region2Servo1.noSpeedVal = SERVO3_NOSPEED_VAL,
  region2Servo1.speedOffset = SERVO3_SPEED_OFFSET,
  region2Servo1.currentPos = 0,
  //region1Servo1.scriptPos = 0,
  region2Servo1.currentScriptInterval = 0,
  region2Servo1.currentIntervalStep = 0,

  region2Servo1.awaitingPickupScriptPair = {
    servo3WheatfieldScript,
    servo3WheatfieldScript
  },

  //NOTE: ballLocation bliver ikke null, når bolden fjernes, så chosen- og idleScript skal være det samme
  region2Servo1.awaitingBallScriptPair = {
    defaultIdleScript,
    defaultIdleScript
  },

  region2Servo1.awaitingAnswerScriptPair = {
    defaultChosenAwaitingAnswerScript,
    defaultIdleAwaitingAnswerScript
  },
  
  region2Servo1.revealScriptPair = {
    defaultChosenRevealScript,
    defaultIdleScript
  },

  //Nuværende script
  region2Servo1.currentScript = &(region2Servo1.awaitingPickupScriptPair.idleScript)
};


sctServo region2Servo2 = {
  region2Servo2.servoDriverPin = SERVO4,
  region2Servo2.noSpeedVal = SERVO4_NOSPEED_VAL,
  region2Servo2.speedOffset = SERVO4_SPEED_OFFSET,
  region2Servo2.currentPos = 0,
  //region1Servo1.scriptPos = 0,
  region2Servo2.currentScriptInterval = 0,
  region2Servo2.currentIntervalStep = 0,

  region2Servo2.awaitingPickupScriptPair = {
    servo4WheatfieldScript,
    servo4WheatfieldScript
  },

  //NOTE: ballLocation bliver ikke null, når bolden fjernes, så chosen- og idleScript skal være det samme
  region2Servo2.awaitingBallScriptPair = {
    defaultIdleScript,
    defaultIdleScript
  },

  region2Servo2.awaitingAnswerScriptPair = {
    defaultChosenAwaitingAnswerScript,
    defaultIdleAwaitingAnswerScript
  },
  
  region2Servo2.revealScriptPair = {
    defaultChosenRevealScript,
    defaultIdleScript
  },

  //Nuværende script
  region2Servo2.currentScript = &(region2Servo2.awaitingPickupScriptPair.idleScript)
};

region region1 = {
    REGION1_IN,
    false,
    {&region1Servo1, &region1Servo2}
};

region region2 = {
    REGION2_IN,
    false,
    {&region2Servo1, &region2Servo2}
};

region* regions[2] = {&region1, &region2};

//int maxIndexOfServoArray = (sizeof(servos) / sizeof(sctServo*)) - 1;
int maxIndexOfRegionArray = (sizeof(regions) / sizeof(region*)) - 1;

enum states {awaitingPickup, awaitingBall, awaitingAnswer, reveal};
states mainState = awaitingPickup;

//***************Servo Driver***************

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


//***************Main Code***************

void setup() {
  Serial.begin(9600);

  pwm.begin();
  pwm.setPWMFreq(60);

  Serial.print("maxIndexOfRegionArray: ");
  Serial.println(maxIndexOfRegionArray);
}

void loop() {
  //96 - 87 (92 middle) (Asymetric)

  region* thisRegion;
  sctServo* thisServo;
  int thisSpeed;
  
  //Skifter servo'ernes opførsel når main brain'en kommer i et nyt state
  if(newStateReceived()){
    ChangeScripts();
    MoveServosToFirstScriptStep();
  }
  
  for(int i = 0; i <= maxIndexOfRegionArray; i++){
    thisRegion = regions[i];
    
    for(int j = 0; j <= /*Længden af array'et*/((sizeof(thisRegion->servos) / sizeof(sctServo*)) - 1); j++){
      thisServo = thisRegion->servos[j];
      thisSpeed = thisServo->currentScript->Speed[thisServo->currentScriptInterval];
      
      //Tilskriv ny hastghed, HVIS vi er i et nyt interval, og den næste position er indenfor MIN_POS og MAX_POS
      if(
        NewScriptInterval(thisServo)
        && 
        (
         ((thisServo->currentPos + thisSpeed) >= MIN_POS)
         &&
         ((thisServo->currentPos + thisSpeed) <= MAX_POS)
        ))
      {
        
        WriteServoSpeed(thisServo, thisSpeed);
        updatePosition(thisServo, thisSpeed);
        
      }else if(
         ((thisServo->currentPos + thisSpeed) < MIN_POS)
         ||
         ((thisServo->currentPos + thisSpeed) > MAX_POS)
        )
      {
        WriteServoSpeed(thisServo, 0);
        //Serial.print("\t");
        //Serial.print("\t");
      }else
      {
        updatePosition(thisServo, thisSpeed);
      }
  
      MoveToNextScriptStep(thisServo);
      //Serial.println("Inner loop done");
      //Serial.print("\t");

      Serial.print("\t");
    }
  }

  //Gør loop'et synkront med milisekunder
  delay(GLOBAL_DELAY);

  Serial.println();
}


//***************Utility Functions***************

bool newStateReceived(){
  states readState;
  
  //Kunne gøres via iteration, men så skal der være structs eller noget
  if(digitalRead(STATE_PIN_AWAITING_PICKUP) == HIGH){
    readState = awaitingPickup;
  
  }else if(digitalRead(STATE_PIN_AWAITING_BALL) == HIGH){
    readState = awaitingBall;
    
  }else if(digitalRead(STATE_PIN_AWAITING_ANSWER) == HIGH){
    readState = awaitingAnswer;
    
  }else if(digitalRead(STATE_PIN_REVEAL) == HIGH){
    readState = reveal;
    
  }else{
  }
  
  if(mainState != readState){
    //Opdaterer til den nuværende state
    mainState = readState;
      
    ballLocation = UpdateBallLocation();
    Serial.print("ballLocation: ");
    Serial.print(ballLocation->regionPin);
    Serial.println();

    Serial.print("\t");
    Serial.print(digitalRead(STATE_PIN_AWAITING_PICKUP));
    Serial.print("\t");
    Serial.print(digitalRead(STATE_PIN_AWAITING_BALL));
    Serial.print("\t");
    Serial.print(digitalRead(STATE_PIN_AWAITING_ANSWER));
    Serial.print("\t");
    Serial.print(digitalRead(STATE_PIN_REVEAL));
    Serial.print("\t");
    Serial.print("\t");
    Serial.print(digitalRead(region1.regionPin));
    Serial.print("\t");
    Serial.println(digitalRead(region2.regionPin));
    return true;
    
  } else {
    return false;  
  }

}

void ChangeScripts(){
  region* thisRegion;
  sctServo* thisServo;
  
  for(int i = 0; i <= maxIndexOfRegionArray; i++){
    thisRegion = regions[i];

    if(thisRegion->regionPin == ballLocation->regionPin){
      thisRegion->isBallLocation = true;  
    }else{
      thisRegion->isBallLocation = false;
    }
    
    for(int j = 0; j <= /*Længden af array'et*/((sizeof(thisRegion->servos) / sizeof(sctServo*)) - 1); j++){
      thisServo = thisRegion->servos[j];

      //Serial.print("mainState: ");
      //Serial.println(mainState);

      switch(mainState){
        case awaitingPickup:
          //Serial.print("Gets awaitingPickupScriptPair");
          //Serial.println("\t");
          //Serial.print("isBallLocation: ");
          //Serial.println(thisRegion->isBallLocation);
          
          thisServo->currentScript = AssignChosenOrIdleScript(
            &(thisServo->awaitingPickupScriptPair), 
            thisRegion->isBallLocation
          );

          ///*Stub*/thisServo->currentScript = &(thisServo->awaitingPickupScriptPair.chosenScript);
          break;
          
        case awaitingBall:
          //Serial.print("Gets awaitingBallScriptPair");
          //Serial.println("\t");
          //Serial.print("isBallLocation: ");
          //Serial.println(thisRegion->isBallLocation);
          
          thisServo->currentScript = AssignChosenOrIdleScript(
            &(thisServo->awaitingBallScriptPair), 
            thisRegion->isBallLocation
          );

          ///*Stub*/thisServo->currentScript = &(thisServo->awaitingBallScriptPair.chosenScript);
          break;
          
        case awaitingAnswer:
          //Serial.print("Gets awaitingAnswerScriptPair");
          //Serial.println("\t");
          //Serial.print("isBallLocation: ");
          //Serial.println(thisRegion->isBallLocation);
          
          thisServo->currentScript = AssignChosenOrIdleScript(
            &(thisServo->awaitingAnswerScriptPair), 
            thisRegion->isBallLocation
          );

          ///*Stub*/thisServo->currentScript = &(thisServo->awaitingAnswerScriptPair.chosenScript);
          break;
          
        case reveal:
          //Serial.print("Gets revealScriptPair");
          //Serial.println("\t");
          //Serial.print("isBallLocation: ");
          //Serial.println(thisRegion->isBallLocation);
          
          thisServo->currentScript = AssignChosenOrIdleScript(
            &(thisServo->revealScriptPair), 
            thisRegion->isBallLocation
          );
          break;
      }    
    }
  }  
}

void MoveServosToFirstScriptStep(){
  region* thisRegion;
  sctServo* thisServo;
  
  for(int i = 0; i <= maxIndexOfRegionArray; i++){
    thisRegion = regions[i];
    
    for(int j = 0; j <= /*Længden af array'et*/((sizeof(thisRegion->servos) / sizeof(sctServo*)) - 1); j++){
      thisServo = thisRegion->servos[j];
      
      thisServo->currentScriptInterval = 0;
      thisServo->currentIntervalStep = 0;

      /*Serial.println("Trackers reset");
      
      Serial.print("currentScriptInterval: ");
      Serial.print(thisServo->currentScriptInterval);
      Serial.print("\t");
      Serial.print("currentIntervalStep: ");
      Serial.println(thisServo->currentIntervalStep);*/
    }
  }
}


void MoveToNextScriptStep(sctServo* thisSctServo){
  //Incrementer hvor mange milisekunder vi er i det nuværende interval
  thisSctServo->currentIntervalStep++;
  
  //Serial.print(thisSctServo->currentScript.Time[thisSctServo->currentScriptInterval]);
  //Serial.print("\t");

  //Reset'er interval steppet til 0, og interval steppet flyttes til næste, hvis der gås ind i et nyt interval,
  if(thisSctServo->currentIntervalStep 
     == thisSctServo->currentScript->Time[thisSctServo->currentScriptInterval]
  ){
    thisSctServo->currentIntervalStep = 0;
    thisSctServo->currentScriptInterval++;
    //Serial.print(F("Next Step "));
    //Serial.print("\t");
  }

  //Hvis script intervallet rammer intervallet med RESTART, reset'es den til checkpointet
  if(thisSctServo->currentScript->Time[thisSctServo->currentScriptInterval] == RESTART)
  {
    thisSctServo->currentScriptInterval = thisSctServo->currentScript->checkpointInterval;
    //Serial.print("Checkpoint");
  }
  
  /*//Serial.print("currentIntervalStep: ");
  Serial.print(thisSctServo->currentIntervalStep);

  Serial.print("\t");

  //Serial.print("currentScriptInterval: ");
  Serial.print(thisSctServo->currentScriptInterval);

  Serial.print("\t");
  Serial.print("\t");*/
}


bool NewScriptInterval(sctServo* thisSctServo){
  if(thisSctServo->currentIntervalStep == 0){
    //Serial.println("returned true");
    return true;
    
  } else {
    //Serial.println("returned false");
    return false;
  }
}

  
void WriteServoSpeed(sctServo* thisSctServo, int speedPlus){
  //Giver ny fart (fx 92 + 4)
  pwm.setPWM(
    thisSctServo->servoDriverPin, 
    0,
    (thisSctServo->noSpeedVal - (speedPlus + thisSctServo->speedOffset))
  );
    
  //Serial.println(thisSctServo->noSpeedVal + thisSctServo->currentScript.Speed[thisSctServo->currentScriptInterval]);
}


region* UpdateBallLocation(){
  region* returnRegion;

  //Returnerer den sidste region med HIGH
  for(int i = 0; i <= maxIndexOfRegionArray; i++){
    if(digitalRead(regions[i]->regionPin) == HIGH){
      returnRegion = regions[i];
    }
  }
  
  return returnRegion;
}

script* AssignChosenOrIdleScript(stateScriptPair* thisScriptPair, bool isBallLocation){
  
  if(isBallLocation == HIGH){
    Serial.print("chosen");
    Serial.print("\t");
    return &(thisScriptPair->chosenScript);
  }else{
    Serial.print("idle");
    Serial.print("\t");
    return &(thisScriptPair->idleScript);
  }

  //return &(thisScriptPair->idleScript);
}

void updatePosition(sctServo* thisServo, int8_t thisSpeed){
  //Opdaterer servoens position. Add'er speed til
  thisServo->currentPos += thisSpeed;
  Serial.print(thisServo->currentPos);
  //Serial.print(thisSctServo->scriptPos);
  //Serial.print("\t");
  //Serial.print("\t");  
}
