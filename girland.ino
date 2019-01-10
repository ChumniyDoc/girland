#include "FastLED.h"          // библиотека для работы с лентой

// число светодиодов в кольце/ленте
#define LED_COUNT 160
//стартовый светодиод звезды
#define LED_STAR_START 150
//длина звезды
#define LED_STAR_LENGHT 10
// пин, куда подключен DIN ленты
#define LED_DT 13
#define PHOTO_SENSOR 2

// максимальная яркость (0 - 255)
#define MAX_BRIGHT 255
// адаптивная подсветка
#define ADAPT_LIGHT          

struct CRGB leds[LED_COUNT];

unsigned long now;
unsigned long time2exit;

int check() {
    now = millis();

#ifdef ADAPT_LIGHT
    int new_bright = 0x01|map(analogRead(PHOTO_SENSOR), 1, 1023, 5, MAX_BRIGHT);   // считать показания с фоторезистора, перевести диапазон
    LEDS.setBrightness(new_bright);        // установить новую яркость
//    Serial.print("bright:");
//    Serial.println(new_bright);
#endif

    return now < time2exit;
}

void show(){
  for(int n=LED_STAR_START-1;n<LED_STAR_START+LED_STAR_LENGHT;n++){
    int c=leds[n].r+leds[n].g+leds[n].b;
    if (c>511) c/=3;
    if (c>255) c/=2;
    leds[n].r=c;
    leds[n].g/=8;
    leds[n].b/=8;
  }
  LEDS.show();
}

void rnd(int x){
  leds[x].r=random8(0, 255);
  leds[x].g=random8(0, 255);
  leds[x].b=random8(0, 255);          
}

// плавная смена цветов всей ленты
void rainbow_fade() {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
    int thisdelay = random8(0, 100);
    char ihue;
    while (check()) {
        for (int i = 0; i < LED_COUNT; i++)
            leds[i] = CHSV(ihue, 255, 255);
        show();
        delay(thisdelay);
        ihue++;
    }
}

// крутящаяся радуга
void rainbow_loop() {                        //-m3-LOOP HSV RAINBOW
    int thisdelay = random8(0, 100);
    int thisdelay2 = random8(0, 100);
    int inc = random8(1, 10);
    int ihue;
    while (check()) {
        for (int i = 0; i < LED_COUNT; i++) {
            leds[i] = CHSV(ihue, 255, 255);
            show();
            delay(thisdelay);
            ihue = (ihue + inc)&0xff;
        }
        delay(thisdelay2);
    }
}

// случайная смена цветов
void random_burst() {                         //-m4-RANDOM INDEX/COLOR
    int thisdelay = random8(0, 100);
    while (check()) {
//        leds[random8(0, LED_COUNT)] = CHSV(random8(0, 255), 255, 255);
        rnd(random8(0, LED_COUNT));
        show();
        delay(thisdelay);
    }
}

void ror() {
    struct CRGB tmp;
    memcpy(&tmp, &leds[LED_COUNT - 1], sizeof(CRGB));
    memmove(&leds[1], &leds[0], (LED_COUNT - 1) * sizeof(CRGB));
    memcpy(&leds[0], &tmp, sizeof(CRGB));
}

void rol() {
    struct CRGB tmp;
    memcpy(&tmp, &leds[0], sizeof(CRGB));
    memmove(&leds[0], &leds[1], (LED_COUNT - 1) * sizeof(CRGB));
    memcpy(&leds[LED_COUNT - 1], &tmp, sizeof(CRGB));
}

// бегающий паровозик светодиодов
void color_bounceFADE() {
    int thisdelay = random8(0, 100);
    int thishue = random8(0, 255);
    int cnt = random8(1, 9);
    for (int i = 1; i <= cnt; i++)
        leds[i - 1] = CHSV(thishue, 255, 255 - 2 * abs(i * 255 / (cnt + 1) - 127));
    while (check()) {
        for (int i = 0; i < LED_COUNT - cnt; i++) {
            ror();
            show();
            delay(thisdelay);
        }
        for (int i = 0; i < LED_COUNT - cnt; i++) {
            rol();
            show();
            delay(thisdelay);
        }
    }
}

