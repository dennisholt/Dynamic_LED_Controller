// Lighting for Black Doodle by Jeanie Holt
// Bbackground rotating spectrum of colors random change direction
// Random start and location of brightness display
// Every 10 to 20 sec shift brightness of all LEDs using display pattern
// top right=0; Top right=22; right bottom=42; bottom left=65; left top=85

/////  Includes  /////
#include <FastLED.h>

/////  Defines  /////
#define LED_PIN    A0  
#define NUM_LEDS   86          //  led string length
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

// Used by main loop and functions
#define NUM_DISP 5  // number of displays at a time
#define BG_BRT 100   // background brightness
#define DIS_LENGTH 60   // display brightness array length
/////  Global Variables  /////
CRGB leds[NUM_LEDS];  // Array for LED color values
uint8_t bg_hue[NUM_LEDS];  // array to save background hue
uint8_t bg_brt[NUM_LEDS];  // array to save background brightness
uint8_t dis_inhibit[NUM_LEDS];  // inhibit starting display too close to another
// active display at this location = 1 otherwise = 0.
int bg_start = 0;  // location of background color start
int bg_direction = 1;   // increment value for shifting bg_start, 1=clockwise
uint8_t bright = BRIGHTNESS ;
uint8_t dis_loc[NUM_DISP];  // display location
uint8_t dis_start[NUM_DISP] = {10, 20, 30, 40, 50};  // countdown delay to start
 // index into display brightness array
uint8_t dis_i[NUM_DISP] = { DIS_LENGTH, DIS_LENGTH, DIS_LENGTH, DIS_LENGTH, DIS_LENGTH }; 
uint8_t dis_bright[2][DIS_LENGTH]={  // Brightness pattern of display 
//  1    2    3    4    5    6    7    8    9    10    // second pattern is not used
  { 35,  40,  45,  50,  55,  60,  65,  70,  80,  90, 
   100, 120, 140, 160, 180, 200, 225, 255, 255, 255,
   255, 200, 150, 100, 100, 150, 200, 255, 255, 255,
   255, 255, 255, 255, 200, 150, 100, 100, 150, 200,   
   255, 200, 150, 100, 100, 100, 100, 100, 150, 200,
   150, 100,  50,  40,  30,  20,  10,   5,   2,   0},
  { 35,  40,  45,  50,  55,  60,  65,  70,  80,  90, 
   100, 120, 140, 160, 180, 200, 225, 255, 255, 255,
   255, 200, 150, 100, 100, 100, 100, 100, 150, 200,
   255, 200, 150, 100, 100, 100, 100, 100, 150, 200,   
   255, 200, 150, 100, 100, 100, 100, 100, 150, 200,
   150, 100,  50,  40,  30,  20,  10,   5,   2,   0}};
   
// parameters used by tuning functions
CHSV temp;
bool inhibit = true; 
uint8_t state = 0, old_state = 0;
uint8_t knob = 0, old_knob = 0;
uint8_t hue = 0, sat = 200, val = 126;
uint8_t msg_delay = 0; 

/////  Functions  ///// 

void paint_background(){   
// set background hue array
// FastLED has the following predefined HUE.. variables
// HUE_RED=0;HUE_ORANGE=32;HUE_YELLOW=64;HUE_GREEN=96;HUE_AQUA=128;HUE_BLUE=160;
// HUE_PURPLE=192;HUE_PINK=224
  for(uint8_t i = 0; i < NUM_LEDS ; i++){
    bg_hue[i] = (i *3) % 256;
  }
}  

void dis(uint8_t n){
  // inactive condition: dis_start != 0; dis_i == DIS_LENGTH
  if(dis_start[n] != 0){  // if display[n] is not active decrement start & exit
    dis_start[n]--;
    return;
  }
  // display is active 
  if(dis_i[n] == DIS_LENGTH ){   // start of new setup display
    // set location not inhibited
    bool test = true;
    while(test){
      dis_loc[n] = random8(NUM_LEDS - 1);
      test = dis_inhibit[dis_loc[n]];
      for(int i= -4; i < 9; i++){
        test = (test | dis_inhibit[(dis_loc[n] + NUM_LEDS + i) % NUM_LEDS]);
      }  
    } 
    dis_inhibit[dis_loc[n]] = 1; // set inhibit
    dis_i[n] = 0;
  }
  // set display brightness
  uint8_t loc[7];
  for(int i = 0; i < 7; i++){
    loc[i] = (dis_loc[n] + NUM_LEDS +i) % NUM_LEDS;
  }
  bg_brt[loc[3]] =  dis_bright[1][dis_i[n]];
  bg_brt[loc[2]] = bg_brt[loc[4]] = scale8(bg_brt[loc[3]], 210); // dim to 82%
  bg_brt[loc[1]] = bg_brt[loc[5]] = scale8(bg_brt[loc[2]], 210);
  bg_brt[loc[0]] = bg_brt[loc[6]] = scale8(bg_brt[loc[1]], 210);  
  dis_i[n]++;
  if(dis_i[n] == DIS_LENGTH ){   // at end of display setup inactive condition
    dis_inhibit[dis_loc[n]] = 0; // clear inhibit
    dis_start[n] = random8(30, 200);// set random dis_start[n] between 10 and 100 
    return;
  }
}

//  global parameters for dis_all()
uint16_t dis_all_start = 200;
uint8_t dis_all_i = DIS_LENGTH;
void dis_all(){  // much less frequent but effects all LEDs
  // inactive condition: dis_start != 0; dis_i == DIS_LENGTH
  if(dis_all_start != 0){  // if display[n] is not active decrement start & exit
    dis_all_start--;
    return;
  }
  // display is active 
  if(dis_all_i == DIS_LENGTH ){   // start of new setup display
    dis_all_i = 0;
  }
  // set display brightness
  for(int i = 0; i < NUM_LEDS; i++){
    bg_brt[i] =  dis_bright[1][dis_all_i];
  }  
  dis_all_i++;
  if(dis_all_i == DIS_LENGTH ){   // at end of display setup inactive condition
    dis_all_start = random16(250, 500);// set random dis_start[n] between 10 and 100 
    return;
  }
}

void cpy_show(){  // Display the brighter [background or background at wave brightness]
//  Serial.println("cpy_show()" );
  for(int i = 0; i < NUM_LEDS; i++){
    // get hue get brigntness set led
    uint8_t temp = BG_BRT ;
    if(bg_brt[i] > BG_BRT) temp = bg_brt[i];
    leds[i] = CHSV(bg_hue[(bg_start + i) % NUM_LEDS], 255, temp); 
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
//    fill_gradient_RGB(leds, START_TUNE, temp, END_TUNE, temp);
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
  random16_add_entropy (10000);  // set random seed for random number generator
  paint_background();  // setup array of background hue spectrum
  for(uint8_t i=0; i< NUM_LEDS; i++){dis_inhibit[i] = 0;} // allow display in any location
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
//  bg_move();  no need for function as called with each main loop
// random dirction change
  uint8_t s =  random8(255);
  if (s < 10) bg_direction *= -1;  // probability 10/255 reverse direction
  bg_start = (bg_start + bg_direction + NUM_LEDS) % NUM_LEDS; // shift background hue starting point  
  
  for(uint8_t i; i < 5; i++){
    dis(i);  // call display n
  }
  dis_all();
  
  cpy_show();  // put it all together and display
  FastLED.delay( LOOP_DELAY );
//  while ( true );     // stop here     
}   //  end of main loop

