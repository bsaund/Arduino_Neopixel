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

#define TEST_FACTOR 1  //Set to one for production system

#define TAU (2000000/TEST_FACTOR) // ms between target colors

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 10 // Time (in milliseconds) to pause between pixels

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

class CombinedColors{
  int num_colors;
  Color colors[10];
  double weights[10];

public:
  CombinedColors(){
    num_colors = 0;
  }
  
  void addColor(const Color &c, double weight){
    colors[num_colors] = c;
    weights[num_colors] = weight;
    num_colors++;
  }

  Color normalize(){
//    Serial.println("Normalizing");
    Color c;
    double sum=0, r=0, g=0, b=0;
    for(int i=0; i<num_colors; i++){
      double w = weights[i];
//      Serial.print("w: ");
//      Serial.println(w);
      r += colors[i].r * w;
      g += colors[i].g * w;
      b += colors[i].b * w;
      sum += w;
    }
    c.r = int(r/sum);
    c.b = int(b/sum);
    c.g = int(g/sum);
    return c;
  }
};

Color weightedAverage(Color c1, Color c2, double r){
  CombinedColors c;
  c.addColor(c1, r);
  c.addColor(c2, 1-r);
  return c.normalize();

  
  if(r > 1){
    Serial.println("OH NO! r>1");
  }
  if(r<0){
    Serial.println("OH NO! r<0");
  }
  return Color(c1.r*r + c2.r*(1-r), c1.g*r+c2.g*(1-r), c1.b*r+c2.b*(1-r));
}

double ringDistance(double a, double b, double ring_size=NUMPIXELS){
  return fabs(fmod(fabs(a-b) + NUMPIXELS/2, NUMPIXELS) - NUMPIXELS/2);
}

class TargetColorGenerator{
  long int create_time;
  Color prev_color;
  Color cur_color;
  int color_ind;

  void pickNewColor(){
    Color colors[5] = {Color(20,0,30), Color(30,20,0), Color(0,40,50), Color(200, 200, 200), Color(20,20,20)};
    prev_color = cur_color;
    cur_color = colors[color_ind];
    color_ind = (++color_ind) % 5;
    Serial.print("Color ind ");
    Serial.println(color_ind);
  }
  
public:
  TargetColorGenerator(){
    create_time = millis();
    prev_color = Color();
    color_ind = 0;
    pickNewColor();
    pickNewColor();
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

class WanderingColor{
public:
  Color color;
  double falloff;
  double velocity;

  WanderingColor(const Color& color_, double velocity_, double falloff_){
    color = color_;
    velocity = velocity_;
    falloff = falloff_;
  }

  double getContribution(double loc){
    double my_loc = fmod(velocity * millis(), NUMPIXELS);
    double dist = ringDistance(my_loc, loc);
//    Serial.print("Dist: ");
//    Serial.println(dist);
    if(dist > falloff){
      return 0.0;
    }
//    Serial.print("Contribution: ");
//    Serial.println(1.0 - fabs(my_loc - loc)/falloff);
    return 1.0 - dist/falloff;
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
  WanderingColor wandering_color_1(Color(0,0,150), 0.0003*TEST_FACTOR, 5);
  WanderingColor wandering_color_2(Color(0, 150, 0), 0.0005*TEST_FACTOR, 3);
  WanderingColor wandering_color_3(Color(150,0,0), 0.0007*TEST_FACTOR, 2);
  while(true){
    Color base_color = gen.getTargetColor();
//    Color base_color = Color(20, 20, 20);
 
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      CombinedColors cc;
      cc.addColor(base_color, 0.5);
      cc.addColor(wandering_color_1.color, wandering_color_1.getContribution(i));
      cc.addColor(wandering_color_2.color, wandering_color_2.getContribution(i));
      cc.addColor(wandering_color_3.color, wandering_color_3.getContribution(i));
      Color c = cc.normalize();
  
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(c.r, c.g, c.b));
  
 
    }
    pixels.show();   // Send the updated pixel colors to the hardware.
  
    delay(DELAYVAL); // Pause before next pass through loop
  }
}
