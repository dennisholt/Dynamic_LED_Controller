// Lighting for RainForest by Juli-Ann Gasper
// Set background colors and brightness
// Have brightness variation (flicker) at fixed locations; 
// random delay to each flicker start; each flicker probability set with min and max random delay.
// Capability to use tuning circuit board input
// code clean-up 10/25/2017
// Direction of LED string is different for this piece: starts in upper right inc anti-clockwise
// top right=0; Top left=25; left bottom=60; bottom right=73; right top=108

/////  Includes  /////
#include <FastLED.h>

/////  Defines  /////
#define LED_PIN    A0  
#define NUM_LEDS  109          //  led string length
#define LOOP_DELAY 150          // Miliseconds delay at end of main loop 
#define NUM_FLICKS 5            // Number of flickers
#define FLICKER_WIDTH 3         // Number of LEDs effected by a flicker
        // We later found the 5th flicker needed to be spread over a large number of LEDs
        // so we coded an exception treatment into the sketch for that flicker

// used for setup
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define CORRECTION  TypicalLEDStrip
#define BRIGHTNESS 255        // dims brightness of entire LED string (255 = no dimming)
#define DEBUG_DELAY(ms) FastLED.delay(ms)  // can substitute which delay function is used
   // builtin Arduino function is delay(ms), FastLED has FastLED.delay(ms) which enables temporal dithering

// Used for optional tuning functions
#define TUNING 0  // 1 to enable development button & knob, 0 to disable 
//  Arduino doesn't work well when sketch references pins that are not connected
//  This sketch will exclude sections of code using #if TUNING ... #endif 
//  when #define TUNING 0 indicates tuning board in not connected or not in use. 
// set the following two lines to select the range of LEDs to be adjusted using the tuning board
#define START_TUNE 61          // '0' will indicate the begining of the string of LEDs
#define END_TUNE   73          // 'NUM_LEDS-1' will indicate the end of the string 
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
// parameters for flickers
uint8_t flicker_width= FLICKER_WIDTH;
uint8_t width_factor[FLICKER_WIDTH] = {120,255,132};  // brightness adjustment across flicker width
uint8_t flicker_location[NUM_FLICKS]={3,7,11,24,99};
bool flicker_running[NUM_FLICKS] = {false,false,false,false,false};
uint8_t flicker_counter[NUM_FLICKS] = {0,0,0,0,0};
uint8_t flick_delay_min[NUM_FLICKS] = {50,60,50,60,50};      // These will determine flicker frequency and time between
uint8_t flick_delay_max[NUM_FLICKS] = {150,200,200,250,200};
// The array flicker_b2 is the brightness series that each flicker will go through. 
uint8_t flicker_b2[40]={4,8,16,32,64,128,128,192,192,128,64,64,64,128,128,128,192,192,128,128,
                         64,32,32,64,128,128,255,255,128,128,64,64,32,32,16,8,4,2,0,0};   
// each flicker could have a different color. We kept them the same.
CHSV flicker_color[NUM_FLICKS]= {CHSV(50,161,255),CHSV(50,161,255),CHSV(50,161,255),CHSV(50,161,255),CHSV(50,161,255)};
CRGB bg[NUM_FLICKS][FLICKER_WIDTH]; // place to save brackground colors

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

// We will need to reset background after a flicker so get and save background 
void get_background(uint8_t width){    // width = max flicker width
  // call once after background is set
  // will save background to global variable bg
  for (int i = 0; i < NUM_FLICKS; i++){
    for (int k = 0; k < width; k++){    
      bg[i][k] = leds[flicker_location[i]+k];
      }
    }  
  }

// function to keep track of delay before flicker and output flicker if active
void drawFlicker(int i){
  if(flicker_running[i] == false){  // if not active decrement delay
    if(flicker_counter[i] <= 0){    // if delay end make actve
      flicker_counter[i] = 0;
      flicker_running[i] = true;  
      return;
      }
    flicker_counter[i]--; 
    return;                 
    }else{                          // if active up-date values and increment counter
    uint8_t f_bright = flicker_b2[flicker_counter[i]];
    CRGB temp = flicker_color[i]; 
    temp.nscale8(f_bright);
    for(uint8_t k = 0; k < FLICKER_WIDTH; k++){
      CRGB temp2 = temp.nscale8(width_factor[k]);
      temp2 += bg[i][k];
      leds[flicker_location[i]+k]= temp2;
      }
    if(i == 4){  // special handling for 5th flicker 
      fill_gradient_RGB(leds, 80, CHSV(83,161,46), 99, temp.nscale8(width_factor[0]));
      }
    flicker_counter[i]++;  
    if(flicker_counter[i] > 39){   // if end of pattern make not active, set delay, and restore background
      flicker_running[i] = false;
      flicker_counter[i] = random8(flick_delay_min[i],flick_delay_max[i]);
      for(uint8_t k = 0; k < FLICKER_WIDTH; k++){  // restore background 
        leds[flicker_location[i]+k]= bg[i][k];  
        }
      if(i == 4){   //  restore background for 5th flicker 
        fill_gradient_RGB(leds, 80, CHSV(83,161,46), 108, CHSV(61,161,50));  // same as setting in setup()
        }
      }           
    } // end if active
  return;
  }

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
  fill_gradient_RGB(leds, 0, CHSV(61,161,50), 25, CHSV(61,161,50));
  fill_gradient_RGB(leds, 26, CHSV(61,161,50), 60, CHSV(88,161,30));
  fill_gradient_RGB(leds, 61, CHSV(88,161,30), 73, CHSV(88,161,45));
  fill_gradient_RGB(leds, 74, CHSV(88,161,45), 80, CHSV(83,161,46));
  fill_gradient_RGB(leds, 80, CHSV(83,161,46), 108, CHSV(61,161,50));

  get_background(flicker_width);  // Save background
  for(uint8_t i=0; i < NUM_FLICKS ; i++){  // load flicker counter with initial delays
    flicker_counter[i] = random8(flick_delay_min[i],flick_delay_max[i]);
    }
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

  for(uint8_t i = 0; i < NUM_FLICKS ; i++ ){
    drawFlicker(i);
    }        
  FastLED.delay(LOOP_DELAY);
}   //  end of main loop

