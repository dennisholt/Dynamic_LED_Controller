// RainForest fabric art by Juli; Set background colors and brightness
// Have brightness variation (flicker) at fixed locations; 
// random delay to each flicker start; each flicker probability set with min and max random delay.
// Use knob to adjust flicker HSV or Brightness parameter 
#include <FastLED.h>

/////  Defines  /////
#define KNOB_PIN   A3 
#define BTN_PIN    A2 
#define DBUG_LED   A1 
#define LED_PIN    A0         // A0 on rainforest; A3 on frame
#define ONBOARD_LED_PIN    13 
#define STATES 4
#define NUM_LEDS 109          //  led string length
#define INHIBIT_LEVEL 10      // min knob change to activate tuning
// Tuning range start and end led number
#define START_TUNE 0          // 0
#define END_TUNE NUM_LEDS-1   // NUM_LEDS-1
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define CORRECTION  TypicalLEDStrip
#define BRIGHTNESS 255        //  low value brightness if used (switches to 255)
#define DEBUG_BLINK 100       // ms on and ms off per blink  (50 = 100ms per blink)
#define DEBUG_PAUSE 200       // ms between pulse sequences  (at 200, 3 50+50ms blinks take 1/2 second, 8 blinks = 1sec including pause)
#define DEBUG_DELAY(ms) FastLED.delay(ms)  // Normally just delay(ms), change if you need a special delay function like FastLED.delay(ms)
#define MSG_DELAY 5           // number of loops between repeating state blink
#define TUNING 0  // 1 to enable tuning button & knob (slower), 0 to disable for normal running (faster)

//  Global Variables
CRGB leds[NUM_LEDS];  // Array for LED color values
CHSV now;
bool inhibit = true; 
uint8_t state = 0, old_state = 0;
uint8_t knob = 0, old_knob = 0;
uint8_t bright = BRIGHTNESS, hue = 0, sat = 200, val = 126;
uint8_t msg_delay = 0; 
uint8_t loop_count = 0;
// parameters for flickers
#define NUM_FLICKS 5 
#define FLICKER_WIDTH 3
uint8_t flicker_width= FLICKER_WIDTH;
uint8_t width_factor[FLICKER_WIDTH] = {120,255,132};
uint8_t flicker_location[NUM_FLICKS]={3,7,11,24,99};
bool flicker_running[NUM_FLICKS] = {false,false,false,false,false};
uint8_t flicker_counter[NUM_FLICKS] = {0,0,0,0,0};
uint8_t flick_delay_min[NUM_FLICKS] = {50,60,50,60,50};
uint8_t flick_delay_max[NUM_FLICKS] = {150,200,200,250,200};
uint8_t flicker_b2[40]={4,8,16,32,64,128,128,192,192,128,64,64,64,128,128,128,192,192,128,128,
                         64,32,32,64,128,128,255,255,128,128,64,64,32,32,16,8,4,2,0,0};
CHSV flicker_color[NUM_FLICKS]= {CHSV(50,161,255),CHSV(50,161,255),CHSV(50,161,255),CHSV(50,161,255),CHSV(50,161,255)};
CRGB bg[NUM_FLICKS][FLICKER_WIDTH]; // place to save brackground colors

#if TUNING
void _btnHandler() {
  // come here when button is pressed
  // Increment state when button is released
  // btn down=0; up=1;
  while (!digitalRead(BTN_PIN)) {   // Wait for button to be released
     FastLED.delay(10);
  }
  int released = 0;
  while (released < 4) {
    if (!digitalRead(BTN_PIN)) {
      released = 0;
    } else {
      released++;
    }
    FastLED.delay(1);
  }
  // Action to be taken when button is released
  inhibit = true;
  state = (state + 1) % STATES;
}

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

void get_background(uint8_t width){    // width = max flicker width
  // call once after background is set
  // will save background to global variable bg
  for (int i = 0; i < NUM_FLICKS; i++){
    for (int k = 0; k < width; k++){    
       bg[i][k] = leds[flicker_location[i]+k];
/*       Serial.print ("i= "); Serial.print (i);Serial.print (" k= "); Serial.print (k);   
       Serial.print (" RGB= "); Serial.print (bg[i][k][0]); Serial.print ("  "); 
       Serial.print (bg[i][k][1]); Serial.print ("  "); Serial.print (bg[i][k][2]);
       Serial.println (); */
    }
  }  
}
void setup() {
  Serial.begin(9600);
  delay( 1000 ); // power-up safety delay
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DBUG_LED, OUTPUT);
  digitalWrite(DBUG_LED, LOW);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( CORRECTION );
  FastLED.setBrightness( bright );
  FastLED.setDither( 1 );  //  0 = disable temporal dithering
  old_knob = knob = analogRead(KNOB_PIN) / 4;
  inhibit = true;
  Serial.print(" HUE  HUE  HUE ");  // starting state
  Serial.println(); 
