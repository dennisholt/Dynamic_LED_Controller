// Lighting for Copper Head by Jeanie Holt
// Set background colors and brightness
// Three waves of diminishing brightness from upper right to lower left
// Dip in brightness. At bottom of dip transition to solid hue background and back.
// top right=0; Top right=26; right bottom=50; bottom left=77; left top=101

/////  Includes  /////
#include <FastLED.h>

/////  Defines  /////
#define LED_PIN    A0  
#define NUM_LEDS  102          //  led string length
#define LOOP_DELAY 40          // Miliseconds delay at end of main loop 

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
CHSV bg[NUM_LEDS];     // array to save background 
uint8_t wave[NUM_LEDS];  // array to save brightness value of wave
// background hue at LED { 0, 13, 14, 50,  51,  77,  78,  90, 91, 101 }
uint8_t bg_points[10] = { 53, 26, 27, 74, 125, 175, 176, 184, 74,  54 };
uint8_t bright = BRIGHTNESS ;
uint8_t i = 0;  // wave increment ( which wave are we on? )

// parameters used by tuning functions
CHSV temp;
bool inhibit = true; 
uint8_t state = 0, old_state = 0;
uint8_t knob = 0, old_knob = 0;
uint8_t hue = 0, sat = 200, val = 126;
uint8_t msg_delay = 0; 

/////  Functions  ///// 

void paint_background(uint8_t points[], uint8_t brt){   
// set background leds
// FastLED has the following predefined HUE.. variables
// HUE_RED=0;HUE_ORANGE=32;HUE_YELLOW=64;HUE_GREEN=96;HUE_AQUA=128;HUE_BLUE=160;
// HUE_PURPLE=192;HUE_PINK=224
  fill_gradient(bg, 0, CHSV(points[0],255,brt), 13, CHSV(points[1],255,brt));
  fill_gradient(bg, 14, CHSV(points[2],255,brt), 50, CHSV(points[3],255,brt));
  fill_gradient(bg, 51, CHSV(points[4],255,brt), 77, CHSV(points[5],255,brt));
  fill_gradient(bg, 78, CHSV(points[6],255,brt), 90, CHSV(points[7],255,brt));
  fill_gradient(bg, 91, CHSV(points[8],255,brt), NUM_LEDS-1, CHSV(points[9],255,brt)); 
}

void cpy_show(){  // Display the brighter [background or background at wave brightness]
  for(int i = 0; i < NUM_LEDS; i++){
    if(wave[i] > bg[i].val){
      temp = bg[i];
      temp.val = wave[i];
      leds[i] = temp;
    } else {
      leds[i] = bg[i];
    } 
  }
  FastLED.show();

#if TUNING  
// Needs to be in frequent executed part of program
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
    temp = CHSV(hue, sat, val);
    fill_gradient_RGB(leds, START_TUNE, temp, END_TUNE, temp);
    FastLED.show(); 
    Serial.print("   HSV= ");
    Serial.print(temp.hue);
    Serial.print(" ");
    Serial.print(temp.sat);
    Serial.print(" ");
    Serial.print(temp.val);      
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
  
}  // end cpy_show()

void shift_wave(){  //  Shift the wave brightness from upper right to lower left
  for(int i = 77; i > 27; i--){
    wave[i] = wave [i-1];
  }
  for(int i = 78; i < 101; i++){
    wave[i] = wave [i+1];
  }
  wave[101] = wave[0];
  for(int i = 0; i < 26; i++){
    wave[i] = wave [i+1];
  }
}
 
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
  FastLED.setBrightness( BRIGHTNESS );  // can set overall brightness dimming amount; 
// overall brightness adjustment is an available tuning function when using tuning board   
// temporal dithering should make transitions smoother; however I haven't noticed much effect
  FastLED.setDither( 1 );  //  0 = disable temporal dithering
  fill_solid(leds,NUM_LEDS, CRGB(0,0,0));  // set all to black for one second
  FastLED.show(); 
  delay(1000);

#if TUNING //  if tuning board is used set-up IO pins and initialize parameters
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DBUG_LED, OUTPUT);
  digitalWrite(DBUG_LED, LOW);  
  old_knob = knob = analogRead(KNOB_PIN) / 4;
  inhibit = true;
  Serial.print(" HUE  HUE  HUE ");  // starting state
  Serial.println(); 
#endif
}  // end setup

/////  Main Loop  ///// 
void loop() {

  
  paint_background(bg_points, 80);  // set background
  uint8_t target[10];
  uint8_t temp_pt[10];
  if(i == 0){  // before first wave dip brightness, slide to solid color, and back
    for(int t = 0; t < 256 ; t++ ){   // loop to increment through dimming dip
      shift_wave();
      wave[26] = wave[27] =  80; // wave becomes steady original brightness
      uint8_t tp = t;  // calculate brightness dip
      uint8_t f1 = scale8( 255 - cos8 ( tp ), 128); 
      FastLED.setBrightness( 255- f1);
      if(t == 128){  // at center of dip transition to solid background         
        for(uint8_t j=0; j<10; j++){  // set target and start of transition
          target[j] = HUE_BLUE ;
          temp_pt[j] = bg_points[j];
        }
        for(uint8_t j=0; j<70; j++){  // transition to new target
          for(uint8_t k=0; k<10; k++){  // increment temp toward target
            if(temp_pt[k] < target[k]) temp_pt[k] += 2;
            if(temp_pt[k] > target[k]) temp_pt[k] -= 2;
          } 
          paint_background(temp_pt, 80); // display temp background
          cpy_show();
          delay( LOOP_DELAY * 2 );
        }
        delay(1000);   //time at solid background
        for(uint8_t j=0; j<10; j++){  // transition back to original background
          target[j] = bg_points[j] ;
          temp_pt[j] = HUE_BLUE ;
        }
        for(uint8_t j=0; j<70; j++){  // transition to new target
          for(uint8_t k=0; k<10; k++){  // increment temp toward target
            if(temp_pt[k] < target[k]) temp_pt[k] += 2;
            if(temp_pt[k] > target[k]) temp_pt[k] -= 2;
          } 
          paint_background(temp_pt, 80); // display temp background
          cpy_show();
          delay( LOOP_DELAY * 2 );
        }
        paint_background(bg_points, 80); // make sure background is set back to original
      } // end if middle of dip  
      cpy_show();
      delay( LOOP_DELAY );         
    } // end for t
    FastLED.setBrightness( 255);
  } // end if i == 0 

  uint8_t delay_adj = 1;
  uint8_t scl[3] = {255, 126, 64};  // scale height of successive waves
  if(i == 0){    //  First wave (brightest) moves slower
    delay_adj = 2;
  }else{
    delay_adj = 1;
  }
  for(int t = 0; t < 256 ; t+= 2 ){   // loop to increment through wave
    shift_wave();
    uint8_t tp = t;
    uint8_t wav_brt1 = 255 - cos8 ( tp ); 
    uint8_t wav_brt = scale8 (wav_brt1, scl[i]);  // 128 = 1/2;  64 = 1/4;
    wave[26] = wave[27] = scale8(wav_brt, 175) + 80; // insert next value
    if (t==254) i = (i + 1) % 3; // at end of wave increment to next wave
    cpy_show();                  // combine background and wave then show
    delay( LOOP_DELAY * delay_adj );         
  } // end for t  
//  while ( true );     // stop here     
}   //  end of main loop

