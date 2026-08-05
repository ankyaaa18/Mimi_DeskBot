#include "clock_weather.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// The display object lives in face.cpp – we reuse it
extern Adafruit_SH1106G display;

// ----- Configuration -----
static const float LAT = 19.076f;
static const float LON = 72.878f;
static const long  GMT_OFFSET_SEC = 19800;   // IST = UTC+5:30
static const int   DAYLIGHT_OFFSET_SEC = 0;  // India has no DST
static const char* NTP_SERVER_1 = "pool.ntp.org";
static const char* NTP_SERVER_2 = "time.google.com";

static const unsigned long WEATHER_REFRESH_MS = 10UL * 60UL * 1000UL; // 10 min
static const unsigned long NTP_RETRY_MS       = 30UL * 1000UL;

// ----- State -----
static bool timeValid = false;
static unsigned long lastNtpAttempt = 0;

static float temperature = NAN;
static int   humidity    = -1;
static int   weatherCode = -1;
static float windSpeed   = NAN;
static unsigned long lastWeatherFetch = 0;
static bool weatherOk = false;

// Simple WMO weather-code → short text
static const char* weatherCodeToText(int code)
{
  if (code == 0)               return "Clear";
  if (code >= 1  && code <= 3) return "Cloudy";
  if (code >= 45 && code <= 48) return "Fog";
  if (code >= 51 && code <= 67) return "Rain";
  if (code >= 71 && code <= 77) return "Snow";
  if (code >= 80 && code <= 82) return "Showers";
  if (code >= 95 && code <= 99) return "Thunder";
  return "—";
}

void clockWeatherBegin()
{
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);
  lastNtpAttempt = millis();
  // First weather fetch will happen in update()
}

bool clockWeatherTimeValid()
{
  return timeValid;
}

static void trySyncTime()
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 50)) {   // 50 ms timeout
    timeValid = true;
  } else {
    timeValid = false;
  }
}

static void fetchWeather()
{
  if (WiFi.status() != WL_CONNECTED) {
    weatherOk = false;
    return;
  }

  // Modern Open-Meteo current endpoint
  String url = "https://api.open-meteo.com/v1/forecast?latitude=";
  url += String(LAT, 3);
  url += "&longitude=";
  url += String(LON, 3);
  url += "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m";
  url += "&timezone=Asia%2FKolkata";

  HTTPClient http;
  http.setTimeout(8000);
  http.begin(url);

  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String payload = http.getString();

    // ArduinoJson v6 / v7 compatible
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      JsonObject current = doc["current"];
      if (!current.isNull()) {
        temperature = current["temperature_2m"] | NAN;
        humidity    = current["relative_humidity_2m"] | -1;
        weatherCode = current["weather_code"] | -1;
        windSpeed   = current["wind_speed_10m"] | NAN;
        weatherOk   = true;
      }
    }
  } else {
    weatherOk = false;
  }
  http.end();
  lastWeatherFetch = millis();
}

void clockWeatherForceRefresh()
{
  lastWeatherFetch = 0; // force next update() to fetch
}

void clockWeatherUpdate()
{
  unsigned long now = millis();

  // Keep trying NTP until we have valid time
  if (!timeValid && (now - lastNtpAttempt > NTP_RETRY_MS)) {
    trySyncTime();
    lastNtpAttempt = now;
  } else if (timeValid) {
    // occasional re-check is cheap
    trySyncTime();
  }

  // Weather refresh
  if (now - lastWeatherFetch > WEATHER_REFRESH_MS || lastWeatherFetch == 0) {
    fetchWeather();
  }
}

void clockWeatherDraw()
{
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // ----- Time (large) -----
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 20)) {
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M", &timeinfo);   // 24-hour

    display.setTextSize(3);
    // roughly centre “HH:MM”
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 4);
    display.print(buf);

    // seconds + date under it
    display.setTextSize(1);
    strftime(buf, sizeof(buf), ":%S", &timeinfo);
    display.setCursor(100, 10);
    display.print(buf);

    strftime(buf, sizeof(buf), "%a %d %b", &timeinfo);
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 30);
    display.print(buf);
  } else {
    display.setTextSize(2);
    display.setCursor(20, 12);
    display.print("Syncing...");
  }

  // ----- Weather line -----
  display.setTextSize(1);
  display.setCursor(0, 46);

  if (weatherOk && !isnan(temperature)) {
    display.printf("%.0fC  %s", temperature, weatherCodeToText(weatherCode));
  } else {
    display.print("Weather --");
  }

  // humidity / wind on the very bottom
  display.setCursor(0, 56);
  if (weatherOk) {
    if (humidity >= 0)
      display.printf("%d%% RH", humidity);
    if (!isnan(windSpeed))
      display.printf("  %.0fkm/h", windSpeed);
  } else {
    display.print("Fetching...");
  }

  display.display();
}