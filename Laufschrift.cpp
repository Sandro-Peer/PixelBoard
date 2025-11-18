#include <FastLED.h>
 
// ---------------------------
// Matrix-Definition
// ---------------------------
#define LED_PIN_1   26
#define LED_PIN_2   25
#define WIDTH       32
#define HEIGHT      8
#define NUM_LEDS    (WIDTH * HEIGHT)
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define BRIGHTNESS  64
 
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
 
// ---------------------------
// Font (5x7, einfache ASCII-Zeichen)
// ---------------------------
const uint8_t font5x7[][5] = {
  // nur die Zeichen, die wir brauchen: H, a, l, o
  // H
  {0x7F,0x08,0x08,0x08,0x7F},
  // a
  {0x20,0x54,0x54,0x54,0x78},
  // l
  {0x00,0x41,0x7F,0x40,0x00},
  // o
  {0x38,0x44,0x44,0x44,0x38}
};
 
// ---------------------------
// Matrix-Funktion
// ---------------------------
uint16_t XY_matrix(uint8_t x, uint8_t y) {
  if (x >= WIDTH || y >= HEIGHT) return 0;
  uint8_t realX = WIDTH - 1 - x;
  uint8_t realY = y;
  uint16_t index;
  if (realX % 2 == 0) {
    index = (realX * HEIGHT) + realY;
  } else {
    index = (realX * HEIGHT) + (HEIGHT - 1 - realY);
  }
  return index;
}
 
void setPixelXY(uint8_t x, uint8_t y, const CRGB& color) {
  if (y < HEIGHT) {
    leds1[XY_matrix(x, y)] = color;
  } else if (y < 2 * HEIGHT) {
    uint8_t y_rel = y - HEIGHT;
    uint8_t y_flipped = HEIGHT - 1 - y_rel;
    uint8_t x_flipped = WIDTH - 1 - x;
    leds2[XY_matrix(x_flipped, y_flipped)] = color;
  }
}
 
// ---------------------------
// Hex-Farbcode (#RRGGBB) -> CRGB
// ---------------------------
CRGB hexToColor(const char* hex) {
  // erwartet String wie "#FF00FF"
  long number = strtol(hex + 1, NULL, 16); // +1 überspringt '#'
  byte r = (number >> 16) & 0xFF;
  byte g = (number >> 8) & 0xFF;
  byte b = number & 0xFF;
  return CRGB(r, g, b);
}
 
// ---------------------------
// Text zeichnen
// ---------------------------
void drawChar(int16_t x, int16_t y, const uint8_t *chr, CRGB color) {
  for (uint8_t col = 0; col < 5; col++) {
    uint8_t line = chr[col];
    for (uint8_t row = 0; row < 7; row++) {
      if (line & (1 << row)) {
        setPixelXY(x + col, y + (6 - row), color);
      }
    }
  }
}
 
void drawText(int16_t x, int16_t y, const char *text, CRGB color) {
  int16_t cursorX = x;
  for (uint8_t i = 0; text[i] != '\0'; i++) {
    char c = text[i];
    // Mapping: H=0, a=1, l=2, o=3
    const uint8_t *glyph;
    switch (c) {
      case 'H': glyph = font5x7[0]; break;
      case 'a': glyph = font5x7[1]; break;
      case 'l': glyph = font5x7[2]; break;
      case 'o': glyph = font5x7[3]; break;
      default: glyph = font5x7[2]; break; // fallback: l
    }
    drawChar(cursorX, y, glyph, color);
    cursorX += 6; // 5 Pixel + 1 Abstand
  }
}
 
// ---------------------------
// Setup
// ---------------------------
void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS); 
  FastLED.addLeds<CHIPSET, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}
 
// ---------------------------
// Loop: Laufschrift
// ---------------------------
void loop() {
  static int16_t offset = WIDTH; // Start rechts außerhalb
  FastLED.clear();
 
  // Beispiel: Farbe per Hexcode
  CRGB myColor = hexToColor("#FF00FF"); // Pink/Magenta
  drawText(offset, 4, "Hallo", myColor);
 
  FastLED.show();
  delay(100);
 
  offset--; // nach links verschieben
  if (offset < -30) { // wenn Text ganz raus ist, neu starten
    offset = WIDTH;
  }
}