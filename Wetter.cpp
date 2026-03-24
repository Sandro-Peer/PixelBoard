#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
 
#define LED_PIN_1     26
#define LED_PIN_2     25
#define WIDTH         32
#define HEIGHT        8
#define NUM_LEDS      (WIDTH * HEIGHT)
#define COLOR_ORDER   GRB
#define CHIPSET       WS2812
#define BRIGHTNESS    80
#define MATRIX_WIDTH  32
#define MATRIX_HEIGHT 16
 
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
 
const char* ssid     = "Sandroooo";
const char* password = "123456789";
const char* apiKey   = "d7b76766f6710c883e714461869f44bd";
const char* city     = "Innsbruck";   // FIX: Stadtname gesetzt
 
float  weatherTemp      = 16.0;
int    weatherCondition = 800;
String weatherLabel     = "sonnig";
unsigned long lastWeatherFetch = 0;
#define WEATHER_INTERVAL 600000UL
 
// =====================================================
// MATRIX MAPPING
// =====================================================
uint16_t XY_matrix(uint8_t x, uint8_t y) {
  if (x >= WIDTH || y >= HEIGHT) return 0;
  uint8_t realX = WIDTH - 1 - x;
  if (realX % 2 == 0) return (realX * HEIGHT) + y;
  else                 return (realX * HEIGHT) + (HEIGHT - 1 - y);
}
 
void setPixelXY(uint8_t x, uint8_t y, const CRGB& color) {
  if (y < HEIGHT)
    leds1[XY_matrix(x, y)] = color;
  else if (y < 2 * HEIGHT) {
    uint8_t y_flipped = HEIGHT - 1 - (y - HEIGHT);
    uint8_t x_flipped = WIDTH - 1 - x;
    leds2[XY_matrix(x_flipped, y_flipped)] = color;
  }
}
 
// =====================================================
// WLAN & WETTER
// =====================================================
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
 
String conditionToLabel(int code) {
  if (code >= 200 && code < 300) return "gewitter";
  if (code >= 300 && code < 400) return "niesel";
  if (code >= 500 && code < 600) return "regen";
  if (code >= 600 && code < 700) return "schnee";
  if (code >= 700 && code < 800) return "nebel";
  if (code == 800)               return "sonnig";
  if (code == 801)               return "leicht";
  if (code == 802)               return "wolkig";
  if (code >= 803)               return "bedeckt";
  return "unbekannt";
}
 
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city)
             + "&appid=" + String(apiKey) + "&units=metric&lang=de";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, http.getString());
    if (!error) {
      weatherTemp      = doc["main"]["temp"];
      weatherCondition = doc["weather"][0]["id"];
      weatherLabel     = conditionToLabel(weatherCondition);
    }
  }
  http.end();
}
 
// =====================================================
// FONTS
// =====================================================
const uint8_t FONT_LARGE[][7] = {
  {0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110}, // 0
  {0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110}, // 1
  {0b01110,0b10001,0b00001,0b00110,0b01000,0b10000,0b11111}, // 2
  {0b11111,0b00001,0b00010,0b00110,0b00001,0b10001,0b01110}, // 3
  {0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010}, // 4
  {0b11111,0b10000,0b11110,0b00001,0b00001,0b10001,0b01110}, // 5
  {0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b01110}, // 6
  {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}, // 7
  {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}, // 8
  {0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b01100}, // 9
  {0b00100,0b01010,0b00100,0b00000,0b00000,0b00000,0b00000}, // °
  {0b01110,0b10000,0b10000,0b10000,0b10000,0b10000,0b01110}, // C
  {0b00000,0b00000,0b00000,0b11111,0b00000,0b00000,0b00000}, // -
};
 
const uint8_t FONT_SM[][5] = {
  {0b010,0b111,0b101,0b101,0b010}, // a
  {0b110,0b101,0b110,0b101,0b110}, // b
  {0b011,0b100,0b100,0b100,0b011}, // c
  {0b110,0b101,0b101,0b101,0b110}, // d
  {0b111,0b100,0b111,0b100,0b111}, // e
  {0b111,0b100,0b110,0b100,0b100}, // f
  {0b011,0b100,0b101,0b101,0b011}, // g
  {0b101,0b101,0b111,0b101,0b101}, // h
  {0b111,0b010,0b010,0b010,0b111}, // i
  {0b011,0b001,0b001,0b101,0b010}, // j
  {0b101,0b110,0b100,0b110,0b101}, // k
  {0b100,0b100,0b100,0b100,0b111}, // l
  {0b101,0b111,0b101,0b101,0b101}, // m
  {0b110,0b101,0b101,0b101,0b101}, // n
  {0b010,0b101,0b101,0b101,0b010}, // o
  {0b110,0b101,0b110,0b100,0b100}, // p
  {0b010,0b101,0b101,0b011,0b001}, // q
  {0b110,0b101,0b110,0b101,0b101}, // r
  {0b011,0b100,0b010,0b001,0b110}, // s
  {0b111,0b010,0b010,0b010,0b010}, // t
  {0b101,0b101,0b101,0b101,0b011}, // u
  {0b101,0b101,0b101,0b010,0b010}, // v
  {0b101,0b101,0b101,0b111,0b101}, // w
  {0b101,0b010,0b010,0b010,0b101}, // x
  {0b101,0b101,0b010,0b010,0b010}, // y
  {0b111,0b001,0b010,0b100,0b111}, // z
  {0b000,0b000,0b000,0b000,0b000}, // space
  {0b000,0b000,0b111,0b000,0b000}, // -
};
 