// вращается несколько цветов сплошные или одиночные
// вращается половина красных и половина синих
void ems_lightsALL() {                    //-m7-EMERGENCY LIGHTS (TWO COLOR SINGLE LED)
    int thisdelay = random8(0, 100);
    char cnt = random8(1, LED_COUNT / 10 + 1);
    char mode = random8(0, 2);
    char dir = random8(0, 2);
    for (char i = 0; i < cnt; i++) {
        for (int n = 0; n < LED_COUNT / cnt; n++) {
//            if (mode || !n) leds[0] = CHSV(random8(0, 255), 255, 255);
            if (mode || !n) rnd(0);
            if (dir) ror(); else rol();
            show();
            delay(thisdelay);
        }
    }
    while (check()) {
        if (dir) ror(); else rol();
        show();
        delay(thisdelay);
    }
}

// случайный стробоскоп
void flicker() {                          //-m9-FLICKER EFFECT
    int m = random8(1, 255 / 2);
    while (check()) {
        int thishue = random8(0, 255);
        int random_bright = random8(0, 255);
        int random_delay = random8(10, 100);
        int random_bool = random8(0, random_bright);
        if (random_bool < m) {
            for (int i = 0; i < LED_COUNT; i++)
                leds[i] = CHSV(thishue, 255, random_bright);
            show();
            delay(random_delay);
        }
    }
}

// пульсация со сменой цветов
void pulse_one_color_all_rev() {              //-m10-PULSE BRIGHTNESS ON ALL LEDS TO ONE COLOR
    int thisdelay = random8(0, 100);
    int thishue = random8(0, 255);
    int inc = random8(1, 10);
    int mode = random8(1, 4);
    int ibright;
    while (check()) {
        ibright = 0;
        while (ibright >= 0) {
            for (int idex = 0; idex < LED_COUNT; idex++)
                switch (mode) {
                    case 1:
                        leds[idex] = CHSV(thishue, 255, ibright);
                        break;
                    case 2:
                        leds[idex] = CHSV(thishue, ibright, 255);
                        break;
                    case 3:
                        leds[idex] = CHSV(ibright, 255, 255);
                        break;
                }
            show();
            delay(thisdelay);
            ibright += inc;
            if (ibright > 255) {
                inc = -inc;
                ibright = 255;
            }
        }
        inc = -inc;
    }
}

// плавная смена яркости по вертикали (для кольца)
void fade_vertical() {                    //-m12-FADE 'UP' THE LOOP
    int thisdelay = random8(0, 100);
    int thishue = random8(0, 255);
    int inc = random8(0, 10);
    int ibright;
    while (check()) {
        for (int i = 0; i < LED_COUNT / 2; i++) {
            leds[i] = CHSV(thishue, 255, ibright);
            leds[LED_COUNT - 1 - i] = CHSV(thishue, 255, ibright);
            show();
            delay(thisdelay);
            ibright = (ibright + inc) & 0xFF;
        }
    }
}

// случайная смена цветов
// безумие красных светодиодов
void random_red() {                       //QUICK 'N DIRTY RANDOMIZE TO GET CELL AUTOMATA STARTED
    int thisdelay = random8(0, 10);
    int thishue = random8(0, 255);
    int thissat = random8(0, 255);
    int dutty = random8(0, 100);
    int mode = random8(0, 2);
    while (check()) {
        int x = random8(0, LED_COUNT);
        if (random8(0, 100) < dutty) {
            if (mode) {
//                thishue = random8(0, 255);
//                thissat = random8(128, 255);
                rnd(x);
            }else
            leds[x] = CHSV(thishue, thissat, random8(0, 255));
        } else {
            leds[x].r = 0;
            leds[x].g = 0;
            leds[x].b = 0;
        }
        show();
        delay(thisdelay);
    }
}

// вращаются случайные цвета (паравозики по одному диоду)
// безумие случайных цветов
void random_march() {                   //-m14-RANDOM MARCH CCW
    int thisdelay = random8(0, 100);
    int mode = random8(0, 2);
    while (check()) {
        if (mode) ror(); else rol();
//        leds[0] = CHSV(random8(0, 255), random8(128, 255), random8(0, 255));
        rnd(0);
        show();
        delay(thisdelay);
    }
}

