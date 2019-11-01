//Sun & Moon Totem

//BYLNK SETUP
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
char auth[] = "wa_JBJQH3z4PubKpj8AqXQKnGT-LvcLM";


//FASTLED SETUP
#include <FastLED.h>
#define DATA_PIN     13
#define NUM_LEDS    33
#define BRIGHTNESS  0
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define FRAMES_PER_SECOND 120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0])) // read array size function

CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);
CRGBSet staffLeds(leds(0, 4));
CRGBSet moonLeds(leds(5, 20));
CRGBSet sunLeds(leds(21, 32));
struct CRGB * ledarray[] = {staffLeds, moonLeds, sunLeds};


//Global Variable Initialzation
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gCyclePatternNumber = 0; // Index number for cycling
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t gHueFast = 0;
int hueUpdate = 25; // hue update speed
uint8_t onOff = 0; // on/off switch
uint8_t cycleOnOff = 0; // cycle on/off switch
uint8_t rS = 120; //Sun default RGB setup
uint8_t gS = 110;
uint8_t bS = 0;
uint8_t rM = 0; // Moon default RGB setup
uint8_t gM = 80;
uint8_t bM = 200;

extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;
uint8_t gCurrentPaletteNumber = 0; // Index number of current palette
CRGBPalette16 gCurrentPalette( CRGB::Black);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );


//BLYNK FUNCTION SETUP
BLYNK_WRITE(V0) // on/off button
{
  uint8_t pinValue = param.asInt();
  if (pinValue == 0) {
    FastLED.setBrightness(0);
    Blynk.virtualWrite(V1, 0);
    onOff = 0;
  }
  if (pinValue == 1) {
    FastLED.setBrightness(25);
    Blynk.virtualWrite(V1, 25); //Update Blynk app with default values
    Blynk.virtualWrite(V3, 25);
    onOff = 1;
  }
}

BLYNK_WRITE(V1) // Brightness Slider
{
  uint8_t pinValue = param.asInt();
  if (onOff == 1) {
    FastLED.setBrightness(pinValue);
  }
}

BLYNK_WRITE(V2) // Mode Select
{
  uint8_t pinValue = param.asInt();
  gCurrentPatternNumber = (pinValue - 1); //cause blynk starts at 1 for some reason
}

BLYNK_WRITE(V3) // Hue Speed Select
{
  uint8_t pinValue = param.asInt();
  hueUpdate = pinValue;
}

BLYNK_WRITE(V4) // SunRGB
{
  uint8_t r = param[0].asInt();
  rS = r;
  uint8_t g = param[1].asInt();
  gS = g;
  uint8_t b = param[2].asInt();
  bS = b;
}

BLYNK_WRITE(V5) // MoonRGB
{
  uint8_t r = param[0].asInt();
  rM = r;
  uint8_t g = param[1].asInt();
  gM = g;
  uint8_t b = param[2].asInt();
  bM = b;
}

BLYNK_WRITE(V6) // Cycle on/off
{
  uint8_t pinValue = param.asInt();
  cycleOnOff = pinValue;
}


void setup()
{
  Serial.begin(9600);
  delay(3000);
  Blynk.setDeviceName("Sun&Moon");
  Blynk.begin(auth);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}


typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {  paletteCycle, solid, twinkle, rainbow, rainbowTwinkle, colorwave, speckles };

void loop()
{
  Blynk.run();
  if ( cycleOnOff == 1) {
    gPatterns[gCyclePatternNumber]();
  }
  else {
    gPatterns[gCurrentPatternNumber]();
  }

  EVERY_N_MILLIS_I( hueTimer, 25 ) { // timer name, inital hue speed == 25
    gHue++;
    gHueFast = gHueFast + 3;
    hueTimer.setPeriod( hueUpdate ); // updates hue timer
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 12); // for colorwave
  }
  EVERY_N_SECONDS( 30 ) { // Color palette timer
    //gCurrentPaletteNumber = random8( gGradientPaletteCount);
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
    Serial.print(gCurrentPaletteNumber);
  }
  EVERY_N_SECONDS( 90 ) { // mode cycle timer
    nextPattern();
  }

  FastLED.show();
  FastLED.delay( 1000 / FRAMES_PER_SECOND );

}


void nextPattern()
{
  // random mode select
  gCyclePatternNumber = random8( ARRAY_SIZE( gPatterns ));
}

//Lighting Modes
/*void defaultSetting()
{
  static uint8_t startindex = 0;
  startindex--;
  fill_palette( leds, NUM_LEDS, startindex, (256 / NUM_LEDS) + 1, gCurrentPalette, 255, LINEARBLEND);
}
*/
void paletteCycle()
{
  static uint8_t startindex = 0;
  startindex--;
  fill_palette( staffLeds, 5, startindex, (256 / 5) + 1, gCurrentPalette, 255, LINEARBLEND);
  fill_palette( moonLeds, 16, startindex, (256 / 16) + 1, gCurrentPalette, 255, LINEARBLEND);
  fill_palette( sunLeds, 12, startindex, (256 / 12) + 1, gCurrentPalette, 255, LINEARBLEND);
}

void rainbow()
{
  fill_rainbow( staffLeds, 5, gHue, 25);
  fill_rainbow( moonLeds, 16, gHueFast, 16);
  fill_rainbow( sunLeds, 12, gHueFast, 21);
}

