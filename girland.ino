#include "FastLED.h"          // библиотека для работы с лентой

// число светодиодов в кольце/ленте
#define LED_COUNT 310
//стартовый светодиод звезды
#define LED_STAR_START 300
//длина звезды
#define LED_STAR_LENGHT 10
//пин, куда подключен DIN ленты
#define LED_DT 13
//пин фотосенсора
#define PHOTO_SENSOR 2

// максимальная яркость (0 - 255)
#define MAX_BRIGHT 255
// адаптивная подсветка
#define ADAPT_LIGHT

//VideoRAM :)
struct CRGB leds[LED_COUNT + 1];

//текущее время
unsigned long NOW;
//время выхода из эффекта
unsigned long TIME2EXIT;

//колво сегментов отображения
uint8_t SEG_CNT;
//колво сегментов одного направления
uint8_t SEG_DIR;
//длина сегмента отображения
unsigned int SEG_LEN;

#define OPT_MODE 1
#define OPT_DIR  2
#define OPT_EVEN 4

uint8_t check() {
#ifdef ADAPT_LIGHT
  int new_bright = 0x01 | map(analogRead(PHOTO_SENSOR), 1, 1023, 5, MAX_BRIGHT);   // считать показания с фоторезистора, перевести диапазон
  LEDS.setBrightness(new_bright);        // установить новую яркость
  //    //Serial.print("bright:");
  //    //Serial.println(new_bright);
#endif

  NOW = millis();
  return NOW < TIME2EXIT;
}

#ifdef LED_STAR_START
  struct CRGB star[LED_STAR_LENGHT];
#endif

void show() {
#ifdef LED_STAR_START
  memcpy(&star[0], &leds[LED_STAR_START], LED_STAR_LENGHT * sizeof(CRGB));
  for (int n = LED_STAR_START; n < LED_STAR_START + LED_STAR_LENGHT; n++) {
    leds[n].r = qadd8(qadd8(leds[n].r, leds[n].g), leds[n].b);
    leds[n].g /= 8;
    leds[n].b /= 8;
  }
#endif
  LEDS.show();
#ifdef LED_STAR_START
  memcpy(&leds[LED_STAR_START], &star[0], LED_STAR_LENGHT * sizeof(CRGB));
#endif
}

void show_delay(uint8_t thisdelay) {
  show();
  delay(thisdelay);
}

void showseg() {
  for (uint8_t n = 1; n < SEG_CNT; n++) {
    if ((n / SEG_DIR) % 2) {
      swap(&leds[SEG_LEN * n], &leds[0], SEG_LEN);
    } else {
      memcpy(&leds[SEG_LEN * n], &leds[0], SEG_LEN * sizeof(CRGB));
    }
  }
  show();
}

void showseg_delay(uint8_t thisdelay) {
  showseg();
  delay(thisdelay);
}

int showseg_delay_check(uint8_t thisdelay) {
  showseg_delay(thisdelay);
  return check();
}

void swap(CRGB *a, CRGB *b, int l) {
  for ( int n = 0; n < l; n++)
    a[l - n - 1] = b[n];
}

void ror(int l) {
  memmove(&leds[1], &leds[0], l * sizeof(CRGB));
  memcpy(&leds[0], &leds[l], sizeof(CRGB));
}

void rol(int l) {
  memcpy(&leds[l], &leds[0], sizeof(CRGB));
  memmove(&leds[0], &leds[1], l * sizeof(CRGB));
}

void rnd(int x) {
  leds[x].r = random8();
  leds[x].g = random8();
  leds[x].b = random8();
}

// плавная смена цветов всей ленты
void rainbow_fade(uint8_t thisdelay, uint8_t thishue) {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
  uint8_t h = random8(0, 5);
  uint8_t s = random8(0, 5);
  uint8_t v = random8(0, 5);
  uint8_t sat;
  uint8_t val;
  while (check()) {
    uint8_t ss = triwave8(sat);
    uint8_t vv = triwave8(val);
    for (int i = 0; i < LED_COUNT; i++)
      leds[i] = CHSV(thishue, ss, vv);
    thishue += h;
    sat += s;
    val += v;
    show_delay(thisdelay);
  }
}

