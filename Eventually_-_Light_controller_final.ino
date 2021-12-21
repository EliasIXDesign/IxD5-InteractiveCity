//***************Includes, definitions and prototypes***************
#include <FastLED.h>

#define MAX_NUM_OF_LEDS    20
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
#define LED_BRIGHTNESS  255

#define STRIP1_NUM_OF_LEDS 13
#define STRIP2_NUM_OF_LEDS 18

//Input:
//For On/Off state of light sources
#define INPUT1 2
#define INPUT2 3
#define INPUT3 4
#define INPUT4 5

//For color of current strips turned on
#define COLOR_IN 13

//Output
#define STRIP1_DATA_PIN 6
#define STRIP2_DATA_PIN 7
#define DIODE1_PIN 8
#define DIODE2_PIN 9

//Used for determining if a light source is a diode or an RGB LED strip
enum lightSourceTypes {diode, LEDStrip};

//***************LED strips***************
//Struct representing a light source
//Matches the input pin (getting signal from the main Arduino) and output pin of each light source
//Also tracks whether or not a strip should light green (On) or yellow (Off), and if a diode should light up or not while flickering
struct lightSource{
  lightSourceTypes type;
  int inputPin;
  int outputPin;

  //Array of the individual LEDs in a strip
  CRGB leds[MAX_NUM_OF_LEDS];
  int numOfLEDs;
  bool isOn;
  bool flickerState;
  };

lightSource strip1 = {
  strip1.type = LEDStrip,
  strip1.inputPin = INPUT1,
  strip1.outputPin = STRIP1_DATA_PIN,
  strip1.numOfLEDs = STRIP1_NUM_OF_LEDS,
  strip1.isOn = false
  };

lightSource strip2 = {
  strip2.type = LEDStrip,
  strip2.inputPin = INPUT2,
  strip2.outputPin = STRIP2_DATA_PIN,
  strip2.numOfLEDs = STRIP2_NUM_OF_LEDS,
  strip2.isOn = false
  };

lightSource diode1 = {
  diode1.type = diode,
  diode1.inputPin = INPUT3,
  diode1.outputPin = DIODE1_PIN,
  diode1.isOn = false,
  strip1.flickerState = false
  };

lightSource diode2 = {
  diode2.type = diode,
  diode2.inputPin = INPUT4,
  diode2.outputPin = DIODE2_PIN,
  diode2.isOn = false,
  strip1.flickerState = false
  };

//Array of all light sources
//Allows iteration across all light sources
lightSource* lightSources[4] = {&strip1, &strip2, &diode1, &diode2};

//***************Main code***************
int maxIndexOfLightSourceArray = (sizeof(lightSources) / sizeof(lightSource*)) - 1;
bool currentColorSignal = LOW;

//Serves as the script for flickering diodes
//Each int is an interval of miliseconds between the diode begins to- or stops lighting up
int flickerIntervals[14] = {
  1600, 50, 
  50, 30, 
  900, 20,
  10, 20, 
  500, 100, 
  40, 30, 
  1000, 10};

//Used for tracking where in the flicker script we currently are
int maxIndexOfIntervalArray = (sizeof(flickerIntervals) / sizeof(int)) - 1;
int currentIntervalIndex = 0;
int millisecondsIntoInterval = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Begin");

  //Sets up the LED arrays, so FastLED can command them
  FastLED.addLeds<CHIPSET, STRIP1_DATA_PIN, COLOR_ORDER>(strip1.leds, strip1.numOfLEDs).setCorrection( TypicalSMD5050 );
  FastLED.addLeds<CHIPSET, STRIP2_DATA_PIN, COLOR_ORDER>(strip2.leds, strip2.numOfLEDs).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( LED_BRIGHTNESS );

  pinMode(DIODE1_PIN, OUTPUT);
  pinMode(DIODE2_PIN, OUTPUT);
}

void loop(){

  //Updates the on/off state of all light sources, when a new signal is received in an input pin
  for(int i = 0; i <= maxIndexOfLightSourceArray; i++){
    if(newOnOffSignal() || newColorSignal()){
      Serial.print("New signal received: ");
      updateLights();
    }
  }
  
  //Progresses through the flicker script
  flickerDiodes();
}


