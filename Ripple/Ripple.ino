// Lighting for Ripple by Jeanie Holt
// code clean-up: 10/22/2017
// top right=23; right bottom=40; bottom left=64; left top=81

/////  Includes  /////
#include <Adafruit_NeoPixel.h>

/////  Defines  /////
#define LED_PIN A0    //  data output pin to LED string
#define NUM_LEDS 82   //  led string length 

/////  Global Variables  /////
 int del = 20;  //time delay between loop runs in milliseconds
 int attack = 1000, sustain = 0, decay = 500;  //length of attack/sustain/decay in milliseconds
 int vel = 200;  //milliseconds to shift from 1 LED to next 
 int brt = 0;
 byte start_LED[] = {21, 17, 13}; // ripple starting location
 byte max_brt[] = {250, 150, 200};
 float fade[] = {1.1, 0.9, 1.4};
 byte done[] = {0, 0, 0};
 int rip_time[] = {0, 0, 0};

/////  Setup /////
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  delay(1000);  // Start-up delay for stable start
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  }
   
/////  Functions  ///// 
 int changeBright (int t, int temp_brt) {
  if (temp_brt < 0) temp_brt = 0;
    if (t < attack/del) {
      brt = temp_brt * t/(attack/del);
    }
    else if (t < (attack + sustain)/del) {
      brt = temp_brt;
    }
    else if (t < (attack + sustain + decay)/del) {
      int decay_cnt = t - (attack + sustain)/del;
      brt = temp_brt - temp_brt * decay_cnt/(decay/del);
    }
    else {brt = 0;}
    return brt;
 }
 
 void ripple(int rpl, int r_time) {
    int temp_brt = max_brt[rpl] - fade[rpl] * r_time; // brightness decreases with time
    done[rpl] = 1;                      // set done flag
    
    for (int LEDoffset = 0; LEDoffset < start_LED[rpl] + 1; LEDoffset++) {
    
      // for each location offset the ripple pulse will arrive later in time
      // t_prm becomes local ripple time for the LED at the offset location
      int t_prm = r_time - LEDoffset * vel/del;
      if (t_prm > 0) brt = changeBright (t_prm, temp_brt);
        else brt = 0;
        
      if (brt > 0) done[rpl] = 0;       // not done yet if any offset has brightness
      if (r_time == 0) done[rpl] = 0;   // not done yet
      
      uint8_t here = strip.getPixelColor(start_LED[rpl] - LEDoffset);
      if (here > brt) brt = here;       // if LED is brighter don't change
      
      strip.setPixelColor(start_LED[rpl] - LEDoffset, strip.Color(brt,brt,brt));
    }
 }

/////  Main Loop  ///// 
  void loop() {  //repeating ripple
    for (int j = 0; j < 82; j++) {  // turn off all LEDs
      strip.setPixelColor(j, strip.Color(0,0,0));
      }
    
    for (int i = 0; i < 3; i++) {         // for each ripple    
      ripple(i, rip_time[i]);             // write ripple to LEDs
      rip_time[i]++;                      // increment time
      if (done[i] == 1) rip_time[i] = 0;  // if done start over 
      }
     
    for (int j = 41; j < 64; j++) {       // paint light blue across bottom
      strip.setPixelColor(j, strip.Color(0,0,20)); 
      }
 
    strip.show();
    delay(del);     
  }
