// Lighting for Storm by Mike Beck
// Set background lighting level
// Simulate lightining flashes in 3 locations 
// Each flash location has a width and direction
// Flash brightness is defined by an array at three points across flash width
// So each flash is created by three LEDs separated by'flash_width_dir'
// Each of the three LEDs follows a brightness array 
// The three flash locations use the same brightness arrays but at different dimming level
// and different direction and different separation between the three LEDs.  
// Code clean-up 10/26/17
// top right=17; right bottom=41; bottom left=59; left top=83

/////////////////////////  NOTES FROM CLEAN-UP   ///////////////
//   Try these out on Storm before art show
//   Might be better to have some light inbetween the three LEDs involved in each flash
//   
///////////////////////////////////////////////////////////////

/////  Includes  /////
#include <FastLED.h>

/////  Defines  /////
#define LED_PIN     A0   
#define NUM_LEDS    84   //  led string length
# define FLASH_TIME_TICKS 30  // length of flash brightness variation array
# define NUM_FLASHES 3
# define FLASH_TRACK 3   // number of points across flash width where brightness is defined

// used for setup
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define CORRECTION  TypicalLEDStrip
#define BRIGHTNESS  255 // dims brightness of entire LED string (255 = no dimming)

/////  Global Variables  /////
CRGB leds[NUM_LEDS];  // Array for LED color values
uint8_t bright = BRIGHTNESS;

//  parameters for flashes 
uint8_t flash_location[NUM_FLASHES]={1,46,72}; // lowest LED location in flash
int8_t flash_width_dir[NUM_FLASHES]={3,2,-4}; // space between flashTrack LEDs
uint8_t flash_time_stretch[NUM_FLASHES]={70,50,100}; // time between flash update
uint8_t flash_dimming[NUM_FLASHES]={255,70,180}; // adjusts relative brightness of flash
uint8_t flash_bright[FLASH_TRACK][FLASH_TIME_TICKS]={  // Brightness pattern of three points 
//  1    2    3    4    5    6    7    8    9    10    // across width of flash 
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

/////  Functions  ///// 
// We will need to reset background after a flash so get and save background 
void get_background(){
  // call once after background is set
  // will save background to global variable bg
  for (int i = 0; i < NUM_FLASHES; i++){
    for(uint8_t k = 0; k < FLASH_TRACK; k++){   
       bg[i][k] = leds[flash_location[i]+k*flash_width_dir[i]];
       }
    }  
  }   // end get_background()

void drawFlash(int i){
  for(uint8_t l = 0; l < FLASH_TIME_TICKS; l++){
    for(uint8_t k = 0; k < FLASH_TRACK; k++){
      uint8_t b = flash_bright[k][l];
      leds[flash_location[i]+k*flash_width_dir[i]]= CRGB(b,b,b).nscale8(flash_dimming[i]);
      }
    FastLED.show(); 
    FastLED.delay(flash_time_stretch[i]); 
    } 
  for(uint8_t k = 0; k < FLASH_TRACK; k++){  // at end of flash replace background
    leds[flash_location[i]+k*flash_width_dir[i]]= bg[i][k];
    }
  FastLED.show();
  }   // end drawFlash()

/////  Setup ///// 
void setup() {
  delay( 1000 ); // power-up safety delay  
  Serial.begin(9600);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( CORRECTION );
  FastLED.setBrightness( bright );  // can set overall brightness dimming amount; 
  FastLED.setDither( 1 );  //  0 = disable temporal dithering
  random16_add_entropy (10000);  // set random seed for random number generator
// set background leds
  fill_gradient_RGB(leds, 0, CRGB(0,0,0), 19, CRGB(0,0,0));
  fill_gradient_RGB(leds, 20, CRGB(0,0,0), 41, CRGB(30,30,30));
  fill_gradient_RGB(leds, 41, CRGB(30,30,30), 50, CRGB(0,0,0));
  fill_gradient_RGB(leds, 51, CRGB(0,0,0), NUM_LEDS-1, CRGB(0,0,0));
  FastLED.show();
  get_background();
  }

/////  Main Loop  ///// 
void loop() {
// random where lighting will strike (0 = top of cloud, 1 = lower right, 2 = left side)
  uint8_t i = 0;
  uint8_t s =  random8(255);
  if (s < 50){   // probability 50/255 flash at top of cloud
    i = 0; 
    }
  else if (s < 165){
    i = 1; 
    }
  else {
    i = 2;
    }
  drawFlash(i);

// random delay to next flash
  s =  random8(255);
  int del = 200 + (40 * s);  // delay between 0.2 and 10.4 seconds
  FastLED.delay(del);
  }   //  end of main loop





 


