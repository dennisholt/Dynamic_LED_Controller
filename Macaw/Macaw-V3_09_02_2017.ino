// July 25, 2017
// Revision of: strandtest-TuningV3-5-17-17
// Rev: Program is unstable if calling knobs and buttons that are not connected. Use #if DEBUGGING to address.
// for Dana's Macaw
// String length 146
// top right=43; right bottom=72; bottom left=116; left top=145
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define DEBUGGING 0  // 1 to enable debugging button & blinks (slower), 0 to disable for normal running (faster)
#define KNOB_PIN    A3   //A3
#define BTN_PIN     A2 
#define DBUG_LED    A1 
#define PIN         A0    //A0
#define NUM_LEDS    146   //  led string length
#define ONBOARD_LED_PIN    13
#define DEBUG_BLINK 100  // ms on and ms off per blink  (50 = 100ms per blink)
#define DEBUG_PAUSE 200  // ms between pulse sequences  (at 200, 3 50+50ms blinks take 1/2 second, 8 blinks = 1sec including pause)
#define DEBUG_DELAY(ms) delay(ms)  // Normally just delay(ms), change if you need a special delay function like FastLED.delay(ms)
#define MSG_DELAY 5  // number of loops between repeating state blink

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
//  Global Variables
uint8_t msg_delay = 0;
uint8_t knob = 126, old_knob = 126;
uint8_t ColorWheelPos=0;

#if DEBUGGING 
void _btnHandler() {
  // come here when button is pressed
  // Increment state when button is released
  // btn down=0; up=1;
  while (!digitalRead(BTN_PIN)) {   // Wait for button to be released
     delay(10);
  }
  int released = 0;
  while (released < 4) {
    if (!digitalRead(BTN_PIN)) {
      released = 0;
    } else {
      released++;
    }
    delay(1);
  }
  // Action to be taken when button is released
    Serial.print(" ColorWheelPos= ");
    Serial.print(ColorWheelPos);
    Serial.print(" Brightness= ");
    Serial.print(knob);
    Serial.println();
}
#endif  // DEBUGGING  

void setup() {
  Serial.begin(9600);
  delay( 1000 ); // power-up safety delay
  
#if DEBUGGING   
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(DBUG_LED, OUTPUT);
  digitalWrite(DBUG_LED, LOW);
  old_knob = knob = analogRead(KNOB_PIN) / 4;
#endif  // DEBUGGING    

  pinMode(ONBOARD_LED_PIN, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  
  old_knob = knob;

  theaterChaseRainbow(20);
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i++) {
        ColorWheelPos = (j) % 255;  // (i+j) % 255;
        strip.setPixelColor(i+q, Wheel(ColorWheelPos));    //turn every third pixel on
      }
      strip.show();
#if DEBUGGING      
      if(!digitalRead(BTN_PIN)){  // Check for button press
        _btnHandler();
      }
#endif  // DEBUGGING      
      delay(wait); 
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
#if DEBUGGING   
  knob = analogRead(KNOB_PIN) / 4;
#else
  knob = 126;
#endif  // DEBUGGING 
  
  int red=0, green=0, blue=0;
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    red = (255 - WheelPos * 3) * knob/255.0 + 0.5;
    blue = (WheelPos * 3) * knob/255.0 + 0.5;
    return strip.Color(red, green, blue);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    green = (WheelPos * 3) * knob/255.0 + 0.5;
    blue = (255 - WheelPos * 3) * knob/255.0 + 0.5;
    return strip.Color(red, green, blue);
  }
  WheelPos -= 170;
  red = (WheelPos * 3) * knob/255.0 + 0.5;
  green = (255 - WheelPos * 3) * knob/255.0 + 0.5;
  return strip.Color(red, green, blue);
}
