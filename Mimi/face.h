#ifndef face_h
#define face_h

#include <Arduino.h>

// =====================================================================
//  SETUP / MAIN LOOP HOOK
// =====================================================================
enum FaceMotion {
    FACE_MOTION_STOP,
    FACE_MOTION_FORWARD,
    FACE_MOTION_BACKWARD,
    FACE_MOTION_LEFT,
    FACE_MOTION_RIGHT
};

void faceSetMotion(FaceMotion motion);
// Call once from setup(). Initializes the display and animation state.
void faceBegin();

// Call continuously (every loop() iteration). This is the single place
// where all eye animations are advanced and the display is redrawn.
// It is internally rate-limited, so calling it as often as you like is fine.
void faceUpdate();

// =====================================================================
//  ORIGINAL / COMPATIBLE FUNCTIONS
//  (same names + purpose as before, now backed by the non-blocking
//   animation system instead of delay()-based loops)
// =====================================================================

void cheeks();

// Immediate one-shot draw with an explicit eyelid closure amount.
// Kept for backwards compatibility with any code that called this directly.
void drawEyes(int blinkAmount = 0);

// Triggers a blink of both eyes. Returns immediately; the blink plays
// out over subsequent faceUpdate() calls.
void blink();

// Legacy name for a right-eye wink. Returns immediately.
void Wink();

void moonEyes();
void faceNoEmo();
void faceHappy();

void LookRight();
void RightCenter();
void LookLeft();
void LeftCenter();

// Triggers the quick eyelid "shiver" animation. Non-blocking.
void flicker();

// =====================================================================
//  NEW ANIMATION / STATE API
// =====================================================================

// Smoothly moves both eyes to an (x, y) offset from their resting position.
// Positive x = right, negative x = left. Positive y = down, negative y = up.
void setEyePosition(int x, int y);

void lookLeft();
void lookRight();
void lookCenter();
void lookUp();
void lookDown();

void winkLeft();
void winkRight();

// Enables/disables natural randomized auto-blinking.
// intervalMs / variationMs control the base gap and its random extra range.
void setBlinkMode(bool enabled, unsigned long intervalMs = 2500, unsigned long variationMs = 3000);

// Enables/disables idle mode: eyes drift to random nearby positions over time.
void setIdleMode(bool enabled, unsigned long intervalMs = 2000, unsigned long variationMs = 4000);

// Read-only helpers exposing the "single value" blink amount some callers
// may want, averaged across both eyes (see face.cpp for the independent
// per-eye state these are derived from).
float currentBlink();
float targetBlink();

#endif // FACE_H
