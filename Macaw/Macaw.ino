// Lighting for Macaw by Dana Newman
// Adapted from theaterChaseRainbow() function 
// in Adafruit_NeoPixel/examples/Strandtest.ino (downloaded 7/29/2015)
// Have brightness constant while color shifts around the color wheel
// Capability to use tuning circuit board input
//    Press Button to change modes
//    Mode=1 is brightness adjustment; Mode=2 is color wheel position adjustment
//    Turn potentiometer knob to adjust value
// Code clean-up 10/27/2017
// top right=43; right bottom=72; bottom left=116; left top=145

/////  Includes  /////
#include <Adafruit_NeoPixel.h>

/////  Defines  /////
#define LED_PIN      A0   
#define NUM_LEDS    146   //  led string length
#define LOOP_DELAY  100   // Miliseconds delay between each color point
#define BRIGHTNESS 255    // dims brightness of entire LED string (255 = no dimming)

// Used for optional tuning functions
#define TUNING 0  // 1 to enable development button & knob, 0 to disable 
//  Arduino doesn't work well when sketch references pins that are not connected
//  This sketch will exclude sections of code using #if TUNING ... #endif 
//  when #define TUNING 0 indicates tuning board in not connected or not in use. 
//  Tuning board will effect all LEDs if enabled 
#define KNOB_PIN    A3   
#define BTN_PIN     A2 
#define DBUG_LED    A1 
#define STATES 2
#define INHIBIT_LEVEL 10      // min knob change to activate tuning
#define MSG_DELAY   5     // number of loops between repeating state blink
#define DEBUG_BLINK 100   // ms on and ms off per blink  (50 = 100ms per blink)
#define DEBUG_PAUSE 200   // ms between pulse sequences  (at 200, 3 50+50ms blinks take 1/2 second, 8 blinks = 1sec including pause)

/////  Global Variables  /////
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
uint8_t ColorWheelPos=0;
// parameters used by tuning functions
bool inhibit = true; 
uint8_t state = 0, old_state = 0;
uint8_t knob = 126, old_knob = 126;
uint8_t bright = BRIGHTNESS;
uint8_t msg_delay = 0; 

/////  Functions  ///// 
#if TUNING 
void _btnHandler() {
  // come here when button is pressed
  // Increment state when button is released
  // btn down=0; up=1;
  // This little section is a debounce routine 
  while (!digitalRead(BTN_PIN)) {   // Wait for button to be released
    delay(10);
    }
  int released = 0;
  while (released < 4) {   // wait until button is released for 4 consecutive delays
    if (!digitalRead(BTN_PIN)) {  
      released = 0;
      } else {
      released++;
      }
    delay(1);
    }
  // Action to be taken when button is released
  inhibit = true;
  state = (state + 1) % STATES;  // increment and if at highest state cycle back to zero
  Serial.print(" ColorWheelPos= ");
  Serial.print(ColorWheelPos);
  Serial.print(" Brightness= ");
  Serial.print(bright);
  Serial.println();
  }

// function to display the current state by blinking the LED on the tuning board
void _debugBlink(int n) {
  for ( int i = 0 ; i < n ; i++ ) {
    digitalWrite(DBUG_LED, HIGH);
    delay(DEBUG_BLINK);
    digitalWrite(DBUG_LED, LOW);
    delay(DEBUG_BLINK);
    }
  delay(DEBUG_PAUSE);
  } 
#endif  // TUNING 

//  lights with shifting throught the rainbow effect
void paintRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
#if TUNING 
//  Let button and knob set the values of bright and ColorWeelPos
#else
    bright = BRIGHTNESS;
    ColorWheelPos = j % 255;  // for color shifting along LED string use (i+j) % 255;
#endif  // TUNING 
    uint32_t color = Wheel(ColorWheelPos);
    for (int i=0; i < strip.numPixels(); i++) { // set all LEDs to this color
      strip.setPixelColor(i, color);    
      }
    strip.show();
    delay(wait);
#if TUNING  
    old_state = state;
    old_knob = knob;    
    if(!digitalRead(BTN_PIN)){  // Check for button press and print current color
      _btnHandler();            // only checked after delay(wait) for button to be detected
      }                         // needs to be held down longer that 'wait' = LOOP_DELAY
// blink state every MSG_DELAY loops
    msg_delay++;
    msg_delay = msg_delay % MSG_DELAY;  // cycle from maximum count back to zero
    if(msg_delay == 0){
      _debugBlink(state + 1);   
      }
    if (state != old_state){ // Print out what state we are in
      switch(state){
        case 0:
          Serial.print(" Brightness Value Mode ");
          break;
        case 1:
          Serial.print(" Color Wheel Value Mode ");
          break;
        }  
      Serial.println();
      _debugBlink(state + 1);  // show current state by blinking LED on tuning board
      }   // end if state change  
  
// don't change anything unless the knob is moved significantly
    knob = analogRead(KNOB_PIN) / 4;
// first move after state change require larger move
    if(inhibit){
      if(abs(old_knob - knob) >= INHIBIT_LEVEL){
        inhibit = false;  
        }else{
          knob = old_knob;
          }
      }  // end if inhibit
    if (knob != old_knob){ 
      switch(state){
        case 0:
          bright = knob;
          break;
        case 1:
          ColorWheelPos = knob;
          break;
        }  // end switch-case 
    // if new setting display and print it out 
      Serial.print(" ColorWheelPos= ");
      Serial.print(ColorWheelPos);
      Serial.print(" Brightness= ");
      Serial.print(bright);
      Serial.println();
      }  // end if knob change
#endif  // TUNING      
    }
  }  // end paintRainbow()

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  int red=0, green=0, blue=0;
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    red = (255 - WheelPos * 3) * bright/255.0 + 0.5;
    blue = (WheelPos * 3) * bright/255.0 + 0.5;
    return strip.Color(red, green, blue);
    }
  if(WheelPos < 170) {
    WheelPos -= 85;
    green = (WheelPos * 3) * bright/255.0 + 0.5;
    blue = (255 - WheelPos * 3) * bright/255.0 + 0.5;
    return strip.Color(red, green, blue);
    }
  WheelPos -= 170;
  red = (WheelPos * 3) * bright/255.0 + 0.5;
  green = (255 - WheelPos * 3) * bright/255.0 + 0.5;
  return strip.Color(red, green, blue);
  }

/////  Setup ///// 
void setup() {
  Serial.begin(9600);
  delay( 1000 ); // power-up safety delay
  
#if TUNING  //  if tuning board is used set-up IO pins and initialize parameters
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DBUG_LED, OUTPUT);
  digitalWrite(DBUG_LED, LOW);
  old_knob = knob = analogRead(KNOB_PIN) / 4;
  inhibit = true;
  Serial.print(" Brightness Value Mode ");  // starting state
  Serial.println();   
#endif  // TUNING    
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

/////  Main Loop  ///// 
void loop() {  
  paintRainbow(LOOP_DELAY);
}

