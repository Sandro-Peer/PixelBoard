#include <Arduino.h>
#include <FastLED.h>
 
// ---------------------------
// Matrix Definition
// ---------------------------
#define LED_PIN_1   26
#define LED_PIN_2   25
#define WIDTH       32
#define HEIGHT      8
#define NUM_LEDS    (WIDTH * HEIGHT)
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define BRIGHTNESS  80
 
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
 
// ---------------------------
// Joystick
// ---------------------------
#define JOY_X 34
#define JOY_Y 35
 
// Joystick Deadzone für präzisere Steuerung
#define JOY_CENTER 2048
#define JOY_DEADZONE 500
#define JOY_THRESHOLD 1200
 
// ---------------------------
// Snake Einstellungen
// ---------------------------
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 16
#define MAX_SNAKE_LENGTH 150
 
struct Point {
  int x;
  int y;
};
 
Point snake[MAX_SNAKE_LENGTH];
int snakeLength = 3;
 
int dirX = 1;
int dirY = 0;
 
// Verhindert sofortiges Umkehren
int lastDirX = 1;
int lastDirY = 0;
 
Point food;
 
unsigned long lastMove = 0;
int gameSpeed = 200;
 
// Geschwindigkeitsanpassung beim Wachsen
int baseSpeed = 200;
int minSpeed = 100;
 
// Farbverlauf für Snake
uint8_t hueOffset = 0;
 
// ---------------------------
// Matrix Mapping (deine Logik)
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
// Game Funktionen
// ---------------------------
void spawnFood() {
  bool validPosition = false;
 
  // Stelle sicher, dass Food nicht auf Snake spawnt
  while (!validPosition) {
    food.x = random(0, MATRIX_WIDTH);
    food.y = random(0, MATRIX_HEIGHT);
   
    validPosition = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        validPosition = false;
        break;
      }
    }
  }
}
 
void resetGame() {
  snakeLength = 3;
 
  // Snake in der Mitte starten
  snake[0] = {16, 8};
  snake[1] = {15, 8};
  snake[2] = {14, 8};
 
  dirX = 1;
  dirY = 0;
  lastDirX = 1;
  lastDirY = 0;
 
  gameSpeed = baseSpeed;
  hueOffset = 0;
 
  spawnFood();
 
  // Kurze Startanimation
  for (int i = 0; i < 3; i++) {
    FastLED.clear();
    FastLED.show();
    delay(200);
   
    for (int j = 0; j < snakeLength; j++) {
      setPixelXY(snake[j].x, snake[j].y, CRGB::Blue);
    }
    setPixelXY(food.x, food.y, CRGB::Red);
    FastLED.show();
    delay(200);
  }
}
 
void readJoystick() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
 
  // Verbesserte Joystick-Logik mit Deadzone
  int xDiff = xVal - JOY_CENTER;
  int yDiff = yVal - JOY_CENTER;
 
  // Ignoriere kleine Bewegungen (Deadzone)
  if (abs(xDiff) < JOY_DEADZONE && abs(yDiff) < JOY_DEADZONE) {
    return;
  }
 
  // Bestimme dominante Richtung
  if (abs(xDiff) > abs(yDiff)) {
    // X-Achse dominiert
    if (xDiff < -JOY_THRESHOLD && lastDirX != 1) {  // Links
      dirX = -1;
      dirY = 0;
    }
    else if (xDiff > JOY_THRESHOLD && lastDirX != -1) {  // Rechts
      dirX = 1;
      dirY = 0;
    }
  } else {
    // Y-Achse dominiert (invertiert wenn nötig)
    if (yDiff < -JOY_THRESHOLD && lastDirY != -1) {
      dirX = 0;
      dirY = 1;
    }
    else if (yDiff > JOY_THRESHOLD && lastDirY != 1) {
      dirX = 0;
      dirY = -1;
    }
  }
}
 