// пульсирует значок радиации
void radiation() {                   //-m16-SORT OF RADIATION SYMBOLISH-
    int thisdelay = random8(0, 10);
    int thishue = random8(0, 255);
    int even = random8(0, 2);
    int mode = random8(0, 2);
    int cnt = random8(1, LED_COUNT / 2);
    float tcount;
    int ibright;
    while (check()) {
        for (int i = 0; i < LED_COUNT; i++) {
            int b = 255 - abs(ibright - 255);
            if ((i / cnt) % 2 == even)
                leds[i] = CHSV(thishue, 255, b);
        }
        show();
        delay(thisdelay);
        ibright += 1;
        if (ibright > 510) {
            ibright = 0;
            if (mode) thishue = random8(0, 255);
        }
    }
}

// красный светодиод бегает по кругу
void color_loop_vardelay() {                    //-m17-COLOR LOOP (SINGLE LED) w/ VARIABLE DELAY
    int thishue = random8(0, 255);
    int x = random8(0, LED_COUNT);
    int mode = random8(0, 2);
    int dir = random8(0, 2);
    while (check()) {
        if (mode) thishue = random8(0, 255);
        leds[0] = CHSV(thishue, 255, 255);
        for (int i = 0; i < LED_COUNT; i++) {
            if (dir) ror(); else rol();
            show();
            int di = abs(x - i);
            int t = constrain((10 / di) * 10, 10, 500);
            delay(t);
        }
    }
}

//переливы
void sin_bright_wave() {        //-m19-BRIGHTNESS SINE WAVE
    int thisdelay = random8(0, 100);
    int thishue = random8(0, 255);
    int inc = random8(25, 100);
    int mode = random8(0, 2);
    int ibright;
    while (check()) {
        for (int i = 0; i < LED_COUNT; i++) {
            int b = 255 - abs(ibright - 255);
            leds[i] = CHSV(thishue, 255, b);
            show();
            delay(thisdelay);
            ibright += inc;
            if (ibright > 510) {
                ibright = 0;
                if (mode) thishue = random8(0, 255);
            }
        }
    }
}

// вспышки спускаются в центр
// красные вспышки спускаются вниз
void pop_horizontal() {        //-m20-POP FROM LEFT TO RIGHT UP THE RING
    int thisdelay = random8(0, 100);
    int thishue = random8(0, 255);
    int thishue2 = thishue;
    int mode = random8(0, 2);
    while (check()) {
        for (int i = 0; i < LED_COUNT / 2; i++) {
            memset8(leds, 0, sizeof(leds));
            leds[i] = CHSV(thishue, 255, 255);
            leds[LED_COUNT - 1 - i] = CHSV(thishue2, 255, 255);
            show();
            delay(thisdelay);
        }
        if (mode) {
            thishue = random8(0, 255);
            thishue2 = random8(0, 255);
        }
    }
}

// эффект пламени
void flame() {                                    //-m22-FLAMEISH EFFECT
    while (check()) {
        int idelay = random8(0, 35);
        float hinc = (90 / (float) LED_COUNT) + random8(0, 3);
        float ihue = 0;
        for (int i = 0; i <= LED_COUNT / 2; i++) {
            ihue = ihue + hinc;
            leds[i] = CHSV(ihue, 255, 255);
            leds[LED_COUNT - 1 - i] = CHSV(ihue, 255, 255);
            show();
            delay(idelay);
        }
    }
}

// радуга в вертикаьной плоскости (кольцо)
void rainbow_vertical() {                        //-m23-RAINBOW 'UP' THE LOOP
    int thisdelay = random8(0, 100);
    int inc = random8(0, 30);
    int ihue;
    while (check()) {
        for (int i = 0; i <= LED_COUNT / 2; i++) {
            ihue = (ihue + inc) & 0xff;
            leds[i] = CHSV(ihue, 255, 255);
            leds[LED_COUNT - 1 - i] = CHSV(ihue, 255, 255);
            show();
            delay(thisdelay);
        }
    }
}

// полицейская мигалка
void ems_lightsSTROBE() {                  //-m26-EMERGENCY LIGHTS (STROBE LEFT/RIGHT)
    while (check()) {
        for (int x = 0; x < 5; x++) {
            for (int i = 0; i < LED_COUNT / 2; i++){
//                leds[i] = CHSV(0, 255, 255);
              leds[i].r=0;
//              leds[i].g=0;
              leds[i].b=255;
            }
            show();
            delay(25);
            memset8(leds, 0, sizeof(leds));
            show();
            delay(25);
        }
        for (int x = 0; x < 5; x++) {
            for (int i = LED_COUNT / 2; i < LED_COUNT; i++){
//                leds[i] = CHSV(160, 255, 255);
              leds[i].r=255;
//              leds[i].g=0;
              leds[i].b=0;
            }
            show();
            delay(25);
            memset8(leds,0,sizeof(leds));
            show();
            delay(25);
        }
    }
}

