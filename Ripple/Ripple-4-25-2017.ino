#include <Adafruit_NeoPixel.h>
#define LED_PIN A0   // was 6
#define NUM_LEDS 82   //  led string length number of leds was 30
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
/*  strip.setPixelColor(11, 50, 50, 50);
  uint32_t col = strip.getPixelColor(11);
  uint8_t here = strip.getPixelColor(11);
  Serial.print(here);
  Serial.println(); */
      }
   
 int red = 0, green = 0, blue = 0;
 int del = 20;  //time delay in milliseconds
 //was a=1000; s=0; d=500
 int attack = 1000, sustain = 0, decay = 500;  //length of attack/sustain/decay in milliseconds
 int vel = 200;  //milliseconds to shift from 1 LED to next  was 400
 int brt = 0;
 // start_LED were 29, 27, 25  
 byte start_LED[] = {21, 17, 13};
 byte startDelay[] = {0, 0, 0};
 //  max_brt  250, 150, 200
 byte max_brt[] = {250, 150, 200};
 // fade was 1.3, 1.7, 2.0
 float fade[] = {1.1, 0.9, 1.4};
 byte done[] = {0, 0, 0};
 int rip_time[] = {0, 0, 0};

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
    int temp_brt = max_brt[rpl] - fade[rpl] * r_time;
    done[rpl] = 1;
    for (int LEDoffset = 0; LEDoffset < start_LED[rpl] + 1; LEDoffset++) {
      int t_prm = r_time - LEDoffset * vel/del;
      if (t_prm > 0) brt = changeBright (t_prm, temp_brt);
      else brt = 0;
      if (brt > 0) done[rpl] = 0;
      if (r_time == 0) done[rpl] = 0;
      uint8_t here = strip.getPixelColor(start_LED[rpl] - LEDoffset);
      if (here > brt) brt = here;
      strip.setPixelColor(start_LED[rpl] - LEDoffset, strip.Color(brt,brt,brt));
    }
 }
 
void loop() {  //repeating ripple
    for (int j = 0; j < 82; j++) {
      strip.setPixelColor(j, strip.Color(0,0,0));
    }
        
      ripple(0, rip_time[0]); //ripple starting on LED 21
      rip_time[0]++;
      if (done[0] == 1) rip_time[0] = 0;

       ripple(1, rip_time[1]); //ripple starting on LED 17
      rip_time[1]++;
      if (done[1] == 1) rip_time[1] = 0;

       ripple(2, rip_time[2]); //ripple starting on LED 13
      rip_time[2]++;
      if (done[2] == 1) rip_time[2] = 0;
     
      for (int j = 41; j < 64; j++) //j is the pixel number
         {
           strip.setPixelColor(j, strip.Color(0,0,20)); //bottom color; does not change
         }
 
      strip.show();
      delay(del); 
     
}
