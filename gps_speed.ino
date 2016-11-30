#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "button.h"

typedef enum EMode {
  EModeDebug = 0,
  EModeCurrentSpeed,
  EModeAverageSpeed,
  EModeTopSpeed,
  EModeCurrentAltitude,
  EModeTopAltitude,
  EModeTotalAltitude,
  EModeDistance,
  EModeLast,
} EMode;


struct SUserData {
  int cur_speed_kmh;
  int avg_speed_kmh;
  int top_speed_kmh;
  int cur_altitude_m;
  int top_altitude_m;
  int tot_altitude_m;
  int distance_km;

  // "private" stuff
  int num_samples;
  float prev_lat;
  float prev_lng;
  int prev_alt;
};

#define STR_GPS_ERROR     F("NO GPS DATA AVAILABLE")
#define STR_WELCOME       F(" Hi :) ")
#define STR_RESET         F(" RESET ")

#define GPS_INTERVAL_MS   1000
#define GPS_TIMEOUT_MS    5000
#define OLED_RESET        10
#define PIN_GPS_RX        4
#define PIN_GPS_TX        3
#define PIN_BUTTON        5
#define GPS_BAUD          9600
#define DEBUG_BAUD        9600

Adafruit_SSD1306          gOLED(OLED_RESET);
TinyGPSPlus               gGPS;
SoftwareSerial            gGPSSerial(PIN_GPS_RX, PIN_GPS_TX);
EMode                     gCurrentMode = EModeDebug;
SButtonData               gButton;
double                    gTS = 0;
SUserData                 gData = { 0 };


#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup() {
  // init serial
  Serial.begin(DEBUG_BAUD);
  gGPSSerial.begin(GPS_BAUD);

  // init button
  HW_SetupButton(&gButton, PIN_BUTTON,  &onButtonEvent);

  // init OLED
  gOLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  OLED_Message(STR_WELCOME);

  // init data 
  gTS = millis();
}


void loop() {


  // if(millis() - gTS < GPS_INTERVAL_MS) {
  //   return;
  // }



  // feed w/ data from GPS during 1 second
  // unsigned long start = millis();
  do {
    HW_ScanButton(&gButton);
    while (gGPSSerial.available()) {
      gGPS.encode(gGPSSerial.read());
    }
  } while (millis() - gTS < GPS_INTERVAL_MS);
  

  // wait to get enough samples
  gData.num_samples++;
  if(gData.num_samples > 1) {
    
    float dist_m = TinyGPSPlus::distanceBetween(gGPS.location.lat(), gGPS.location.lng(), gData.prev_lat, gData.prev_lng);
    
    // current speed
    gData.cur_speed_kmh = (3600.0 * dist_m) / (millis() - gTS);

    // average speed
    gData.avg_speed_kmh += gData.cur_speed_kmh;

    // top speed
    if(gData.cur_speed_kmh > gData.top_speed_kmh) {
      gData.top_speed_kmh = gData.cur_speed_kmh;
    }

    // current altitude
    gData.cur_altitude_m = gGPS.altitude.meters();

    // top altitude
    if(gData.cur_altitude_m > gData.top_altitude_m) {
      gData.top_altitude_m = gData.cur_altitude_m;
    }

    // total descending altitude
    if(gData.prev_alt > gData.cur_altitude_m) {
      gData.tot_altitude_m += (gData.prev_alt - gData.cur_altitude_m);
    }

    // distance
    gData.distance_km += (dist_m / 1000.0);

    // debug
    Serial.print(F("  lat:"));
    Serial.print(gGPS.location.lat(), DEC);
    Serial.print(F("  lng:"));
    Serial.print(gGPS.location.lng(), DEC);
    Serial.print(F("  spd:"));
    Serial.print(gData.cur_speed_kmh, DEC);
    Serial.print(F("  avg:"));
    Serial.print(gData.avg_speed_kmh / gData.num_samples, DEC);
    Serial.print(F("  top:"));
    Serial.print(gData.top_speed_kmh, DEC);
    Serial.print(F("  alt:"));
    Serial.print(gData.cur_altitude_m, DEC);
    Serial.print(F("  top:"));
    Serial.print(gData.top_altitude_m, DEC);
    Serial.print(F("  tot:"));
    Serial.print(gData.tot_altitude_m, DEC);
    Serial.print(F("  dst:"));
    Serial.print(gData.distance_km, DEC);
    Serial.println(F(""));
  }
  
  // update internal stuff
  gData.prev_lng = gGPS.location.lng();
  gData.prev_lat = gGPS.location.lat();
  gData.prev_alt = gData.cur_altitude_m;
  gTS = millis();

  if (millis() - gTS > GPS_TIMEOUT_MS && gGPS.charsProcessed() < 10) {
    OLED_DisplayError(STR_GPS_ERROR);
  }
  else {
    OLED_ShowData();
  }


}


