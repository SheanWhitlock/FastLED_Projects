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

CRGB rawleds[NUM_LEDS];
CRGBSet leds(rawleds, NUM_LEDS);
CRGBSet staffLeds(leds(0,4));
CRGBSet moonLeds(leds(5,20));
CRGBSet sunLeds(leds(21,32));
struct CRGB * ledarray[] ={staffLeds, moonLeds, sunLeds}; 


//Global Variable Initialzation 
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t gHueFast = 0;
int hueUpdate = 40; // hue update speed
int hueTimer = 40; // Name of the hue timer
int strobeSpeed = 3000; // in ms
uint8_t onOff = 0; // on/off switch
uint8_t rS = 120; //Sun default RGB setup
uint8_t gS = 110;
uint8_t bS = 0;
uint8_t rM = 0; // Moon default RGB setup
uint8_t gM = 80;
uint8_t bM = 200;
//uint8_t staffLeds[] = {0,1,2,3,4};
//uint8_t moonLeds[] = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
//uint8_t sunLeds[] = {21,22,23,24,25,26,27,28,29,30,31,32};


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
    Blynk.virtualWrite(V3, 40);
    Blynk.virtualWrite(V6, 3);
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
  gCurrentPatternNumber = (pinValue - 1);
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

BLYNK_WRITE(V6) // Strobe Speed
{
  uint8_t pinValue = param.asInt();
  strobeSpeed = pinValue*1000;
}


void setup()
{
  Serial.begin(9600);
  delay(1000);
  Blynk.setDeviceName("Sun&Moon");
  Blynk.begin(auth);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}


typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {  defaultSetting, solid, twinkle, twinkleShift, strobe, twinkleTest };


void loop()
{
  Blynk.run();

  gPatterns[gCurrentPatternNumber]();
  EVERY_N_MILLIS_I( hueTimer, 40 ) { // timer name, inital hue shift == 0
    gHue++;
    gHueFast = gHueFast +3;
    hueTimer.setPeriod( hueUpdate ); // updates hue timer
  }
  
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}


void defaultSetting()
{
  fill_rainbow( staffLeds, 5, gHue, 25);
  fill_rainbow( moonLeds, 16, gHueFast, 16);
  fill_rainbow( sunLeds, 12, gHueFast, 21);
}

void solid()
{
  fill_rainbow( leds, 5, gHue, 20); // staff leds
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
}

void twinkle()
{
  solid();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void twinkleShift()
{
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  addGlitter(80);
}

void strobe()
{
  fill_solid( leds, NUM_LEDS, gHue);
  FastLED.delay( strobeSpeed );
}


// Testing Themes
void twinkleTest()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
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

void MRStrobe()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}
