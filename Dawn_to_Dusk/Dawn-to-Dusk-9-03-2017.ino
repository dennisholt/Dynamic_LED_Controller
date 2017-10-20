// July 25, 2017 New frame and string starts in top left
// revision of: Dawn-Dusk-AutoCycle-9-04-2016-exp
// revision to control for open development inputs
// longer string with LEDs across bottom 
// Sept 04, 2016 New frame & Banggood mini pro
// String length 86
// top right=19; right bottom=42; bottom left=62; left top=85

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

/////  Defines  /////
#define DEV_BOARD 0  // 1 to enable development button & knob, 0 to disable Arduino doesn't work well when not connected
//  This program doesn't utilize these inputs at this time. If later include in #if DEV_BOARD ... #endif 
//  to disable program sections when dev board in not in use.  

#define PIN A0   // data output pin to LED string
#define LED_LENGTH 90  //actually 86 but dithering needs divisable by 5  //60

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_LENGTH, PIN, NEO_GRB + NEO_KHZ800);
  
void setup() {
  Serial.begin(9600);
  delay(1000);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

/////  Global Variables  /////
//Initializing lighting break (brk) points
// refrence LEDs can have up to 12 breakpoints including the ends
// co[LED][color][brk_pnt] reserved space for 13 breakpoints just in case
// color=0 is distance to previous breakpoint; color=1,2,3 is Red,Green,Blue
uint8_t co[12][4][12]={
   {                           // LED 21->1  Top left hand corner
      {0,8,24,25,12,18,25,14}, // distance from last breakpoint
      {0,7,85,239,239,100,0},  // break point red value
      {0,4,48,239,239,50,0,0}, //             green
      {0,6,70,186,286,5,0,0}   //             blue
   },
   {                        // LED 30->11  center of top
      {0,11,21,26,11,13,6,30,9}, 
      {0,7,85,255,255,191,200,0,0},
      {0,4,48,255,255,191,100,0,0},
      {0,6,70,200,200,150,10,0,0}
   },
   {                        // LED 40->21  upper right hand side
      {0,14,18,26,11,13,6,8,23,4,6}, 
      {0,7,85,255,255,225,200,100,25,0,0},
      {0,4,48,255,255,200,100,25,0,0,0},
      {0,6,70,200,200,100,10,0,0,0,0}
   },
   {                        // LED 53->35
      {0,16,16,22,15,11,8,9,24,4,4}, 
      {0,7,85,207,207,225,200,100,25,0,0},
      {0,4,48,207,207,200,100,25,0,0,0},
      {0,6,70,162,162,100,10,0,0,0,0}
   },
   {                        // LED 57->39
      {0,17,15,20,17,10,7,11,9,18,4,2}, 
      {0,7,85,191,191,225,200,200,90,25,0,0},
      {0,4,48,191,191,200,100,50,15,0,0,0},
      {0,6,70,150,150,100,10,10,0,0,0,0}
   },
   {                        // LED 60-43     Right side Bottom 
      {0,18,14,20,17,9,7,11,11,18,4}, 
      {0,7,85,191,191,225,200,200,90,25,0},
      {0,4,48,191,191,200,100,50,15,0,0},
      {0,6,70,150,150,100,10,10,0,0,0}
   },
      {                     // LED 44 lower left hand corner
      {0,31,20,17,35,21},   
      {0,0,0,0,0,0},   
      {0,0,0,0,0,0},   
      {0,0,0,0,0,0}   
   },      
      {                     // LED 63 lower left hand corner
      {0,31,20,17,35,21},   
      {0,0,0,0,0,0},   
      {0,0,0,0,0,0},   
      {0,0,0,0,0,0}   
   },   
      {                     // LED 1->64 lower left hand corner
      {0,31,20,17,35,21},   
      {1,80,191,191,0,0},   
      {0,30,191,191,0,0},   
      {1,100,150,150,0,0}   
   },
   {                        // LED 4->68
      {0,3,29,20,17,37,19}, 
      {0,1,77,191,191,0,0},
      {0,0,29,191,191,0,0},
      {0,1,97,150,150,0,0}
   },
    {                        // LED 8->72
      {0,5,27,20,17,39,17}, 
      {0,4,77,191,191,0,0},
      {0,1,29,191,191,0,0},
      {0,4,97,150,150,0,0}
   },
      {                        // LED 21-> 86   upper left hand side
      {0,8,24,25,12,18,25,14}, 
      {0,7,85,239,239,100,0},
      {0,4,48,239,239,50,0,0},
      {0,6,70,186,286,5,0,0}
   }
};
//Other initializations 
// led[ ] defines the reference LEDs with defined time series. Linear interp for LEDs between.
byte led[12]{1,11,21,35,39,43,44,63,64,68,72,86};
// out array needs length of LEDs
byte out[3][LED_LENGTH + 3], brk[12], dis[12];
int red = 10, green = 5, blue = 5;
float next[12][3], nowa[12][3], delta[12][3];  

void loop() {
//Set up for start of dawn; will come back here at end of "night"
 for (uint8_t k = 0; k < 12; k++){   // for each reference LED
    brk[k] = 0; //  keep track of index into co[][][] array
    dis[k] = co[k][0][0]; // distance to previous breakpoint
    for (uint8_t i = 0; i < 3; i++){    // load first breakpoint color
       next[k][i] = co[k][i + 1][brk[k]];
    }
 }
 //Timing loop 
 for (uint8_t t = 0; t < 120; t++){
    for (uint8_t k = 0; k < 12; k++){
          for (uint8_t i = 0; i < 3; i++){
             nowa[k][i] = next[k][i];
          } 
          if (dis[k] <= 0){
             brk[k]++; 
             dis[k] = co[k][0][brk[k]] - 1;
             for (uint8_t i = 0; i < 3; i++){
                next[k][i] = co[k][i + 1][brk[k]];
             } 
             for (uint8_t i = 0; i < 3; i++){
                delta[k][i] = (next[k][i] - nowa[k][i])/(dis[k]);
             }     
          }
          dis[k]--;
          for (uint8_t i = 0; i < 3; i++){
             next[k][i] = nowa[k][i] + delta[k][i];
           //  while(t == 10){/* empty infinate loop */} // potential debug interupt
          } 
          for (uint8_t i = 0; i < 3; i++){
          out[i][led[k]] = nowa[k][i];
          } 
    }
    for (uint8_t g = 0; g < 11; g++){
      uint8_t d = led[g + 1] - led[g];
      for (uint8_t b = 1; b < d; b++){
        for (uint8_t i = 0; i < 3; i++){
          out[i][led[g] + b] = nowa[g][i] + b * (nowa[g + 1][i] - nowa[g][i])/d;
        }
         /*  Serial.print ("t= "); Serial.print (t); Serial.print ("\t");  
             Serial.print ("g= "); Serial.print (g); Serial.print ("\t"); 
             Serial.print ("d= "); Serial.print (d); Serial.print ("\t");
             Serial.print ("b= "); Serial.print (b); Serial.print ("\t");
             Serial.print ("nowa1= "); Serial.print (nowa[g][0]); Serial.print ("\t"); 
             Serial.print ("nowa2= "); Serial.print (nowa[g+1][0]); Serial.print ("\t"); 
             Serial.print ("out= "); Serial.print (out[0][led[g] + b]); Serial.print ("\t"); 
             Serial.println ();*/ // delay(1000);
            // while(t == 31){/* empty infinate loop */} 
      }
    }
// time dither by displaying every 5th wait 0.1 sec shift over one display every 5th etc.  
    uint8_t m_end = LED_LENGTH / 5;
    for (uint8_t i = 1; i < 6; i++){    
      for (uint8_t m = 0; m < m_end; m++){
        uint8_t k = m * 5 + i;
        red = out[0][k];
        green = out[1][k];
        blue = out[2][k];
        strip.setPixelColor(k-1, strip.Color(red, green, blue));
      }   
      strip.show();
      delay(100);
    }
  }  // end timing loop
// while(1){/* stop here */}  // potential debub inertupt
  delay(3000);  // delay for night time then restart dawn to dusk
}
  
 


