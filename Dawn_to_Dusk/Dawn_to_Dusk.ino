// Lighting for Dawn_to_Dusk by Jeanie Holt
// code clean-up: 10/22/2017
// top right=19; right bottom=42; bottom left=62; left top=85

/////  Includes  /////
#include <Adafruit_NeoPixel.h>

/////  Defines  /////
#define LED_PIN A0   // data output pin to LED string
#define NUM_LEDS 90  //actually 86 but dithering needs divisable by 5 

/////  Global Variables  /////
//Initializing lighting break (brk) points
// refrence LEDs can have up to 12 breakpoints including the ends
// co[LED][color][brk_pnt] reserved space for 12 breakpoints just in case
// color=0 is distance to previous breakpoint; color=1,2,3 is Red,Green,Blue
uint8_t co[12][4][12]={
   {                           // LED 0  Top left hand corner    8
      {0,8,24, 25, 12, 18,25,14}, // distance from last breakpoint
      {0,7,85,239,239,100, 0, 0},  // break point red value
      {0,4,48,239,239, 50, 0, 0}, //             green
      {0,6,70,186,286,  5, 0, 0}   //             blue
   },
   {                        // LED 10  center of top    9
      {0,11,21, 26, 11, 13,  6,30,9}, 
      {0, 7,85,255,255,191,200, 0,0},
      {0, 4,48,255,255,191,100, 0,0},
      {0, 6,70,200,200,150, 10, 0,0}
   },
   {                        // LED 20  upper right hand side   11
      {0,14,18, 26, 11, 13,  6,  8,23,4,6}, 
      {0, 7,85,255,255,225,200,100,25,0,0},
      {0, 4,48,255,255,200,100, 25, 0,0,0},
      {0, 6,70,200,200,100, 10,  0, 0,0,0}
   },
   {                        // LED 34         11
      {0,16,16, 22, 15, 11,  8,  9,24,4,4}, 
      {0, 7,85,207,207,225,200,100,25,0,0},
      {0, 4,48,207,207,200,100, 25, 0,0,0},
      {0, 6,70,162,162,100, 10,  0, 0,0,0}
   },
   {                        // LED 38       12
      {0,17,15, 20, 17, 10,  7, 11, 9,18,4,2}, 
      {0, 7,85,191,191,225,200,200,90,25,0,0},
      {0, 4,48,191,191,200,100, 50,15, 0,0,0},
      {0, 6,70,150,150,100, 10, 10, 0, 0,0,0}
   },
   {                        // LED 42     Right side Bottom    11
      {0,18,14, 20, 17,  9,  7, 11,11,18,4}, 
      {0, 7,85,191,191,225,200,200,90,25,0},
      {0, 4,48,191,191,200,100, 50,15, 0,0},
      {0, 6,70,150,150,100, 10, 10, 0, 0,0}
   },
      {                     // LED 43 lower left hand corner    6
      {0,31,20,17,35,21},   
      {0, 0, 0, 0, 0, 0},   
      {0, 0, 0, 0, 0, 0},   
      {0, 0, 0, 0, 0, 0}   
   },      
      {                     // LED 62 lower left hand corner    6
      {0,31,20,17,35,21},   
      {0, 0, 0, 0, 0, 0},   
      {0, 0, 0, 0, 0, 0},   
      {0, 0, 0, 0, 0, 0}   
   },   
      {                     // LED 63 lower left hand corner  6
      {0, 31, 20, 17,35,21},   
      {1, 80,191,191, 0, 0},   
      {0, 30,191,191, 0, 0},   
      {1,100,150,150, 0, 0}   
   },
   {                        // LED 67    7
      {0,3,29, 20, 17,37,19}, 
      {0,1,77,191,191, 0, 0},
      {0,0,29,191,191, 0, 0},
      {0,1,97,150,150, 0, 0}
   },
    {                        // LED 71    7
      {0,5,27, 20, 17,39,17}, 
      {0,4,77,191,191, 0, 0},
      {0,1,29,191,191, 0, 0},
      {0,4,97,150,150, 0, 0}
   },
      {                        // LED 85   upper left hand side 8
      {0,8,24, 25, 12, 18,25,14}, 
      {0,7,85,239,239,100, 0, 0},
      {0,4,48,239,239, 50, 0, 0},
      {0,6,70,186,286,  5, 0, 0}
   }
};
//Other initializations 
// led[ ] defines the reference LEDs with defined time series. Linear interp for LEDs between.
byte led[12]{0,10,20,34,38,42,43,62,63,67,71,85};  // zero based index
// out array needs length of LEDs
byte out[3][NUM_LEDS], brk[12], dis[12];
float next[12][3], nowa[12][3], delta[12][3];  

/////  Setup /////
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
  
void setup() {
  delay(1000);  // Start-up delay for stable start
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

/////  Functions  ///// 

/////  Main Loop  ///// 
void loop() {
//Set up for start of dawn; will come back here at end of "night"
  for (uint8_t k = 0; k < 12; k++){   // for each reference LED
    brk[k] = 0; //  keep track of index into co[][][] array
    dis[k] = co[k][0][0]; // distance to previous breakpoint (starts as 0)
    }
 //Timing loop every half second calculate new lighting values
  for (uint8_t t = 0; t < 120; t++){    // each time tick
    // For each indicator LED up-date values between break-points
    for (uint8_t k = 0; k < 12; k++){   // each indicator LED
      if (dis[k] <= 0){  // are we at the next break-point?
        brk[k]++; // up-date break-point counter for this indicator LED
        dis[k] = co[k][0][brk[k]] - 1;  // time increments to the next break-point
        for (uint8_t i = 0; i < 3; i++){
          nowa[k][i] = co[k][i + 1][brk[k]-1];  // values where we are at
          next[k][i] = co[k][i + 1][brk[k]];    // values at the next break-point
          delta[k][i] = (next[k][i] - nowa[k][i])/(dis[k]); // size of incremental change for each color      
          }    
        }else{
          for (uint8_t i = 0; i < 3; i++){
            nowa[k][i] = nowa[k][i] + delta[k][i]; 
            } 
          }  
      dis[k]--; 
      for (uint8_t i = 0; i < 3; i++){
        out[i][led[k]] = nowa[k][i];
        } 
      }  // end each indicator LED
    for (uint8_t g = 0; g < 11; g++){   // interpolate between indicator LEDs (11 spaces between LEDs)
      uint8_t d = led[g+1] - led[g];
      for (uint8_t b = 1; b < d; b++){  // for each intermediate LED set its color
        for (uint8_t i = 0; i < 3; i++){
          out[i][led[g] + b] = nowa[g][i] + b * (nowa[g + 1][i] - nowa[g][i])/d;
          }
        } 
      } // end interpolate between indicator LEDs
    
// time dither by displaying every 5th wait 0.1 sec shift over one display every 5th etc.  
    uint8_t m_end = NUM_LEDS / 5;
    for (uint8_t i = 0; i < 5; i++){    
      for (uint8_t m = 0; m < m_end; m++){
        uint8_t k = m * 5 + i;
        strip.setPixelColor(k, strip.Color(out[0][k], out[1][k], out[2][k]));
        }   
      strip.show();
      delay(100);
      }
    }  // end timing loop
// while(1){/* stop here */}  // potential debub inertupt
  delay(3000);  // delay for night time then restart dawn to dusk
}
  
 


