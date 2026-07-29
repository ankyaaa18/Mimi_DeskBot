#ifndef clock_weather_h
#define clock_weather_h

#include <Arduino.h>

// Call once after WiFi is connected
void clockWeatherBegin();

// Call from loop() every iteration (non-blocking)
void clockWeatherUpdate();

// Force an immediate weather fetch (optional)
void clockWeatherForceRefresh();

// Draw the clock + weather screen (call this instead of faceUpdate when in clock mode)
void clockWeatherDraw();

// True once NTP has successfully synced at least once
bool clockWeatherTimeValid();

#endif