#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
 
#define MATRIX_PIN     6
#define MATRIX_WIDTH  32
#define MATRIX_HEIGHT  8
 
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_PIN,
  NEO_MATRIX_TOP  + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB         + NEO_KHZ800);

const uint16_t textColor = matrix.Color(200, 200, 200);
const uint16_t heartRed1 = matrix.Color(200, 0, 0);
const uint16_t heartRed2 = matrix.Color(100, 0, 0);

int incomingByte = 0; 
int x = matrix.width();
int pass = 0;
int textWidth = 50;
String incomingMessage = "";
String text = "";
 
void setup() {
  Serial.begin(9600); // For receiving content
  
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(textColor);

  configure(".");
}

void loop() {
  scrollText();
  
  if (Serial.available()) {
    incomingMessage = Serial.readString();
    configure(incomingMessage);
  }
}

void configure(String configuration) {

  if(configuration.charAt(0) == '#') {
    text = configuration.substring(7);
    matrix.setTextColor(parseColor(configuration.substring(0, 7)));
  } else {
    text = configuration;
  }

  
  textWidth = text.length() * 6 + matrix.width() + 1;
  x = 0;
}

void scrollText() {
  x--;
  if(x < -textWidth) {
    x = matrix.width();
  }
  
  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  matrix.print(text);

  matrix.show();
  delay(30);
}

uint16_t parseColor(String hexval) {
  long long number = strtol( &hexval[1], NULL, 16);
  
  long r = number >> 16;
  long g = number >> 8 & 0xFF;
  long b = number & 0xFF;

  return matrix.Color(r, g, b);
}