// set background leds
// HUE_RED=0;HUE_ORANGE=32;HUE_YELLOW=64;HUE_GREEN=96;HUE_AQUA=128;HUE_BLUE=160;HUE_PURPLE=192;HUE_PINK=224
   fill_gradient_RGB(leds, 0, CHSV(61,161,50), 25, CHSV(61,161,50));
   fill_gradient_RGB(leds, 26, CHSV(61,161,50), 60, CHSV(88,161,30));
   fill_gradient_RGB(leds, 61, CHSV(88,161,30), 73, CHSV(88,161,45));
   fill_gradient_RGB(leds, 74, CHSV(88,161,45), 80, CHSV(83,161,46));
   fill_gradient_RGB(leds, 80, CHSV(83,161,46), 108, CHSV(61,161,50));
//   fill_gradient_RGB(leds, 74, CHSV(HUE_BLUE,255,50), 85, CHSV(HUE_BLUE,255,50));
//   fill_gradient_RGB(leds, 86, CHSV(HUE_PURPLE,255,50), 108, CHSV(HUE_PURPLE,255,50));
//   fill_gradient_RGB(leds, 0, CHSV(HUE_RED,255,100), 25, CHSV(HUE_RED,255,100));
//   fill_gradient_RGB(leds, 4, CRGB(0,0,50), 4, CRGB(0,0,50));
//   fill_gradient_RGB(leds, 0, CRGB(,,), NUM_LEDS - 1, CRGB(,,));
//   fill_gradient_RGB(leds, 0, CRGB(,,), NUM_LEDS - 1, CRGB(,,));
   get_background(flicker_width);
   for(uint8_t i=0; i < NUM_FLICKS ; i++){  // load flicker counter with initial delay
    flicker_counter[i] = random8(flick_delay_min[i],flick_delay_max[i]);
   }
   FastLED.show();
}

void loop() {

#if TUNING  
  old_state = state;
  old_knob = knob;

  if(!digitalRead(BTN_PIN)){  // Check for button press
    _btnHandler();
  }

  if (state != old_state){ // Report what state we are in
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
    _debugBlink(state + 1);
  }   // end if state change  
  
// blink state every MSG_DELAY loops
  msg_delay++;
  msg_delay = msg_delay % MSG_DELAY;
  if(msg_delay == 0){
    _debugBlink(state + 1);   
  }
// don't change anything unless the knob is moved significantly
  knob = analogRead(KNOB_PIN) / 4;
// if first move after state change require larger move
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
  FastLED.delay(150);
}   //  end of loop

void drawFlicker(int i){
//    Serial.print("   i= ");
//    Serial.print(i);
//    Serial.print(" flicker_counter= ");
//    Serial.print(flicker_counter[i]);
//    Serial.print(" flicker_running= ");
//    Serial.print(flicker_running[i]);      
//    Serial.println();
  if(flicker_running[i] == false){  // if not active decrement delay
    if(flicker_counter[i] <= 0){    // if delay end make actve
      flicker_counter[i] = 0;
      flicker_running[i] = true;  
      return;
    }
    flicker_counter[i]--; 
    return;                 
  }else{                            // if active up-date values and increment counter
    uint8_t bright = flicker_b2[flicker_counter[i]];
    CRGB temp = flicker_color[i]; 
    temp.nscale8(bright);
/*    Serial.print(" bright= ");
    Serial.print(bright);
    Serial.print(" flicker_color.red= ");
    Serial.print(temp.r);
    Serial.print(" flicker_color.green= ");
    Serial.print(temp.g); 
    Serial.print(" flicker_color.blue= ");
    Serial.print(temp.b);
    Serial.print(" flicker_location= ");
    Serial.print(flicker_location[i]);
    Serial.print(" width factor= ");
    Serial.print(width_factor[0]);      
    Serial.println();*/
    for(uint8_t k = 0; k < FLICKER_WIDTH; k++){
      CRGB temp2 = temp.nscale8(width_factor[k]);
      temp2 += bg[i][k];
      leds[flicker_location[i]+k]= temp2;
    }
    if(i == 4){
      fill_gradient_RGB(leds, 80, CHSV(83,161,46), 99, temp.nscale8(width_factor[0]));
//      fill_gradient_RGB(leds, 80, CHSV(83,161,46), 108, CHSV(61,161,50));
    }
    flicker_counter[i]++;  
    if(flicker_counter[i] > 39){   // if end of pattern make not active, set delay, and restore background
      flicker_running[i] = false;
      flicker_counter[i] = random8(flick_delay_min[i],flick_delay_max[i]);
      for(uint8_t k = 0; k < FLICKER_WIDTH; k++){
        leds[flicker_location[i]+k]= bg[i][k];  
      }
      if(i == 4){
        fill_gradient_RGB(leds, 80, CHSV(83,161,46), 108, CHSV(61,161,50));
      }
    }           
  } // end if active
  return;
}

