#include <Adafruit_NeoPixel.h>

#define PIN_LED   0
#define PIN_NTC   A1      // A1 = pin P2 on the Digispark
#define NUM_LEDS  3
#define BRILLO    30      // max brightness (0-255), low to save the battery
#define SENSIB    30      // heat needed to reach pink (lower = easier)

Adafruit_NeoPixel ring(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Mood ring colours, from "cold" to "warm"
const uint8_t paleta[][3] = {
  {255,  20,  20},  // red       - resting
  {255,  50,  70},  // cherry
  {255,  70, 120},  // rose
  {255,  20, 160},  // hot pink
  {255,   0, 200},  // neon pink
  {220,   0, 180},  // magenta
  {255,  80, 180},  // bubblegum - warmest
};
#define N_COL 7

int base16;          // resting temperature (x16)
int nivel16 = 0;     // smoothed heat level (x16), 0..4080
uint8_t fase = 0;    // breathing phase

int leerNTC() {
  int s = 0;
  for (uint8_t i = 0; i < 16; i++) s += analogRead(PIN_NTC);
  return s / 16;
}

uint8_t triangulo(uint8_t x) {
  return x < 128 ? x : 255 - x;
}

uint32_t colorHumor(uint8_t p, uint8_t brillo) {
  uint16_t pos = (uint16_t)p * (N_COL - 1);
  uint8_t i = pos >> 8;
  uint8_t f = pos & 0xFF;
  uint8_t c[3];
  for (uint8_t k = 0; k < 3; k++) {
    int a = paleta[i][k], z = paleta[i + 1][k];
    int v = a + (int)(((long)(z - a) * f) >> 8);
    c[k] = (uint16_t)v * brillo / 255;
  }
  return ring.Color(c[0], c[1], c[2]);
}

void setup() {
  ring.begin();
  ring.show();
  base16 = leerNTC() * 16;
}

void loop() {
  int v = leerNTC();
  int d = v - base16 / 16;

  if (d < 3) base16 += (v * 16 - base16) / 50;

  int objetivo = constrain(d, 0, SENSIB) * 255 / SENSIB;
  int dif = objetivo * 16 - nivel16;
  int paso = (dif > 0) ? dif / 10 : dif / 60;
  if (paso == 0 && dif != 0) paso = (dif > 0) ? 1 : -1;
  nivel16 += paso;
  uint8_t nivel = nivel16 / 16;

  fase += 1;
  uint8_t br = BRILLO / 2 + (BRILLO / 2) * triangulo(fase) / 127;

  uint32_t c = colorHumor(nivel, br);
  for (uint8_t i = 0; i < NUM_LEDS; i++) ring.setPixelColor(i, c);
  ring.show();
  delay(30);
}