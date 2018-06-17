// Read distances from SHARP sensor and "move the color-wheel" with a Neopixel
// T.Goerke 02/2018

// Nano w/328
#include <SharpIR.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL 3
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, PIXEL, NEO_GRB + NEO_KHZ800);

#define ir A7
#define model 1080
// ir: the pin where your sensor is attached
// model: an int that determines your sensor:  1080 for GP2Y0A21Y
//                                            20150 for GP2Y0A02Y
//                                            (working distance range according to the datasheets)

SharpIR SharpIR(ir, model);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pixel.begin();
  pixel.show(); // Initialize all pixels to 'off'
}

void loop() {
  
  static uint32_t avg = 0; 
  int color;
  
  delay(100);   

  int distance=SharpIR.distance();
  
  // two smoothing methods; just comment in the line you want
  // http://bleaklow.com/2012/06/20/sensor_smoothing_and_optimised_maths_on_the_arduino.html
  if (distance < 255 ) avg = distance * 0.2 + avg * 0.8; else avg = 0; color = avg; 
  // you can also remove the resetting part or set to other values like 255 etc.
  
  //if (distance < 255) avg = moving_average(avg, distance); else avg=0; color = avg/8,
  
  Serial.println(color);
  //map(distance, 10, 150, 0, 255)
  pixel.setPixelColor(0, Wheel(color));
  pixel.show();
}


// from Adafruit_NeoPixel strandtest example
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

float moving_average(float avr_old, float avr) {
  float result;
  // http://www.daycounter.com/LabBook/Moving-Average.phtml
  // MA*[i]= MA*[i-1] +X[i] - MA*[i-1]/N
  // MA* is the moving average times N
  result = avr_old + avr - avr_old / 8 ;  // N = 8
  return result;
}