// крутящаяся радуга
// крутая плавная вращающаяся радуга
void rainbow_loop(int thisdelay, uint8_t opt) {                        //-m3-LOOP HSV RAINBOW
  int thisdelay2 = random8(0, 100);
  int cnt = random8(1, 10);
  int inc = random8(-10, 10);
  int ihue;
  while (check()) {
    for (int i = 0; i < SEG_LEN; i++) {
      leds[i] = CHSV((i * 255 / SEG_LEN * cnt + ihue) & 0xff, 255, 255);
      if (opt & OPT_MODE) showseg_delay(thisdelay);
    }
    ihue += inc;
    showseg_delay(thisdelay2);
  }
}

// случайная смена цветов
void random_burst(int thisdelay) {                         //-m4-RANDOM INDEX/COLOR
  while (check()) {
    rnd(random16(0, LED_COUNT));
    show_delay(thisdelay);
  }
}

// бегающий паровозик светодиодов
void color_bounceFADE(int thisdelay, int thishue) {                     //-m6-BOUNCE COLOR (SIMPLE MULTI-LED FADE)
  memset8(leds, 0, sizeof(leds));
  int cnt = random8(1, 10);
  for (int i = 0; i < cnt; i++)
    leds[i] = CHSV(thishue, 255, triwave8(i * 255 / cnt));
  int dir = 1;
  while (check()) {
    for (int i = 0; i < SEG_LEN - cnt - 1; i++) {
      if (dir) ror(SEG_LEN); else rol(SEG_LEN);
      showseg_delay(thisdelay);
    }
    dir = !dir;
  }
}

// вращается несколько цветов сплошные или одиночные
// вращается половина красных и половина синих
void ems_lightsALL(uint8_t thisdelay, uint8_t opt, uint8_t dir) {         //-m7-EMERGENCY LIGHTS (TWO COLOR SINGLE LED)
  uint8_t cnt = random8(1, SEG_LEN / 10 + 1);
  for (uint8_t i = 0; i < cnt; i++) {
    for (uint8_t n = 0; n < SEG_LEN / cnt; n++) {
      if ((opt & OPT_MODE) || !n) rnd(0);
      if (dir) ror(SEG_LEN); else rol(SEG_LEN);
      showseg_delay(thisdelay);
    }
  }
  while (check()) {
    if (dir) ror(SEG_LEN); else rol(SEG_LEN);
    showseg_delay(thisdelay);
  }
}

// случайный стробоскоп
void flicker() {                          //-m9-FLICKER EFFECT
  int m = random8(1, 255 / 2);
  while (check()) {
    int thishue = random8();
    int random_bright = random8();
    int random_delay = random8(10, 100);
    int random_bool = random8(0, random_bright);
    if (random_bool < m) {
      for (int i = 0; i < LED_COUNT; i++)
        leds[i] = CHSV(thishue, 255, random_bright);
      show_delay(random_delay);
    }
  }
}

//// пульсация со сменой цветов
//void pulse_one_color_all_rev(int thisdelay, int thishue) {              //-m10-PULSE BRIGHTNESS ON ALL LEDS TO ONE COLOR
//    int inc = random8(1, 10);
//    int mmode = random8(1, 4);
//    int ibright;
//    while (check()) {
//        ibright = 0;
//        while (ibright >= 0) {
//            for (int idex = 0; idex < LED_COUNT; idex++)
//                switch (mmode) {
//                    case 1:
//                        leds[idex] = CHSV(thishue, 255, ibright);
//                        break;
//                    case 2:
//                        leds[idex] = CHSV(thishue, ibright, 255);
//                        break;
//                    case 3:
//                        leds[idex] = CHSV(ibright, 255, 255);
//                        break;
//                }
//            show_delay(thisdelay);
//            ibright += inc;
//            if (ibright > 255) {
//                inc = -inc;
//                ibright = 255;
//            }
//        }
//        inc = -inc;
//    }
//}