// уровень звука
// случайные вспышки красного в вертикаьной плоскости
void kitt() {                                      //-m28-KNIGHT INDUSTIES 2000
    int thishue = random8(0, 255);
    while (check()) {
        int rand = random8(1, LED_COUNT / 2);
        for (int i = 0; i < rand; i++) {
            leds[LED_COUNT / 2 + i] = CHSV(thishue, 255, 255);
            leds[LED_COUNT / 2 - i] = CHSV(thishue, 255, 255);
            show();
            delay(100 / rand);
        }
        int rand2 = random8(1, LED_COUNT / 2);
        for (int i = rand; i > 0; i--) {
            leds[LED_COUNT / 2 + i] = CHSV(thishue, 255, 0);
            leds[LED_COUNT / 2 - i] = CHSV(thishue, 255, 0);
            show();
            delay(100 / rand2);
        }
    }
}

// зелёненькие бегают по кругу случайно
void matrix() {                                   //-m29-ONE LINE MATRIX
    int thishue = random8(0, 255);
    int thisdelay = random8(0, 100);
    int mode = random8(0, 2);
    int dir = random8(0, 2);
    while (check()) {
        int rand = random8(0, 100);
        if (rand > 90)
            leds[0] = CHSV(thishue, 255, 255);
        else {
            leds[0].r = 0;
            leds[0].g = 0;
            leds[0].b = 0;
        }
        if (dir) ror(); else rol();
        show();
        delay(thisdelay);
        if (mode)
            thishue = random8(0, 255);
    }
}

// там rainbow_loop инкремент подвинуть и тоже самое
// крутая плавная вращающаяся радуга
void new_rainbow_loop() {                      //-m88-RAINBOW FADE FROM FAST_SPI2
    int thishue;
    int thisdelay = random8(0, 100);
    while (check()) {
        fill_rainbow(leds, LED_COUNT, thishue);
        show();
        delay(thisdelay);
        thishue++;
    }
}

//-----------------------------плавное заполнение цветом-----------------------------------------
void colorWipe() {
    int thishue = random8(0, 255);
    int thisdelay = random8(0, 100);
    int mode = random8(0, 2);
    int dir = random8(0, 2);
    while (check()) {
        for (int i = 0; i < LED_COUNT * 2; i++) {
            if (i < LED_COUNT) leds[0] = CHSV(thishue, 255, 255);
            else {
                leds[0].r = 0;
                leds[0].g = 0;
                leds[0].b = 0;
            }
            if (dir) ror(); else rol();
            show();
            delay(thisdelay);
        }
        if (mode)thishue = random8(0, 255);
    }
}

void setup() {
    Serial.begin(9600);              // открыть порт для связи
    randomSeed(analogRead(0));
    LEDS.addLeds<WS2811, LED_DT, GRB>(leds, LED_COUNT);  // настрйоки для нашей ленты (ленты на WS2811, WS2812, WS2812B)
    LEDS.setBrightness(MAX_BRIGHT);        // установить новую яркость
}

void loop() {
  check();
  time2exit=now+random16(5000,30000);
  int mode=random8(0,21);
  Serial.print("mode:");
  Serial.println(mode);
  switch (mode){
  case 0: rainbow_fade(); break;
  case 1: rainbow_loop(); break;
  case 2: random_burst(); break;
  case 3: random_burst(); break;
  case 4: ems_lightsALL(); break;
  case 5: flicker(); break;
  case 6: pulse_one_color_all_rev(); break;
  case 7: fade_vertical(); break;
  case 8: random_red(); break;
  case 9: random_march(); break;
  case 10: radiation(); break;
  case 11: color_loop_vardelay(); break;
  case 12: sin_bright_wave(); break;
  case 13: pop_horizontal(); break;
  case 14: flame(); break;
  case 15: rainbow_vertical(); break;
  case 16: ems_lightsSTROBE(); break;
  case 17: kitt(); break;
  case 18: matrix(); break;
  case 19: new_rainbow_loop(); break;
  case 20: colorWipe(); break;
  }
}