// =====================================================
// ZEICHNEN
// =====================================================
void drawLargeChar(int x_off, int y_off, uint8_t idx, CRGB color) {
  if (idx > 12) return;
  for (int row = 0; row < 7; row++) {
    uint8_t bits = FONT_LARGE[idx][row];
    for (int col = 0; col < 5; col++) {
      if (!(bits & (1 << (4 - col)))) continue;
      int gx = MATRIX_WIDTH - 1 - (x_off + col);
      int gy = y_off + row;
      if (gx < 0 || gx >= MATRIX_WIDTH || gy < 0 || gy >= MATRIX_HEIGHT) continue;
      setPixelXY(gx, gy, color);
    }
  }
}
 
void drawSmallChar(int x_off, int y_off, uint8_t idx, CRGB color) {
  if (idx > 27) return;
  for (int row = 0; row < 5; row++) {
    uint8_t bits = FONT_SM[idx][row];
    for (int col = 0; col < 3; col++) {
      if (!(bits & (1 << (2 - col)))) continue;
      int gx = MATRIX_WIDTH - 1 - (x_off + col);
      int gy = y_off + row;
      if (gx < 0 || gx >= MATRIX_WIDTH || gy < HEIGHT || gy >= MATRIX_HEIGHT) continue;
      setPixelXY(gx, gy, color);
    }
  }
}
 
// =====================================================
// TEMPERATUR
// =====================================================
void drawTemperature() {
  uint8_t chars[6]; int len = 0;
  int t = (int)round(weatherTemp);
  if (t < 0) { chars[len++] = 12; t = -t; }
  if (t >= 10) chars[len++] = t / 10;
  chars[len++] = t % 10;
  chars[len++] = 10; // °
  chars[len++] = 11; // C
 
  CRGB col = CRGB(255, 255, 255);  // gut sichtbar
 
  int totalW = len * 6 - 1;
  int startX = (MATRIX_WIDTH - totalW) / 2;
  int startY = 3; // etwas höher, damit unten Platz für Text bleibt
 
  for (int i = 0; i < len; i++)
    drawLargeChar(startX + i * 6, startY, chars[i], col);
}
 
// =====================================================
// WETTERTEXT
// =====================================================
int labelScrollX = MATRIX_WIDTH;
unsigned long lastLabelStep = 0;
#define LABEL_STEP_MS 75
 
void updateLabelScroll() {
  if (millis() - lastLabelStep < LABEL_STEP_MS) return;
  lastLabelStep = millis();
  labelScrollX--;
  if (labelScrollX < -(int)(weatherLabel.length() * 4))
    labelScrollX = MATRIX_WIDTH;
}
 
void drawWeatherLabel() {
  String lbl = weatherLabel;
  lbl.toLowerCase();
  for (int i = 0; i < (int)lbl.length(); i++) {
    char c = lbl[i];
    uint8_t idx = (c >= 'a' && c <= 'z') ? (uint8_t)(c - 'a')
                : (c == ' ')             ? 26
                : (c == '-')             ? 27 : 26;
    int xPos = labelScrollX + i * 4;
    //if (xPos > MATRIX_WIDTH || xPos < -3) continue;
    //drawSmallChar(xPos, 11, idx, CRGB(0, 140, 255)); // unten auf der zweiten Matrix
  }
}
 
// =====================================================
// HINTERGRUND
// =====================================================
struct Particle { float x; float y; float speed; };
Particle particles[20];
bool particlesInit = false;
 
void initParticles() {
  for (int i = 0; i < 20; i++) {
    particles[i] = { (float)random(0, MATRIX_WIDTH),
                     (float)random(0, MATRIX_HEIGHT * 100) / 100.0f,
                     random(10, 40) / 100.0f };
  }
  particlesInit = true;
}
 