bool checkCollision(int x, int y) {
  // =====================================================
  // MAUERN AUF ALLEN SEITEN = TOD
  // =====================================================
 
  // X-Koordinate: RAND = TOD (links/rechts)
  if (x < 0 || x >= MATRIX_WIDTH) {
    return true;  // ❌ TOD bei linkem/rechtem Rand
  }
 
  // Y-Koordinate: RAND = TOD (oben/unten)
  if (y < 0 || y >= MATRIX_HEIGHT) {
    return true;  // ❌ TOD bei oberem/unterem Rand
  }
 
  // Selbstkollision prüfen (nicht Kopf)
  for (int i = 1; i < snakeLength; i++) {
    if (snake[i].x == x && snake[i].y == y)
      return true;  // ❌ TOD bei Selbstkollision
  }
 
  return false;
}
 
// ---------------------------
// Setup
// ---------------------------
void setup() {
  Serial.begin(115200);
 
  FastLED.addLeds<CHIPSET, LED_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS);
  FastLED.addLeds<CHIPSET, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS);
 
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
 
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
 
  randomSeed(analogRead(0) + analogRead(34) + analogRead(35));
 
  // Startanimation (Regenbogen)
  for (int i = 0; i < 256; i += 4) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      for (int y = 0; y < MATRIX_HEIGHT; y++) {
        setPixelXY(x, y, CHSV(i + x * 8, 255, 255));
      }
    }
    FastLED.show();
    delay(10);
  }
 
  FastLED.clear();
  FastLED.show();
  delay(500);
 
  resetGame();
}
 
// ---------------------------
// Loop
// ---------------------------
void loop() {
 
  if (millis() - lastMove > gameSpeed) {
 
    lastMove = millis();
 
    readJoystick();
 
    int newX = snake[0].x + dirX;
    int newY = snake[0].y + dirY;
 
    // =====================================================
    // KEIN WRAPPING MEHR!
    // Kollisionserkennung übernimmt alles
    // =====================================================
 
    // Kollision prüfen (Wände + Selbstkollision)
    if (checkCollision(newX, newY)) {
      // Tod-Animation
      for (int i = 0; i < 3; i++) {
        // Zeige Snake in Rot blinkend
        for (int j = 0; j < snakeLength; j++) {
          setPixelXY(snake[j].x, snake[j].y, CRGB::Red);
        }
        FastLED.show();
        delay(150);
       
        FastLED.clear();
        FastLED.show();
        delay(150);
      }
     
      resetGame();
      return;
    }
 
    // Snake verschieben
    for (int i = snakeLength; i > 0; i--) {
      snake[i] = snake[i - 1];
    }
 
    snake[0].x = newX;
    snake[0].y = newY;
   
    // Letzte Richtung speichern
    lastDirX = dirX;
    lastDirY = dirY;
 
    // Essen?
    if (newX == food.x && newY == food.y) {
      if (snakeLength < MAX_SNAKE_LENGTH) {
        snakeLength++;
       
        // Geschwindigkeit erhöhen
        gameSpeed = max(minSpeed, baseSpeed - (snakeLength * 3));
       
        // Food-Sammel-Animation
        for (int i = 0; i < 5; i++) {
          setPixelXY(food.x, food.y, CHSV(random(0, 255), 255, 255));
          FastLED.show();
          delay(30);
        }
      }
      spawnFood();
    }
 
    // =====================================================
    // SCHÖNES ZEICHNEN
    // =====================================================
    FastLED.clear();
 
    // Snake mit Regenbogen-Farbverlauf
    for (int i = 0; i < snakeLength; i++) {
      // Farbverlauf von Kopf zu Schwanz
      uint8_t hue = hueOffset + (i * 255 / snakeLength);
     
      // Kopf heller und intensiver
      if (i == 0) {
        setPixelXY(snake[i].x, snake[i].y, CHSV(hue, 200, 255));
      } else {
        // Körper mit abnehmendem Farbton
        uint8_t brightness = 255 - (i * 100 / snakeLength);
        setPixelXY(snake[i].x, snake[i].y, CHSV(hue, 255, brightness));
      }
    }
 
    // Food pulsierend
    uint8_t foodBrightness = beatsin8(60, 100, 255);
    setPixelXY(food.x, food.y, CHSV(0, 255, foodBrightness));
 
    // Hue-Offset erhöhen für Regenbogen-Effekt
    hueOffset++;
 
    FastLED.show();
  }
}
 