// =========================================================================
// BUTTON HANDLING
// =========================================================================
void onButtonEvent(uint8_t aPin, EButtonScanResult aResult) {
    if (aResult == EButtonClick) {
      // cycle thru modes
      Serial.println(F("PRESS"));
      gCurrentMode = gCurrentMode + 1; // cannot use '++', wtf ?
      if(gCurrentMode == EModeLast) {
        gCurrentMode = 0;
      }
      //OLED_ShowData();
    }
    else if(aResult == EButtonLongpress) {
      Serial.println(F("LONG PRESS"));
      switch(gCurrentMode) {
        case EModeAverageSpeed:
        gData.avg_speed_kmh = 0;
        gData.num_samples = 0;
        OLED_Message(STR_RESET);
        break;
        case EModeTopSpeed:
        gData.top_speed_kmh = 0;
        OLED_Message(STR_RESET);
        break;
        case EModeTopAltitude:
        gData.top_altitude_m = 0;
        OLED_Message(STR_RESET);
        break;
        case EModeTotalAltitude:
        gData.tot_altitude_m = 0;
        gData.prev_alt = 0;
        OLED_Message(STR_RESET);
        break;
        case EModeDistance:
        gData.distance_km = 0;
        OLED_Message(STR_RESET);
        break;
        default: break;
      }
    }
}
// =========================================================================


// =========================================================================
// DISPLAY
// =========================================================================
void OLED_SetHeader() {
    gOLED.setCursor(0, 0);
    gOLED.setTextSize(1);
    gOLED.setTextColor(BLACK, WHITE);
}


void OLED_SetValue() {
  gOLED.setCursor(0, 10);
  gOLED.setTextSize(3);
  gOLED.setTextColor(WHITE);
}


void OLED_DisplayError(const __FlashStringHelper * aMessage) {
  gOLED.clearDisplay();
  gOLED.setCursor(0, 0);
  gOLED.setTextSize(1);
  gOLED.setTextColor(BLACK, WHITE);
  gOLED.println(aMessage);
  gOLED.display();
}


void OLED_Message(const __FlashStringHelper * aMessage) {
  gOLED.clearDisplay();
  gOLED.setCursor(0, 10);
  gOLED.setTextSize(3);
  gOLED.setTextColor(WHITE);
  gOLED.println(aMessage);
  gOLED.display();
  delay(500);
}


void OLED_ShowData() {
  static EMode prev_mode = EModeLast;

  if(gData.num_samples < 2) {
    return;
  }

  gOLED.clearDisplay();
  OLED_SetHeader();
  
  switch(gCurrentMode) {
    case EModeDebug:
    gOLED.setTextColor(WHITE);
    gOLED.setTextSize(1);
    gOLED.setCursor(0, 0);
    gOLED.print(F("SPD ")); gOLED.print(gData.cur_speed_kmh); gOLED.print(F("kmh"));
    gOLED.setCursor(0, 9);
    gOLED.print(F("AVG ")); gOLED.print(gData.avg_speed_kmh / gData.num_samples); gOLED.print(F("kmh"));
    gOLED.setCursor(0, 17);
    gOLED.print(F("TOP ")); gOLED.print(gData.top_speed_kmh); gOLED.print(F("kmh"));
    gOLED.setCursor(0, 25);
    gOLED.print(F("DST ")); gOLED.print(gData.distance_km); gOLED.print(F("km"));
    gOLED.setCursor(64, 0);
    gOLED.print(F("ALT ")); gOLED.print(gData.cur_altitude_m); gOLED.print(F("m"));
    gOLED.setCursor(64, 9);
    gOLED.print(F("TOP ")); gOLED.print(gData.top_altitude_m); gOLED.print(F("m"));
    gOLED.setCursor(64, 17);
    gOLED.print(F("DSC ")); gOLED.print(gData.tot_altitude_m); gOLED.print(F("m"));
    break;
    case EModeCurrentSpeed:
    gOLED.println(F("        SPEED        "));
    OLED_SetValue();
    gOLED.print(gData.cur_speed_kmh); gOLED.println(F("km/h"));
    break;
    case EModeAverageSpeed:
    gOLED.println(F("    AVERAGE SPEED    "));
    OLED_SetValue();
    gOLED.print(gData.avg_speed_kmh / gData.num_samples); gOLED.println(F("km/h"));
    break;
    case EModeTopSpeed:
    gOLED.println(F("      TOP SPEED      "));
    OLED_SetValue();
    gOLED.print(gData.top_speed_kmh); gOLED.println(F("km/h"));
    break;
    case EModeCurrentAltitude:
    gOLED.println(F("       ALTITUDE      "));
    OLED_SetValue();
    gOLED.print(gData.cur_altitude_m); gOLED.println(F("m"));
    break;
    case EModeTopAltitude:
    gOLED.println(F("     TOP ALTITUDE    "));
    OLED_SetValue();
    gOLED.print(gData.top_altitude_m); gOLED.println(F("m"));
    break;
    case EModeTotalAltitude:
    gOLED.println(F("    TOTAL ALTITUDE   "));
    OLED_SetValue();
    gOLED.print(gData.tot_altitude_m); gOLED.println(F("m"));
    break;
    case EModeDistance:
    gOLED.println(F("       DISTANCE      "));
    OLED_SetValue();
    gOLED.print(gData.distance_km); gOLED.println(F("km"));    
    break;
  }

  gOLED.display();
}
// =========================================================================