// плавная смена яркости по вертикали (для кольца)
//похоже на sin_wave
void fade_vertical(int thisdelay, int thishue) {                    //-m12-FADE 'UP' THE LOOP
  int inc = random8(2, 128);
  int ibright;
  while (check()) {
    for (int i = 0; i < SEG_LEN / 2; i++) {
      CHSV chsv(thishue, 255, triwave8(ibright));
      leds[i] = chsv;
      leds[SEG_LEN - 1 - i] = chsv;
      ibright += inc;
      showseg_delay(thisdelay);
    }
  }
}

// случайная смена цветов
// безумие красных светодиодов
void random_red(int thisdelay, int thishue, int opt) {         //QUICK 'N DIRTY RANDOMIZE TO GET CELL AUTOMATA STARTED
  int thissat = random8();
  int dutty = random8(0, 100);
  while (check()) {
    int x = random16(0, LED_COUNT);
    if (random8(0, 100) < dutty) {
      if (opt & OPT_MODE) rnd(x);
      else leds[x] = CHSV(thishue, thissat, random8());
    } else {
      leds[x] = 0;
    }
    show_delay(thisdelay);
  }
}

// вращаются случайные цвета (паравозики по одному диоду)
// безумие случайных цветов
void random_march(int thisdelay, int opt) {                   //-m14-RANDOM MARCH CCW
  while (check()) {
    if (opt & OPT_MODE) ror(SEG_LEN); else rol(SEG_LEN);
    rnd(0);
    showseg_delay(thisdelay);
  }
}

// пульсирует значок радиации
void radiation(int thisdelay, int thishue, int opt) {                   //-m16-SORT OF RADIATION SYMBOLISH-
  uint8_t even = opt & OPT_EVEN;
  int cnt = random8(1, 10);
  uint8_t ibright;
  while (check()) {
    for (int i = 0; i < LED_COUNT; i++) {
      if ((i * cnt / LED_COUNT) % 2 == even)
        leds[i] = CHSV(thishue, 255, triwave8(ibright));
    }
    show_delay(thisdelay);
    if (!++ibright) {
      if (opt & OPT_MODE) thishue = random8();
    }
  }
}

// красный светодиод бегает по кругу
void color_loop_vardelay(uint8_t thishue, int mmode, int dir) {       //-m17-COLOR LOOP (SINGLE LED) w/ VARIABLE DELAY
  int x = random16(0, SEG_LEN);
  while (check()) {
    if (mmode) thishue = random8();
    leds[0] = CHSV(thishue, 255, 255);
    for (int i = 0; i < SEG_LEN; i++) {
      if (dir) ror(SEG_LEN); else rol(SEG_LEN);
      int di = abs(x - i);
      int t = constrain((10 / di) * 10, 10, 500);
      showseg_delay(t);
    }
  }
}

//переливы
void sin_bright_wave(int thisdelay, int thishue, uint8_t opt) {        //-m19-BRIGHTNESS SINE WAVE
  int inc = random8(2, 128);
  uint8_t ibright;
  while (check()) {
    for (int i = 0; i < SEG_LEN; i++) {
      int b = triwave8(ibright);
      leds[i] = CHSV(thishue, 255, b);
      showseg_delay(thisdelay);
      ibright += inc;
      if (!b && (opt & OPT_MODE)) thishue = random8();
    }
  }
}

