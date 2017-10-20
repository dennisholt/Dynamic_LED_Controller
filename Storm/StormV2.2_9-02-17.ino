// Utility to explore rapid brightness change
// for Mike's Storm
// String length 84
// top right=17; right bottom=41; bottom left=59; left top=83
#include <FastLED.h>

/////  Defines  /////
#define LED_PIN    A0   
#define ONBOARD_LED_PIN    13 
#define NUM_LEDS 84   //  led string length
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define CORRECTION  TypicalLEDStrip
#define BRIGHTNESS 255 //  low value brightness if used (switches to 255)
#define DEBUG_DELAY(ms) FastLED.delay(ms)  // Normally just delay(ms), change if you need a special delay function like FastLED.delay(ms)
# define NUM_FLASHES 3
# define FLASH_TRACK 3
# define FLASH_TIME_TICKS 30

//  Global Variables
CRGB leds[NUM_LEDS];  // Array for LED color values
CHSV now;
uint8_t bright = BRIGHTNESS, hue = 0, sat = 200, val = 126;
uint16_t loopCount = 0;

//  parameters for flashes 
uint8_t flash_location[NUM_FLASHES]={1,46,72}; // lowest LED location in flash
int8_t flash_width_dir[NUM_FLASHES]={3,2,-4}; // lowest LED location in flash
uint8_t flash_time_stretch[NUM_FLASHES]={70,50,100}; // lowest LED location in flash
uint8_t flash_dimming[NUM_FLASHES]={255,70,180}; // lowest LED location in flash
uint8_t flash_bright[FLASH_TRACK][FLASH_TIME_TICKS]={
//  1    2    3    4    5    6    7    8    9    10  
  {255, 255, 255,   0,   0, 128, 128, 255, 255,  255, 
   255, 128, 128,  64,  64,  64,  32,  32,  32,   32,
     0,   0,   0,   0,   0,   0,   0,   0,   0,    0},
  {128, 128, 255, 255, 255, 255, 128, 128,  64,   64,
   255, 255, 255, 255, 128, 128, 128,  64,  64,   64,
    32,  32,  32,  20,  18,  16,  14,  12,   5,    0},
  { 32,  32,  32,  64,  64, 128,   0,   0,  32,   32,
    64,  64, 128, 128, 255, 255, 255, 255,  64,   64,
   255, 255, 255, 128, 128, 128,  64,  64,  32,   32}};

CRGB bg[NUM_FLASHES][FLASH_TRACK];    // place to save background colors
void get_background(){
  // call once after background is set
  // will save background to global variable bg
  for (int i = 0; i < NUM_FLASHES; i++){
    for(uint8_t k = 0; k < FLASH_TRACK; k++){   
       bg[i][k] = leds[flash_location[i]+k*flash_width_dir[i]];
    }
  }  
}
 
void setup() {
  Serial.begin(9600);
  delay( 1000 ); // power-up safety delay
//  Serial.print ("Setup"); Serial.println ();
  pinMode(ONBOARD_LED_PIN, OUTPUT);
//  Serial.print ("Setup A  "); Serial.println ();
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( CORRECTION );
  FastLED.setBrightness( bright );
  FastLED.setDither( 1 );  //  0 = disable temporal dithering
// set background leds
   fill_gradient_RGB(leds, 0, CRGB(0,0,0), 19, CRGB(0,0,0));
   fill_gradient_RGB(leds, 20, CRGB(0,0,0), 41, CRGB(30,30,30));
   fill_gradient_RGB(leds, 41, CRGB(30,30,30), 50, CRGB(0,0,0));
   fill_gradient_RGB(leds, 51, CRGB(0,0,0), NUM_LEDS-1, CRGB(0,0,0));
   FastLED.show();
   get_background();
//   Serial.print ("Setup end"); Serial.println ();
}

void loop() {
//  while(true){}   // stop here

// random where lighting will strike (0 = top of cloud, 1 = lower right, 2 = left side)
  uint8_t i = 0;
  random16_add_entropy (10000);
  uint8_t s =  random8(255);
  if (s < 50){
    i = 0; 
  }
  else if (s < 165){
    i = 1; 
  }
  else {
    i = 2;
  }
  drawFlash(i);
//  Serial.print ("Flash"); Serial.println ();

// random delay to next strike
  s =  random8(255);
  int del = 200 + (40 * s);
//  Serial.print ("del= ");Serial.print (del); Serial.println ();
  FastLED.delay(del);

}   //  end of loop

void drawFlash(int i){
  for(uint8_t l = 0; l < FLASH_TIME_TICKS; l++){
    for(uint8_t k = 0; k < FLASH_TRACK; k++){
      uint8_t b = flash_bright[k][l];
//      Serial.print ("b= ");Serial.print (b); Serial.println ();
      leds[flash_location[i]+k*flash_width_dir[i]]= CRGB(b,b,b).nscale8(flash_dimming[i]);
    }
    FastLED.show(); 
    FastLED.delay(flash_time_stretch[i]); 
  } 
  for(uint8_t k = 0; k < FLASH_TRACK; k++){  // at end of flash replace background
    leds[flash_location[i]+k*flash_width_dir[i]]= bg[i][k];
  }
  FastLED.show();
}



 


