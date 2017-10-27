// Lighting for test light frame 
// Set background colors and brightness
// Uses tuning circuit board input to adjust lighting in whole frame on specific segments
// When lighting valus are chosen they can be set as background in setup() 
// Then the tuning circuit can be used to explore lighting in outher segments
// code clean-up 10/27/2017
// top left=0; Top right=25; right bottom=51; bottom left=77; left top=103

/////  Includes  /////
#include <FastLED.h>

/////  Defines  /////
#define LED_PIN    A0  
#define NUM_LEDS  104          //  led string length
#define LOOP_DELAY 150          // Miliseconds delay at end of main loop 

// used for setup
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define CORRECTION  TypicalLEDStrip
#define BRIGHTNESS 255        // dims brightness of entire LED string (255 = no dimming)
#define DEBUG_DELAY(ms) FastLED.delay(ms)  // can substitute which delay function is used
   // builtin Arduino function is delay(ms), FastLED has FastLED.delay(ms) which enables temporal dithering

// Used for optional tuning functions
#define TUNING 1  // 1 to enable development button & knob, 0 to disable 
//  Arduino doesn't work well when sketch references pins that are not connected
//  This sketch will exclude sections of code using #if TUNING ... #endif 
//  when #define TUNING 0 indicates tuning board in not connected or not in use. 
// set the following two lines to select the range of LEDs to be adjusted using the tuning board
#define START_TUNE 0              // '0' will indicate the begining of the string of LEDs
#define END_TUNE   NUM_LEDS-1     // 'NUM_LEDS-1' will indicate the end of the string 
// Note don't set these LEDs elswhere in the sketch or you won't see effect of tuning board
#define KNOB_PIN   A3 
#define BTN_PIN    A2 
#define DBUG_LED   A1   
#define STATES 4
#define INHIBIT_LEVEL 10      // min knob change to activate tuning
#define MSG_DELAY 5           // number of loops between repeating state blink
#define DEBUG_BLINK 100       // ms on and ms off per blink  (50 = 100ms per blink)
#define DEBUG_PAUSE 200       // ms between pulse sequences  (at 200, 3 50+50ms blinks take 1/2 second, 8 blinks = 1sec including pause)

/////  Global Variables  /////
CRGB leds[NUM_LEDS];  // Array for LED color values
CHSV now;
// parameters used by tuning functions
bool inhibit = true; 
uint8_t state = 0, old_state = 0;
uint8_t knob = 0, old_knob = 0;
uint8_t bright = BRIGHTNESS, hue = 0, sat = 200, val = 126;
uint8_t msg_delay = 0; 

/////  Functions  ///// 
#if TUNING    // Exclude tuning functions if tuning board is not in use
void _btnHandler() {
  // come here when button is pressed
  // Increment state when button is released
  // btn down=0; up=1;
  // This little section is a debounce routine 
  while (!digitalRead(BTN_PIN)) {   // Wait for button to be released
    FastLED.delay(10);
    }
  int released = 0;
  while (released < 4) {   // wait until button is released for 4 consecutive delays
    if (!digitalRead(BTN_PIN)) {
      released = 0;
      } else {
      released++;
      }
    FastLED.delay(1);
    }
  // Action to be taken when button is released
  inhibit = true;
  state = (state + 1) % STATES;  // increment and if at highest state cycle back to zero
  }

// function to display the current state by blinking the LED on the tuning board
void _debugBlink(int n) {
  for ( int i = 0 ; i < n ; i++ ) {
    digitalWrite(DBUG_LED, HIGH);
    DEBUG_DELAY(DEBUG_BLINK);
    digitalWrite(DBUG_LED, LOW);
    DEBUG_DELAY(DEBUG_BLINK);
    }
  DEBUG_DELAY(DEBUG_PAUSE);
  } 
#endif

/////  Setup /////
void setup() {
  delay( 1000 ); // power-up safety delay
  Serial.begin(9600); 
   
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( CORRECTION );
  FastLED.setBrightness( bright );  // can set overall brightness dimming amount; 
// overall brightness adjustment is an available tuning function when using tuning board   
// temporal dithering should make transitions smoother; however I haven't noticed much effect
  FastLED.setDither( 1 );  //  0 = disable temporal dithering
// set background leds
// FastLED has the following predefined HUE.. variables
// HUE_RED=0;HUE_ORANGE=32;HUE_YELLOW=64;HUE_GREEN=96;HUE_AQUA=128;HUE_BLUE=160;HUE_PURPLE=192;HUE_PINK=224
// These are examples of how to set some background values. They get changed when there is input from tuning circuit 
  fill_gradient_RGB(leds, 0, CHSV(HUE_RED,200,100), 25, CHSV(HUE_GREEN,200,100));
  fill_gradient_RGB(leds, 26, CHSV(HUE_GREEN,200,100), 60, CHSV(HUE_BLUE,200,100));
  fill_gradient_RGB(leds, 61, CHSV(HUE_BLUE,200,100), 73, CHSV(HUE_RED,200,100));
  fill_gradient_RGB(leds, 74, CHSV(80,200,100), 80, CHSV(100,200,100));
  fill_gradient_RGB(leds, 80, CHSV(100,200,100), 103, CHSV(61,200,1));

  FastLED.show();  // display background

#if TUNING //  if tuning board is used set-up IO pins and initialize parameters
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DBUG_LED, OUTPUT);
  digitalWrite(DBUG_LED, LOW);  
  old_knob = knob = analogRead(KNOB_PIN) / 4;
  inhibit = true;
  Serial.print(" HUE  HUE  HUE ");  // starting state
  Serial.println(); 
#endif
  }

/////  Main Loop  ///// 
void loop() {
#if TUNING  
  old_state = state;
  old_knob = knob;
  if(!digitalRead(BTN_PIN)){  // Check for button press
    _btnHandler();
    }
  if (state != old_state){ // Print out what state we are in
    switch(state){
      case 0:
        Serial.print(" HUE  HUE  HUE ");
        break;
      case 1:
        Serial.print(" SAT  SAT  SAT ");
        break;
      case 2:
        Serial.print(" VAL  VAL  VAL ");
        break;
      case 3:
        Serial.print(" Brightness  Brightness  Brightness ");
        break;
      }  
    Serial.println();
    _debugBlink(state + 1);  // show current state by blinking LED on tuning board
    }   // end if state change  
  
// blink state every MSG_DELAY loops
  msg_delay++;
  msg_delay = msg_delay % MSG_DELAY;  // cycle from maximum count back to zero
  if(msg_delay == 0){
    _debugBlink(state + 1);   
    }
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
        hue = knob;
        break;
      case 1:
        sat = knob;
        break;
      case 2:
        val = knob;
        break;
      case 3:
        bright = knob;
        break;
      }  // end switch-case 
    // if new setting display and print it out 
    FastLED.setBrightness(bright);  
    now = CHSV(hue, sat, val);
    fill_gradient_RGB(leds, START_TUNE, now, END_TUNE, now);
    FastLED.show(); 
    Serial.print("   HSV= ");
    Serial.print(now.hue);
    Serial.print(" ");
    Serial.print(now.sat);
    Serial.print(" ");
    Serial.print(now.val);      
    Serial.print("; RGB= ");
    Serial.print(leds[START_TUNE][0]);
    Serial.print(" ");
    Serial.print(leds[START_TUNE][1]);
    Serial.print(" ");
    Serial.print(leds[START_TUNE][2]);
    Serial.print("; Brightness= ");
    Serial.print(bright);
    Serial.println();
    }  // end if knob change
#endif
       
  FastLED.delay(LOOP_DELAY);
}   //  end of main loop