void drawBackground() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds1[i].fadeToBlackBy(40);
    leds2[i].fadeToBlackBy(40);
  }
 
  // Regen
  if (weatherCondition >= 300 && weatherCondition < 600) {
    if (!particlesInit) initParticles();
    static unsigned long last = 0;
    if (millis() - last > 60) {
      last = millis();
      for (int i = 0; i < 20; i++) {
        particles[i].y += particles[i].speed * 4;
        if (particles[i].y >= MATRIX_HEIGHT) { particles[i].y = 0; particles[i].x = random(0, MATRIX_WIDTH); }
        setPixelXY((int)particles[i].x, (int)particles[i].y, CRGB(20, 60, 220));
      }
    }
  }
  // Schnee
  else if (weatherCondition >= 600 && weatherCondition < 700) {
    if (!particlesInit) initParticles();
    static unsigned long last = 0;
    if (millis() - last > 130) {
      last = millis();
      for (int i = 0; i < 20; i++) {
        particles[i].y += particles[i].speed * 1.2f;
        if (particles[i].y >= MATRIX_HEIGHT) { particles[i].y = 0; particles[i].x = random(0, MATRIX_WIDTH); }
        uint8_t b = 180 + random(0, 50);
        setPixelXY((int)particles[i].x, (int)particles[i].y, CRGB(b, b, b));
      }
    }
  }
  // Sonne
  else if (weatherCondition == 800) {
    if (!particlesInit) initParticles();
    static unsigned long last = 0;
    if (millis() - last > 100) {
      last = millis();
      for (int i = 0; i < 20; i++) {
        particles[i].y += particles[i].speed;
        if (particles[i].y >= MATRIX_HEIGHT) { particles[i].y = 0; particles[i].x = random(0, MATRIX_WIDTH); }
        setPixelXY((int)particles[i].x, (int)particles[i].y, CHSV(35, 220, 130));
      }
    }
  }
  // Bewölkt
  else if (weatherCondition > 800) {
    static unsigned long last = 0; static int cx = 0;
    if (millis() - last > 180) { last = millis(); cx = (cx + 1) % MATRIX_WIDTH; }
    int pts[][2] = {{0,0},{1,0},{2,0},{3,0},{-1,0},{0,-1},{1,-1},{2,-1},{0,1},{1,1},{2,1}};
    for (auto& p : pts) {
      setPixelXY((cx + p[0] + MATRIX_WIDTH) % MATRIX_WIDTH,      constrain(4  + p[1], 0, 7),  CRGB(55,55,55));
      setPixelXY((cx + p[0] + 16 + MATRIX_WIDTH) % MATRIX_WIDTH, constrain(11 + p[1], 8, 15), CRGB(55,55,55));
    }
  }
  // Gewitter
  else if (weatherCondition >= 200 && weatherCondition < 300) {
    static unsigned long lastBolt=0, boltStart=0;
    static bool boltOn=false; static int bx=16;
    if (!boltOn && millis()-lastBolt > (unsigned long)random(800,2500)) {
      boltOn=true; bx=random(3,MATRIX_WIDTH-3); boltStart=lastBolt=millis();
    }
    if (boltOn) {
      int x=bx;
      for (int y=0; y<MATRIX_HEIGHT; y++) {
        setPixelXY(x, y, CRGB(220,220,80));
        if (y%3==1) x+=random(-1,2);
        x=constrain(x,0,MATRIX_WIDTH-1);
      }
      if (millis()-boltStart>80) boltOn=false;
    }
  }
  // Nebel
  else if (weatherCondition >= 700 && weatherCondition < 800) {
    static unsigned long t = 0; t += 2;
    for (int x=0; x<MATRIX_WIDTH; x++) {
      for (int y=0; y<MATRIX_HEIGHT; y++) {
        uint8_t n = inoise8(x*30, y*30, t);
        uint8_t b = map(n, 0, 255, 8, 45);
        setPixelXY(x, y, CRGB(b,b,b+10));
      }
    }
  }
}
 
// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS);
  FastLED.addLeds<CHIPSET, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
 
  // kleiner Start-Sweep, um zu sehen, ob alles läuft
  for (int i = 0; i < MATRIX_WIDTH; i++) {
    FastLED.clear();
    for (int y = 0; y < MATRIX_HEIGHT; y++) setPixelXY(i, y, CRGB(0, 0, 60));
    FastLED.show();
    delay(20);
  }
 
  connectWiFi();
  fetchWeather();
  lastWeatherFetch = millis();
  labelScrollX = MATRIX_WIDTH;
}
 
// =====================================================
// LOOP
// =====================================================
void loop() {
  unsigned long now = millis();
 
  if (now - lastWeatherFetch > WEATHER_INTERVAL) {
    lastWeatherFetch = now;
    fetchWeather();
    particlesInit = false;
    labelScrollX = MATRIX_WIDTH;
  }
 
  drawBackground();
  drawTemperature();
  updateLabelScroll();
  drawWeatherLabel();      // FIX: jetzt wird der Text wirklich gezeichnet
  FastLED.show();
  delay(20);
}