// вспышки спускаются в центр
// красные вспышки спускаются вниз
void pop_horizontal(int thisdelay, int thishue, int mmode) {        //-m20-POP FROM LEFT TO RIGHT UP THE RING
  int thishue2 = thishue;
  while (check()) {
    for (int i = 0; i < SEG_LEN / 2; i++) {
      memset8(leds, 0, sizeof(leds));
      leds[i] = CHSV(thishue, 255, 255);
      leds[SEG_LEN - 1 - i] = CHSV(thishue2, 255, 255);
      showseg_delay(thisdelay);
    }
    if (mmode) {
      thishue = random8();
      thishue2 = random8();
    }
  }
}

// эффект пламени
//радуга в цетр
void flame() {                                    //-m22-FLAMEISH EFFECT
  while (check()) {
    int idelay = random8(0, 35);
    float hinc = ((float) 45 / (float) LED_COUNT) + random8(0, 3);
    float ihue = 0;
    for (int i = 0; i <= LED_COUNT / 2; i++) {
      ihue = ihue + hinc;
      CHSV chsv(ihue, 255, 255);
      leds[i] = chsv;
      leds[LED_COUNT - 1 - i] = chsv;
      show_delay(idelay);
    }
  }
}

// радуга в вертикаьной плоскости (кольцо)
//flame??
void rainbow_vertical(int thisdelay) {                        //-m23-RAINBOW 'UP' THE LOOP
  int inc = random8(0, 30);
  int ihue;
  while (check()) {
    for (int i = 0; i <= LED_COUNT / 2; i++) {
      ihue = (ihue + inc) & 0xff;
      CHSV chsv(ihue, 255, 255);
      leds[i] = chsv;
      leds[LED_COUNT - 1 - i] = chsv;
      show_delay(thisdelay);
    }
  }
}

// полицейская мигалка
void ems_lightsSTROBE() {                  //-m26-EMERGENCY LIGHTS (STROBE LEFT/RIGHT)
  while (check()) {
    for (uint8_t x = 0; x < 5; x++) {
      for (int i = 0; i < SEG_LEN / 2; i++) {
        //        leds[i].r = 0;
        leds[i].b = 255;
      }
      showseg_delay(25);
      memset8(leds, 0, sizeof(leds));
      showseg_delay(25);
    }
    for (uint8_t x = 0; x < 5; x++) {
      for (int i = SEG_LEN / 2; i < SEG_LEN; i++) {
        leds[i].r = 255;
        //        leds[i].b = 0;
      }
      showseg_delay(25);
      memset8(leds, 0, sizeof(leds));
      showseg_delay(25);
    }
  }
}

// уровень звука
// случайные вспышки красного в вертикаьной плоскости
void kitt(int thishue) {                                      //-m28-KNIGHT INDUSTIES 2000
  while (check()) {
    int rand = random16(1, SEG_LEN / 2);
    for (int i = 0; i < rand; i++) {
      CHSV chsv(thishue, 255, 255);
      leds[SEG_LEN / 2 + i] = chsv;
      leds[SEG_LEN / 2 - i] = chsv;
      showseg_delay(100 / rand);
    }
    int rand2 = random8(1, 50);
    for (int i = rand; i > 0; i--) {
      leds[SEG_LEN / 2 + i] = 0;
      leds[SEG_LEN / 2 - i] = 0;
      showseg_delay(100 / rand2);
    }
  }
}

// зелёненькие бегают по кругу случайно
void matrix(int thisdelay, int thishue, int mmode, int dir) {                                   //-m29-ONE LINE MATRIX
  while (check()) {
    int rand = random8(0, 100);
    if (rand > 90)
      leds[0] = CHSV(thishue, 255, 255);
    else {
      leds[0] = 0;
    }
    if (dir) ror(SEG_LEN); else rol(SEG_LEN);
    showseg_delay(thisdelay);
    if (mmode)
      thishue = random8();
  }
}

//// там rainbow_loop инкремент подвинуть и тоже самое
//// крутая плавная вращающаяся радуга
//void new_rainbow_loop(int thisdelay) {                      //-m88-RAINBOW FADE FROM FAST_SPI2
//    int thishue;
//    while (check()) {
//        fill_rainbow(leds, LED_COUNT, thishue);
//        show();
//        delay(thisdelay);
//        thishue++;
//    }
//}

