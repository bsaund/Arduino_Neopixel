// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 64 

#define TAU 2000 // ms between target colors

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels

struct Color{
  int r;
  int g;
  int b;

  Color(){
    r=0;
    g=0;
    b=0;
  };
  Color(int r_, int g_, int b_){
    r = r_;
    g = g_;
    b = b_;
  }
};

Color weightedAverage(Color c1, Color c2, double r){
  if(r > 1){
    Serial.println("OH NO! r>1");
  }
  if(r<0){
    Serial.println("OH NO! r<0");
  }
  return Color(c1.r*r + c2.r*(1-r), c1.g*r+c2.g*(1-r), c1.b*r+c2.b*(1-r));
}

class TargetColorGenerator{
  long int create_time;
  Color prev_color;
  Color cur_color;
  int color_ind;

  void pickNewColor(){
    Color colors[3] = {Color(255,0,0), Color(0,255,0), Color(0,0,255)};
    prev_color = cur_color;
    cur_color = colors[color_ind];
    color_ind = (++color_ind) % 3;
    Serial.print("Color ind ");
    Serial.println(color_ind);
  }
  
public:
  TargetColorGenerator(){
    create_time = millis();
    prev_color = Color();
    color_ind = 0;
  }

  Color getTargetColor(){
    long int t = millis();
    if(t > create_time + TAU){
      pickNewColor();
      create_time = t;
    }

    return weightedAverage(cur_color, prev_color, (double)(t - create_time) / TAU);
  }
};



void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  Serial.begin(9600);
}

void loop() {
  TargetColorGenerator gen;
  while(true){
    Color c = gen.getTargetColor();
 
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
  
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(c.r, c.g, c.b));
  
 
    }
    pixels.show();   // Send the updated pixel colors to the hardware.
  
    delay(DELAYVAL); // Pause before next pass through loop
  }
}
