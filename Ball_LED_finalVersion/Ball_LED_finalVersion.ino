
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <FastLED.h>

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int8_t threshold, count; 
bool zero_detect; 
bool TurnOnZI = false;


#define LED_PIN     3

// Information about the LED strip itself
#define NUM_LEDS    18
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];


#define BRIGHTNESS  255


void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();

    // initialize serial communication
    Serial.begin(38400);

  
    accelgyro.initialize();
	
    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    //Set up zero motion
	
    accelgyro.setAccelerometerPowerOnDelay(100);
	

    /** Get the high-pass filter configuration.
   
    * ACCEL_HPF | Filter Mode | Cut-off Frequency
    * ----------+-------------+------------------
    * 0         | Reset       | None
    * 1         | On          | 5Hz
    * 2         | On          | 2.5Hz
    * 3         | On          | 1.25Hz
    * 4         | On          | 0.63Hz
    * 7         | Hold        | None
    * </pre>
    */	
    
    //DEBUG_PRINTLN("Setting DHPF bandwidth to 5Hz...");
    accelgyro.setDHPFMode(1);

	
    // Get motion detection event acceleration threshold.
    	
    //Serial.println("Setting motion detection threshold to 2...");
    
    accelgyro.setMotionDetectionThreshold(10);


    // Get zero motion detection event acceleration threshold.
    //Serial.println("Setting zero-motion detection threshold to 156...");
    
    accelgyro.setZeroMotionDetectionThreshold(2);


    // Get motion detection event duration threshold.
 
    //Serial.println("Setting motion detection duration to 80...");
    accelgyro.setMotionDetectionDuration(80);

    // Get zero motion detection event duration threshold.
    //Serial.println("Setting zero-motion detection duration to 0...");
    
    accelgyro.setZeroMotionDetectionDuration(0);	
	

    // configure LED strip:
    delay( 3000 ); // power-up safety delay
    
  // It's important to set the color correction for your LED strip here,
  // so that colors can be more accurately rendered through the 'temperature' profiles
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( BRIGHTNESS );
    
}

void loop() {
    // read raw accel/gyro measurements from device
    //Serial.println("Getting raw accwl/gyro measurements");
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	
    zero_detect = accelgyro.getIntMotionStatus();
    threshold = accelgyro.getZeroMotionDetectionThreshold();
  
	
    // display tab-separated accel/gyro x/y/z values
    //(.) at the end to treat int as float
    Serial.print("a/g:\t");
    Serial.print(ax/16384.); Serial.print("\t");
    Serial.print(ay/16384.); Serial.print("\t");
    Serial.print(az/16384.); Serial.print("\t");
    Serial.print(gx/131.072); Serial.print("\t");
    Serial.print(gy/131.072); Serial.print("\t");
    Serial.println(gz/131.072);
	
    Serial.print("ZeroMotion(97):\t");	
    Serial.print(zero_detect); Serial.print("\t");
    Serial.println(accelgyro.getMotionDetectionThreshold());
    
   if(zero_detect == 1)
  {
    for(int n = 0; n < NUM_LEDS; n++) {
      leds[n] = CRGB::Green;
      FastLED.show();
    }
    delay(800);
   }
   else{
    for(int n = 0; n < NUM_LEDS; n++) {
      leds[n] = CRGB::Black;
      FastLED.show();
   }
 }
}