//-----------------------------плавное заполнение цветом-----------------------------------------
void colorWipe(int thisdelay, int thishue, int mmode, int dir) {
  while (check()) {
    for (int i = 0; i < SEG_LEN * 2; i++) {
      if (i < SEG_LEN) leds[0] = CHSV(thishue, 255, 255);
      else {
        leds[0] = 0;
      }
      if (dir) ror(SEG_LEN); else rol(SEG_LEN);
      showseg_delay(thisdelay);
    }
    if (mmode)thishue = random8();
  }
}

//---------------------------------линейный огонь-------------------------------------
void Fire(int SpeedDelay, int Cooling, int Sparking) {
  byte heat[LED_COUNT / 2];
  int cooldown;
  while (check()) {
    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < LED_COUNT / 2; i++) {
      cooldown = random8(0, Cooling * 10 / LED_COUNT / 2 + 2);
      if (cooldown > heat[i]) {
        heat[i] = 0;
      } else {
        heat[i] = heat[i] - cooldown;
      }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = LED_COUNT / 2 - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if ( random8() < Sparking ) {
      int y = random8(0, 7);
      heat[y] = heat[y] + random8(160, 255);
      //heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for ( int j = 0; j < LED_COUNT / 2; j++) {
      setPixelHeatColor(j, heat[j]);
      setPixelHeatColor(LED_COUNT - 1 - j, heat[j]);
    }

    show_delay(SpeedDelay);
  }
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  //  byte t192 = round((temperature / 255.0) * 191);
  //  byte t192 = scale8(temperature, 191);
  byte t192 = temperature * 191 / 255;

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if ( t192 > 0x80) {                    // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if ( t192 > 0x40 ) {            // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

void fade(int f) {
  for ( int i = 0; i < LED_COUNT; i++)
    leds[i].nscale8(f);
}

void ufade() {
  CRGB t = leds[0];
  for ( int i = 1; i < LED_COUNT - 1; i++) {
    CRGB tt = leds[i];
    leds[i] = t / 2 + leds[i] + leds[i + 1] / 2;
    t = tt;
  }
}

void fireworks(int thisdelay, int thishue, int opt) {
  int f = random8(96, 133);
  while (check()) {
    if (opt & OPT_MODE) thishue = random8();
    int i = random(0, LED_COUNT);
    leds[i] = CHSV(thishue, 255, 255);
    if (opt & OPT_DIR) ufade();
    fade(f);
    show_delay(thisdelay);
  }
}

void comet(int thisdelay, int thishue, int opt) {
  int f = random8(96, 133);
  while (check()) {
    for (int i = 0; i < SEG_LEN * ((opt & OPT_MODE) + 1); i++) {
      if (i < SEG_LEN) leds[i] = CHSV(thishue, 255, 255);
      else  leds[SEG_LEN * 2 - i - 1] = CHSV(thishue, 255, 255);
      showseg_delay(thisdelay);
      fade(f);
    }
    if (opt & OPT_EVEN) thishue = random8();
  }
}
void setup() {
  Serial.begin(9600);              // открыть порт для связи
  random16_set_seed(analogRead(0));
  LEDS.addLeds<WS2811, LED_DT, GRB>(leds, LED_COUNT);  // настрйоки для нашей ленты (ленты на WS2811, WS2812, WS2812B)
  //  LEDS.setBrightness(MAX_BRIGHT);        // установить новую яркость
  check(); // установить новую яркость
}

void loop() {
  TIME2EXIT = random16(5000, 30000);
  Serial.print("duration:");
  Serial.println(TIME2EXIT);
  TIME2EXIT += NOW;
  uint8_t effect = random8(0, 40);
  Serial.print("effect:");
  Serial.println(effect);
  uint8_t thisdelay = random8(0, 100);
  Serial.print("delay:");
  Serial.println(thisdelay);
  uint8_t thishue = random8();
  Serial.print("hue:");
  Serial.println(thishue);
  uint8_t opt = random8();
  uint8_t mmode = opt & OPT_MODE;
  Serial.print("mmode:");
  Serial.println(mmode);
  uint8_t dir = opt & OPT_DIR;
  Serial.print("dir:");
  Serial.println(dir);
  SEG_CNT = random8(1, 10);
  Serial.print("SEG_CNT:");
  Serial.println(SEG_CNT);
  SEG_LEN = LED_COUNT / SEG_CNT;
  Serial.print("SEG_LEN:");
  Serial.println(SEG_LEN);
  while (1) {
    SEG_DIR = random8(1, SEG_CNT + 1);
    if (!(SEG_CNT % SEG_DIR)) break;
  }
  Serial.print("SEG_DIR:");
  Serial.println(SEG_DIR);
  //  return;
  switch (effect) {
    case 0:
      rainbow_fade(thisdelay, thishue);
      break;
    case 1:
      rainbow_loop(thisdelay, opt);
      break;
    case 2:
      random_burst(thisdelay);
      break;
    case 3:
      color_bounceFADE(thisdelay, thishue);
      break;
    case 5:
      ems_lightsALL(thisdelay, opt, dir);
      break;
    case 6:
      flicker();
      break;
    case 7:
      fade_vertical(thisdelay, thishue);
      break;
    case 8:
      random_red(thisdelay, thishue, opt);
      break;
    case 9:
      random_march(thisdelay, opt);
      break;
    case 10:
      radiation(thisdelay, thishue, opt);
      break;
    case 11:
      color_loop_vardelay(thishue, mmode, dir);
      break;
    case 12:
      sin_bright_wave(thisdelay, thishue, opt);
      break;
    case 13:
      pop_horizontal(thisdelay, thishue, mmode);
      break;
    case 14:
      flame();
      break;
    case 15:
      rainbow_vertical(thisdelay);
      break;
    case 16:
      ems_lightsSTROBE();
      break;
    case 17:
      kitt(thishue);
      break;
    case 18:
      matrix(thisdelay, thishue, mmode, dir);
      break;
    case 19:
      colorWipe(thisdelay, thishue, mmode, dir);
      break;
    case 20:
      Fire(thisdelay, 55, 120);
      break;
    case 30:
      fireworks(thisdelay, thishue, opt);
      break;
    case 31:
      comet(thisdelay, thishue, opt);
      break;
  }
}

/*
  Скетч использует 10848 байт (35%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 1697 байт (82%) динамической памяти, оставляя 351 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 10754 байт (35%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 1715 байт (83%) динамической памяти, оставляя 333 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 10300 байт (33%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 815 байт (39%) динамической памяти, оставляя 1233 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 9716 байт (31%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 811 байт (39%) динамической памяти, оставляя 1237 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 9410 байт (30%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 809 байт (39%) динамической памяти, оставляя 1239 байт для локальных переменных. Максимум: 2048 байт.

  кетч использует 9662 байт (31%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 813 байт (39%) динамической памяти, оставляя 1235 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 9408 байт (30%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 813 байт (39%) динамической памяти, оставляя 1235 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 9370 байт (30%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 813 байт (39%) динамической памяти, оставляя 1235 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 9414 байт (30%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 813 байт (39%) динамической памяти, оставляя 1235 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 9054 байт (29%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 809 байт (39%) динамической памяти, оставляя 1239 байт для локальных переменных. Максимум: 2048 байт.

  Скетч использует 7034 байт (22%) памяти устройства. Всего доступно 30720 байт.
  Глобальные переменные используют 973 байт (47%) динамической памяти, оставляя 1075 байт для локальных переменных. Максимум: 2048 байт.
*/