void solid()
{
  fill_rainbow( staffLeds, 5, gHue, 25); // staff leds
  for (int i = 5; i <= 20; i++) // moon leds
  {
    leds[i].r = rM;
    leds[i].g = gM;
    leds[i].b = bM;
  }
  for (int i = 21; i <= 32; i++) // sun leds
  {
    leds[i].r = rS;
    leds[i].g = gS;
    leds[i].b = bS;
  }
  //addBreathing()
}

void twinkle()
{
  solid();
  addGlitter(40);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void addBreathing()
{
  //sin wave set.brightness
}

void rainbowTwinkle()
{
  rainbow();
  addGlitter(80);
}

void strobe()
{
  fill_solid( leds, NUM_LEDS, gHue);
}

void speckles()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}


// Testing Modes
void colorwave()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 150); // , , max subtractive brightness
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for ( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if ( h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    index = triwave8( index);
    //index = scale8( index, 240);

    CRGB newcolor = ColorFromPalette( gCurrentPalette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 128);
  }
}


void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = map(hueUpdate, 40, 1, 30, 1);
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < 5; i++) {
    staffLeds[i] = ColorFromPalette(gCurrentPalette, gHue + (i * 2), beat - gHue + (i * 10));
  }
  for ( int i = 0; i < 16; i++) {
    moonLeds[i] = ColorFromPalette(gCurrentPalette, gHue + (i * 2), beat - gHue + (i * 10));
  }
  for ( int i = 0; i < 12; i++) {
    sunLeds[i] = ColorFromPalette(gCurrentPalette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void MRSolid()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


//Gradient Definitions
DEFINE_GRADIENT_PALETTE( mono_blue ) {
  0, 178, 229, 242,
  35,  33, 167, 220,
  255,   0,   0, 225
};

DEFINE_GRADIENT_PALETTE( mono_red ) {
  0, 242, 178, 178,
  35, 255,  78, 101,
  255, 255,   0,   0
};

DEFINE_GRADIENT_PALETTE( mono_green ) {
  0, 195, 246, 195,
  35,   0, 125,   0,
  255,   0, 255,   0
};

DEFINE_GRADIENT_PALETTE( mono_magenta ) {
  0, 255, 200, 255,
  35, 255,  95, 255,
  255, 255,   0, 255
};

DEFINE_GRADIENT_PALETTE( cyan_to_orange ) {
  0,   0,   0, 255,
  100,   0, 255, 255,
  125, 168, 220, 209,
  150, 255, 160, 100,
  255, 255, 100,   0
};

DEFINE_GRADIENT_PALETTE( sunset ) {
  0, 255, 255,   0,
  117, 255, 117,   0,
  255, 255,   0,   0
};

DEFINE_GRADIENT_PALETTE( flowergarden ) {
  0, 245, 199, 160,
  25, 242, 145, 145,
  75, 255,  50,  65,
  125, 255,  50,  65,
  126, 126, 178, 221,
  255,  70,  90, 255
};

DEFINE_GRADIENT_PALETTE( beachball ) {
  0,   0,   0, 255,
  50,   0,   0, 255,
  51,  84, 143, 255,
  100,  84, 143, 255,
  101, 255, 255,   0,
  150, 255, 255,   0,
  151, 255, 100, 100,
  200, 255, 100, 100,
  201, 255,   0,  80,
  255, 255,   0,  80
};

DEFINE_GRADIENT_PALETTE( trans ) {
  0,  30, 222, 255,
  50,  30, 222, 255,
  51, 255, 127, 195,
  100, 255, 127, 195,
  101, 255, 255, 255,
  150, 255, 255, 255,
  151, 255, 127, 195,
  200, 255, 127, 195,
  201,  30, 222, 255,
  255,  30, 222, 255
};

DEFINE_GRADIENT_PALETTE( bi ) {
  0, 255,   0, 110,
  117, 127,   0, 127,
  255,   0,   0, 225
};

DEFINE_GRADIENT_PALETTE( pan ) {
  0, 255,   0, 140,
  117, 255, 255,   0,
  255,   0, 255, 255
};

DEFINE_GRADIENT_PALETTE( Blue_Cyan_Yellow_gp ) {
  0,   0,  0, 255,
  63,   0, 55, 255,
  127,   0, 255, 255,
  191,  42, 255, 45,
  255, 255, 255,  0
};

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
  0, 120,  0,  0,
  22, 179, 22,  0,
  51, 255, 104,  0,
  85, 167, 22, 18,
  135, 100,  0, 103,
  198,  16,  0, 130,
  255,   0,  0, 160
};

DEFINE_GRADIENT_PALETTE( gr65_hult_gp ) {
  0, 247, 176, 247,
  48, 255, 136, 255,
  89, 220, 29, 226,
  160,   7, 82, 178,
  216,   1, 124, 109,
  255,   1, 124, 109
};

DEFINE_GRADIENT_PALETTE( rainbowsherbet_gp ) {
  0, 255, 33,  4,
  43, 255, 68, 25,
  86, 255,  7, 25,
  127, 255, 82, 103,
  170, 255, 255, 242,
  209,  42, 255, 22,
  255,  87, 255, 65
};

const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
  mono_blue,
  mono_red,
  mono_green,
  mono_magenta,
  cyan_to_orange,
  sunset,
  flowergarden,
  beachball,
  trans,
  bi,
  pan,
  Blue_Cyan_Yellow_gp,
  Sunset_Real_gp,
  gr65_hult_gp,
  rainbowsherbet_gp,
};


// Count of how many cpt-city gradients are defined:
const uint8_t gGradientPaletteCount =
  sizeof( gGradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
