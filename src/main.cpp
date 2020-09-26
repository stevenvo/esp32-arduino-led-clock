#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "secrets.h"

#define LED_PIN 5
#define NUM_LEDS 92
#define BRIGHTNESS 250
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100  // 100
#define UPDATES_INTERVAL 1 //sec

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

int* d;
int tick_count = 0;
int color_index = 0;

int segment_2_led_map[4][7][3] = {{
                                      // digit #0
                                      {35, 36, 37},
                                      {38, 39, 40},
                                      {22, 23, 24},
                                      {25, 26, 27},
                                      {28, 29, 30},
                                      {32, 33, 34},
                                      {41, 42, 43},
                                  },
                                  {
                                      // digit #1
                                      {6, 7, 8},
                                      {9, 10, 11},
                                      {13, 14, 15},
                                      {16, 17, 18},
                                      {19, 20, 21},
                                      {3, 4, 5},
                                      {0, 1, 2},
                                  },
                                  {
                                      // digit #2
                                      {64, 65, 66},
                                      {67, 68, 69},
                                      {51, 52, 53},
                                      {54, 55, 56},
                                      {57, 58, 59},
                                      {61, 62, 63},
                                      {48, 49, 50},
                                  },
                                  {
                                      // digit #3
                                      {73, 74, 75},
                                      {76, 77, 78},
                                      {80, 81, 82},
                                      {83, 84, 85},
                                      {86, 87, 88},
                                      {70, 71, 72},
                                      {89, 90, 91},
                                  }};

bool digit_layout[10][7] = {
    // Lets define 10 numbers (0-9) with 7 segments each, 1 = segment is on, 0 =
    // segment is off
    {1, 1, 1, 1, 1, 1, 0},  // 0
    {0, 1, 1, 0, 0, 0, 0},  // 1
    {1, 1, 0, 1, 1, 0, 1},  // 2 -> and so on...
    {1, 1, 1, 1, 0, 0, 1},  // 3
    {0, 1, 1, 0, 0, 1, 1},  // 4
    {1, 0, 1, 1, 0, 1, 1},  // 5
    {1, 0, 1, 1, 1, 1, 1},  // 6
    {1, 1, 1, 0, 0, 0, 0},  // 7
    {1, 1, 1, 1, 1, 1, 1},  // 8
    {1, 1, 1, 1, 0, 1, 1},  // 9
};



void drawDigit(int digit, int val) {
  int(*digit_leds)[3] = segment_2_led_map[digit];
  bool *segments = digit_layout[val];
  // Serial.println(digit);
  for (int seg_id = 0; seg_id < 7; seg_id++) {
    // Serial.println(seg_id);

    for (int idx = 0; idx < 3; idx++) {
      // Serial.print(segment_2_led_map[digit][seg_id][idx]);
      // Serial.print(", ");
      if (segments[seg_id]) {
        leds[segment_2_led_map[digit][seg_id][idx]] =
            ColorFromPalette(currentPalette, color_index, BRIGHTNESS,
                             currentBlending);  // CRGB::Blue;
      } else {
        leds[segment_2_led_map[digit][seg_id][idx]] = CRGB::Black;
      }
      color_index += 3;
    }

    // Serial.println("");
  }
  // Serial.println("");
}

void drawSplit(){
  leds[44] = ColorFromPalette(currentPalette, color_index, BRIGHTNESS, currentBlending);
  color_index += 3;
  leds[45] = ColorFromPalette(currentPalette, color_index, BRIGHTNESS, currentBlending);
  color_index += 3;
  leds[46] = ColorFromPalette(currentPalette, color_index, BRIGHTNESS, currentBlending);
  color_index += 3;
  leds[47] = ColorFromPalette(currentPalette, color_index, BRIGHTNESS, currentBlending);
  color_index += 3;
}

void setupLed() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

  // FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
    currentPalette = PartyColors_p;
    currentBlending = LINEARBLEND;
}

void setupNTP(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-25200); //PST timezone
}

void setup() {
  delay(3000);  // power-up safety delay
  Serial.begin(115200);
  setupLed();
  setupNTP();
}

int start_value = 0;


void drawClock(){
  // Serial.println("drawClock");

  // Serial.print(d[0]); Serial.print(" ");
  drawDigit(0, d[0]);

  // Serial.print(d[1]); Serial.print(" ");
  drawDigit(1, d[1]);

  // Serial.print(d[2]); Serial.print(" ");
  drawDigit(2, d[2]);

  // Serial.print(d[3]); Serial.print(" ");
  drawDigit(3, d[3]);

  drawSplit();

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void updateNTPTime() {

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  // Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  // Serial.print("DATE: ");
  // Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  // Serial.print("HOUR: ");
  // Serial.println(timeStamp);

  d = (int*)malloc(4 * sizeof(int));
  d[0] = timeStamp[0] - '0';
  d[1] = timeStamp[1] - '0';
  d[2] = timeStamp[3] - '0';
  d[3] = timeStamp[4] - '0';

}

void loop() {
  color_index = 0;
  // drawLED();
  if (tick_count == 0) {
    updateNTPTime();
    tick_count++;
  } else if (tick_count == UPDATES_PER_SECOND*5) {
    tick_count = 0;
  } else {
    tick_count++;
  }

  drawClock();
  delay(1000 / UPDATES_PER_SECOND);
}