//***************Utility functions***************
//Returns whether or not a new signal (HIGH or LOW) is received from the main Arduino in any input pin
bool newOnOffSignal(){

  //Runs through all light sources
  //If the signal from the light source's input pin is not corresponding to the light source's current on/off state,
  //AKA if there is a new signal, then return true.
  for(int i = 0; i <= maxIndexOfLightSourceArray; i++){
    if(lightSources[i]->isOn != digitalRead(lightSources[i]->inputPin)){
      Serial.println("Light turned on/off");

      //Resets flicker script. Makes sure diodes follow the flicker pattern and not in reverse
      Serial.println("Resets flicker interval variables");
      currentIntervalIndex = 0;
      millisecondsIntoInterval = 0;  
      
      return true;    
    }
  }
  return false;
}

//Returns true if a new color signal is received from the main controller
bool newColorSignal(){
  if(digitalRead(COLOR_IN) != currentColorSignal){
    Serial.println("New color");
    return true;
  }else{
    return false;  
  }
}

//Updates the on/off state and color of all light sources
bool updateLights(){
  CRGB color;
  currentColorSignal = digitalRead(COLOR_IN);
  FastLED.clearData();

  //Updates the current color
  if(currentColorSignal == HIGH){
    color = CRGB::Green;
  }else{
    color = CRGB::Orange;
  }

  //Turns each light source on or off according to their isOn value
  for(int i = 0; i <= maxIndexOfLightSourceArray; i++){

    //Updates the on/off state of each light source
    lightSources[i]-> isOn = digitalRead(lightSources[i]->inputPin);

    //Updates light sources differently depending on thier type
    switch(lightSources[i]->type){
      case LEDStrip:
      
        //For LED strips: Updated to the current color if turned on
        if(lightSources[i]->isOn == true){
          colorRGBStrip(lightSources[i]->leds, lightSources[i]->numOfLEDs, color);
        }else{
        }
        break;
        
      case diode:
        
        //For diodes: Stops lighting up, if they are off
        //Prevents diodes from staying lit up, when they are turned off
        if(lightSources[i]->isOn == false){
          digitalWrite(lightSources[i]->outputPin, LOW);
          lightSources[i]->flickerState = false;
        }
        break;
    }
  }

  //Makes LED strips shine in the new color
  FastLED.show(); 
  return true;
}

//Sets each LED in a strip to a new color
void colorRGBStrip(CRGB leds[], int lastLEDIndex, CRGB color){
  for(int n = 0; n < lastLEDIndex; n++) {
    leds[n] = color;
  }
}

//Progresses to the next step in the flicker script
//Makes diodes start/stop lighting up, when entering a new interval in the flicker script
void flickerDiodes(){

  //Runs through all light sources
  for(int i = 0; i <= maxIndexOfLightSourceArray; i++){  

    //For diodes: Starts/stops lighting up, when entering a new interval in the flicker script
    if(
      millisecondsIntoInterval == 0 
      && lightSources[i]->type == diode 
      && lightSources[i]->isOn == true){

      //Toggles flickerState of the diode
      lightSources[i]->flickerState = !(lightSources[i]->flickerState);

      //Digitally writes flickerState to the diode
      digitalWrite(lightSources[i]->outputPin, lightSources[i]->flickerState);
    }
  }

  //Makes sure the function at least follows real world milliseconds
  delay(1);

  //Tracks the current milisecond into the interval
  millisecondsIntoInterval++;

  //Updates variables when reaching a new interval
  if(millisecondsIntoInterval > flickerIntervals[currentIntervalIndex]){
    currentIntervalIndex++;
    millisecondsIntoInterval = 0;

    Serial.print("currentIntervalIndex: ");
    Serial.println(currentIntervalIndex);
  }

  //Resets currentIntervalIndex when moving away from the final interval
  if(currentIntervalIndex > maxIndexOfIntervalArray){
    currentIntervalIndex = 0;
  }
}
