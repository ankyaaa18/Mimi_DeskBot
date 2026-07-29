#include "face.h"
 
#include <math.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
 
// =====================================================================
//  EYE GEOMETRY (unchanged from the original design)
// =====================================================================
 
const int eyeWidth  = 36;
const int eyeHeight = 40;
const int eyeRadius = 8;
 
// Resting ("home") anchor positions for each eye. All motion is expressed
// as an offset from these anchors, so cheeks(), moonEyes(), etc. keep
// working exactly as before when the eyes are at rest (offset = 0).
const int leftEyeBaseX  = 18;
const int rightEyeBaseX = 74;
const int baseEyeY      = 8;
 
// =====================================================================
//  ANIMATED POSITION STATE
//  eyeX / eyeY are the *current*, rendered values. targetEyeX/targetEyeY
//  are where they are easing towards. Both eyes move together, exactly
//  like the original LookLeft()/LookRight() behaviour.
// =====================================================================
 
float eyeX = 0;            // current horizontal offset from rest
float eyeY = baseEyeY;      // current vertical position (absolute)
float targetEyeX = 0;       // target horizontal offset from rest
float targetEyeY = baseEyeY; // target vertical position (absolute)
 
// How far a "look" gesture moves the eyes, in pixels.
const int LOOK_SIDE_OFFSET     = 10;
const int LOOK_VERTICAL_OFFSET = 8;
 
// Ease-toward-target speed for position. Smaller = smoother/slower.
const float POS_EASING = 0.18f;
 
// =====================================================================
//  BLINK / EYELID STATE (independent per eye, so winks work correctly)
//  blinkAmountL/R are the *current* eyelid closure amount (0 = fully
//  open, eyeHeight = fully closed). targetBlinkL/R are what they are
//  easing towards. This is the same "current chases target" idea used
//  for position, applied to the eyelids.
// =====================================================================
 
float blinkAmountL = 0;
float blinkAmountR = 0;
float targetBlinkL = 0;
float targetBlinkR = 0;
 
// True while a triggered blink/wink is actively playing out (closing,
// holding, or opening again). Lets faceUpdate() know when to reopen.
bool blinkActiveL = false;
bool blinkActiveR = false;
unsigned long blinkHoldUntilL = 0;
unsigned long blinkHoldUntilR = 0;
 
// Closing is quicker than opening (mirrors the original blink()'s
// "delay(6) closing / delay(12) opening" feel), done here via easing
// factors instead of different delay times.
const float BLINK_CLOSE_EASING   = 0.55f;
const float BLINK_OPEN_EASING    = 0.25f;
const unsigned long BLINK_HOLD_MS = 60; // how long the eye stays fully shut
 
// =====================================================================
//  QUICK "FLICKER" SHIVER (non-blocking replacement for the old
//  delay()-driven step sequence)
// =====================================================================
 
bool flickerActive = false;
unsigned long flickerStartTime = 0;
const int flickerSteps[]              = {1, 3, 5, 3, 1, 0};
const int flickerStepCount            = 6;
const unsigned long flickerStepDuration = 15; // ms per step, matches original timing
 
// =====================================================================
//  AUTO-BLINK / IDLE MODE TIMERS
// =====================================================================
 
bool autoBlinkEnabled = false;
unsigned long nextAutoBlinkTime  = 0;
unsigned long autoBlinkInterval  = 2500;
unsigned long autoBlinkVariation = 3000;
 
bool idleEnabled = false;
unsigned long nextIdleTime  = 0;
unsigned long idleInterval  = 2000;
unsigned long idleVariation = 4000;
 
// =====================================================================
//  DRIVE-MOTION STATE
//  Tracks what the robot's motors are currently doing, so the eyes can
//  react automatically: stopped -> idle drift, forward/backward -> eyes
//  centred, left/right -> eyes look that way (using the exact same
//  eyeX-driven squint the idle system already produces, since both
//  paths go through setEyePosition()/lookLeft()/lookRight()).
//  The FaceMotion enum itself is declared in face.h (public API) --
//  see the note alongside this file for the exact lines to add there.
// =====================================================================
 
static FaceMotion currentMotion = FACE_MOTION_STOP;
static bool motionStateInitialized = false; // forces the first call through
 
// =====================================================================
//  EXPRESSION STATE
// =====================================================================
 
enum FaceExpression { EXPR_NEUTRAL, EXPR_HAPPY };
FaceExpression currentExpression = EXPR_NEUTRAL;
 
// =====================================================================
//  FRAME LIMITER for faceUpdate()
// =====================================================================
 
unsigned long lastFrameTime = 0;
const unsigned long frameInterval = 20; // ~50 fps cap
 
// =====================================================================
//  INTERNAL HELPERS
// =====================================================================
 
// Moves `current` a fraction of the way towards `target`. This is the
// exponential-smoothing / lerp-towards-target technique RoboEyes uses
// (there it's written as current=(current+next)/2 each frame); using an
// explicit factor here just makes the easing speed tunable per-state.
static float easeToward(float current, float target, float factor)
{
    float delta = target - current;
    if (fabs(delta) < 0.05f)
        return target;
    return current + delta * factor;
}
 
static void updateEyePosition()
{
    eyeX = easeToward(eyeX, targetEyeX, POS_EASING);
    eyeY = easeToward(eyeY, targetEyeY, POS_EASING);
}
 
// Advances one eye's blink state by one frame. Handles closing, holding
// shut briefly, and reopening again, all driven by millis() rather than
// delay(), and all interruptible (retriggering just resets the target).
static void updateBlinkEye(float &current, float &target, bool &active, unsigned long &holdUntil)
{
    float factor = (target > current) ? BLINK_CLOSE_EASING : BLINK_OPEN_EASING;
    current = easeToward(current, target, factor);
 
    if (!active)
        return;
 
    if (target > 0 && (eyeHeight - current) < 2.0f) {
        // Eye has reached (near enough) fully closed.
        if (holdUntil == 0)
            holdUntil = millis() + BLINK_HOLD_MS;
        if (millis() >= holdUntil) {
            target = 0;      // start reopening
            holdUntil = 0;
        }
    } else if (target == 0 && current < 0.5f) {
        current = 0;
        active = false;      // blink cycle complete
    }
}
 
static void triggerBlink(bool left, bool right)
{
    if (left) {
        targetBlinkL = eyeHeight;
        blinkActiveL = true;
        blinkHoldUntilL = 0;
    }
    if (right) {
        targetBlinkR = eyeHeight;
        blinkActiveR = true;
        blinkHoldUntilR = 0;
    }
}
 
static void updateFlicker()
{
    unsigned long elapsed = millis() - flickerStartTime;
    int stepIndex = elapsed / flickerStepDuration;
 
    if (stepIndex >= flickerStepCount) {
        flickerActive = false;
        blinkAmountL = 0;
        blinkAmountR = 0;
        targetBlinkL = 0;
        targetBlinkR = 0;
        return;
    }
 
    // Flicker directly drives the current eyelid amount for both eyes;
    // the regular target-based blink easing is skipped while this runs
    // (see faceUpdate()) so the two systems never fight each other.
    blinkAmountL = flickerSteps[stepIndex];
    blinkAmountR = flickerSteps[stepIndex];
}
 
static void updateAutoBlink()
{
    if (!autoBlinkEnabled)
        return;
    if (millis() < nextAutoBlinkTime)
        return;
 
    // Same 70/30 flicker-vs-blink split the original faceNoEmo() used.
    int r = random(100);
    if (r < 70) {
        flicker();
    } else {
        triggerBlink(true, true);
    }
    nextAutoBlinkTime = millis() + autoBlinkInterval + random(autoBlinkVariation);
}
 
static void updateIdle()
{
    if (!idleEnabled)
        return;
    if (millis() < nextIdleTime)
        return;
 
    int rx = random(-LOOK_SIDE_OFFSET, LOOK_SIDE_OFFSET + 1);
    int ry = random(-LOOK_VERTICAL_OFFSET, LOOK_VERTICAL_OFFSET + 1);
    setEyePosition(rx, ry);
 
    nextIdleTime = millis() + idleInterval + random(idleVariation);
}
 
// =====================================================================
//  CHEEKS
// =====================================================================
 
void cheeks()
{
    int ey = (int)eyeY;
    int ex = (int)eyeX;   // shift cheeks together with the eyes so the whole
                          // face reads as turning, not just the eyes sliding
 
    // Left cheek
    display.drawLine(ex + 8 + 2 + 2, ey + eyeHeight + 1, ex + 10 + 2 + 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 12 + 2 + 2, ey + eyeHeight + 1, ex + 14 + 2 + 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 16 + 2 + 2, ey + eyeHeight + 1, ex + 18 + 2 + 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 20 + 2 + 2, ey + eyeHeight + 1, ex + 22 + 2 + 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 24 + 2 + 2, ey + eyeHeight + 1, ex + 26 + 2 + 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 28 + 2 + 2, ey + eyeHeight + 1, ex + 30 + 2 + 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
 
    // Right cheek
    display.drawLine(ex + 98 - 2 - 2, ey + eyeHeight + 1, ex + 102 - 2 - 3, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 102 - 2 - 2, ey + eyeHeight + 1, ex + 104 - 2 - 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 106 - 2 - 2, ey + eyeHeight + 1, ex + 108 - 2 - 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 110 - 2 - 2, ey + eyeHeight + 1, ex + 112 - 2 - 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 114 - 2 - 2, ey + eyeHeight + 1, ex + 116 - 2 - 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
    display.drawLine(ex + 118 - 2 - 2, ey + eyeHeight + 1, ex + 120 - 2 - 2, ey + eyeHeight + 3 + 1, SH110X_WHITE);
}
 
// =====================================================================
//  NEUTRAL EYE RENDERING
// =====================================================================
 
// Immediate one-shot draw using an explicit blink amount. Kept for
// backwards compatibility (e.g. old code calling drawEyes(20) directly);
// it reads the current animated eyeX/eyeY offsets but does not touch the
// ongoing blinkAmountL/R animation state.
void drawEyes(int blinkAmount)
{
    display.clearDisplay();
 
    // The further the eyes glance sideways, the narrower + a little shorter
    // and rounder they get -- a natural-looking squint instead of a flat
    // rectangle just sliding across the screen.
    int shrinkW = min(4, (int)(fabs(eyeX) / 2));
    int shrinkH = min(6, (int)(fabs(eyeX) / 1.7));
    int dynamicRadius = eyeRadius + shrinkW;
 
    int h = eyeHeight - blinkAmount - shrinkH;
    if (h < 2)
        h = 2;
 
    int w = eyeWidth - shrinkW;
    int wOffset = (eyeWidth - w) / 2;
 
    int lx = leftEyeBaseX + (int)eyeX + wOffset;
    int rx = rightEyeBaseX + (int)eyeX + wOffset;
 
    // Close from the centre outward (both eyelids meeting in the middle)
    // rather than the top sliding down to a fixed bottom edge.
    int y = (int)eyeY + (eyeHeight - h) / 2;
 
    display.fillRoundRect(lx, y, w, h, dynamicRadius, SH110X_WHITE);
    display.fillRoundRect(rx, y, w, h, dynamicRadius, SH110X_WHITE);
 
    cheeks();       // drawn before display() so it shows in the same frame
    display.display();
}
 
// Per-frame render used by faceUpdate() while in the neutral expression.
// Uses the independently animated blinkAmountL/blinkAmountR so winks and
// blinks can differ between the two eyes.
static void renderNeutralFace()
{
    display.clearDisplay();
 
    // Eyes narrow and shorten a little the further they look sideways, and
    // pick up a slightly rounder corner -- a natural relaxed squint rather
    // than a flat rectangle sliding across the screen. This is on top of
    // whatever the blink/wink animation is doing to the height.
    int shrinkW = min(4, (int)(fabs(eyeX) / 2));
    int shrinkH = min(6, (int)(fabs(eyeX) / 1.7));
    int dynamicRadius = eyeRadius + shrinkW;
 
    int lHeight = eyeHeight - (int)blinkAmountL - shrinkH;
    int rHeight = eyeHeight - (int)blinkAmountR - shrinkH;
    if (lHeight < 2) lHeight = 2;
    if (rHeight < 2) rHeight = 2;
 
    int w = eyeWidth - shrinkW;
    int wOffset = (eyeWidth - w) / 2;
 
    int lx = leftEyeBaseX + (int)eyeX + wOffset;
    int rx = rightEyeBaseX + (int)eyeX + wOffset;
 
    // Close from the centre outward -- both eyelids meet in the middle of
    // the eye instead of the top sliding down to a fixed bottom edge.
    int ly = (int)eyeY + (eyeHeight - lHeight) / 2;
    int ry = (int)eyeY + (eyeHeight - rHeight) / 2;
 
    display.fillRoundRect(lx, ly, w, lHeight, dynamicRadius, SH110X_WHITE);
    display.fillRoundRect(rx, ry, w, rHeight, dynamicRadius, SH110X_WHITE);
 
    cheeks();
    display.display();
}
 
// =====================================================================
//  MOON EYES / HAPPY EXPRESSION
// =====================================================================
 
// A single genuinely-happy squinting eye: a full circle with everything
// below a thin band near its top masked out in black, leaving a crisp,
// centred, upward-curving crescent sliver -- the classic "^_^" closed
// happy eye, instead of the lopsided slab that used to be cut out here.
static void drawHappySquintEye(int centerX, int centerY)
{
    const int r = 20;             // eye circle radius (unchanged eye size)
    const int arcThickness = 9;   // how tall the visible crescent sliver is
 
    display.fillCircle(centerX, centerY, r, SH110X_WHITE);
 
    int cutY = centerY - r + arcThickness;
    int cutW = 2 * r + 8;                // wider than the circle so the
    int cutX = centerX - cutW / 2;       // cut edges never show, only the
    int cutH = 2 * r;                    // circle's own curve does
    display.fillRect(cutX, cutY, cutW, cutH, SH110X_BLACK);
}
 
// Pure drawing helper — draws into the buffer without clearing or pushing,
// so both moonEyes() (standalone) and the happy-face renderer can reuse it.
static void drawMoonEyesShape()
{
    int lhx = leftEyeBaseX + (int)eyeX + eyeWidth / 2;
    int hy  = (int)eyeY + eyeHeight / 2;
    int rhx = rightEyeBaseX + (int)eyeX + eyeWidth / 2;
 
    drawHappySquintEye(lhx, hy);
    drawHappySquintEye(rhx, hy);
}
 
// A big, bold, FILLED grin: take a large circle and mask off its top half,
// leaving a wide filled bottom half-disk -- much cuter and more visible
// than a thin one-pixel outline arc.
static void drawHappySmile()
{
    const int r  = 20;
    const int cx = 64 + (int)eyeX;   // shifts with the rest of the face
    const int cy = 40;               // flat edge of the grin sits here
 
    display.fillCircle(cx, cy, r, SH110X_WHITE);
    display.fillRect(cx - r - 4, cy - r - 4, 2 * r + 8, r + 4, SH110X_BLACK);
}
 
void moonEyes()
{
    display.clearDisplay();
    drawMoonEyesShape();
    cheeks();
    display.display();
}
 
static void renderHappyFace()
{
    display.clearDisplay();
    drawMoonEyesShape();
    drawHappySmile();
    cheeks();
    display.display();
}
 
void faceHappy()
{
    currentExpression = EXPR_HAPPY;
    renderHappyFace();   // draw immediately so the caller sees it right away;
                          // faceUpdate() will keep re-rendering it each frame
}
 
// =====================================================================
//  NEUTRAL / IDLE EXPRESSION ENTRY POINT
// =====================================================================
 
// Original faceNoEmo() blockingly drew the eyes, delay()'d a random
// 1.5-4s, then rolled a die between flicker()/blink(). The non-blocking
// equivalent is to switch to the neutral expression and turn on the
// auto-blink system with the same timing character (interval + random
// variation) and the same 70/30 flicker/blink split — faceUpdate() then
// keeps that behaviour running continuously instead of one shot at a time.
void faceNoEmo()
{
    currentExpression = EXPR_NEUTRAL;
    setBlinkMode(true, 1500, 2500);
}
 
// =====================================================================
//  BLINK / WINK TRIGGERS  (public API)
// =====================================================================
 
void blink()
{
    triggerBlink(true, true);
}
 
void winkLeft()
{
    triggerBlink(true, false);
}
 
void winkRight()
{
    triggerBlink(false, true);
}
 
// Legacy name — the original Wink() winked the right eye.
void Wink()
{
    winkRight();
}
 
float currentBlink()
{
    return (blinkAmountL + blinkAmountR) / 2.0f;
}
 
float targetBlink()
{
    return (targetBlinkL + targetBlinkR) / 2.0f;
}
 
// =====================================================================
//  FLICKER TRIGGER (public API)
// =====================================================================
 
void flicker()
{
    flickerActive = true;
    flickerStartTime = millis();
}
 
// =====================================================================
//  EYE POSITION / LOOK API
// =====================================================================
 
void setEyePosition(int x, int y)
{
    targetEyeX = x;
    targetEyeY = baseEyeY + y;
}
 
void lookRight()  { setEyePosition(LOOK_SIDE_OFFSET, (int)(targetEyeY - baseEyeY)); }
void lookLeft()   { setEyePosition(-LOOK_SIDE_OFFSET, (int)(targetEyeY - baseEyeY)); }
void lookUp()     { setEyePosition((int)targetEyeX, -LOOK_VERTICAL_OFFSET); }
void lookDown()   { setEyePosition((int)targetEyeX, LOOK_VERTICAL_OFFSET); }
void lookCenter() { setEyePosition(0, 0); }
 
// Legacy names, now thin non-blocking wrappers over the new look API.
// Note the behaviour change: these used to block until the movement (and
// a 180ms hold) finished. Now they just set a target and return
// immediately — call RightCenter()/LeftCenter() (or lookCenter()) whenever
// you're ready to bring the eyes back, which also makes it trivial to
// interrupt a look with another one.
void LookRight()   { lookRight(); }
void RightCenter() { lookCenter(); }
void LookLeft()    { lookLeft(); }
void LeftCenter()  { lookCenter(); }
 
// =====================================================================
//  DRIVE-MOTION -> EYE BEHAVIOUR  (public API)
//
//  Call this once, whenever the robot's drive command CHANGES (i.e. from
//  your motor-control code: right after you tell the motors to go
//  forward/backward/left/right, or to stop). It deliberately does
//  nothing if called again with the same motion, so it's safe to call
//  every loop() iteration if that's easier than tracking "did the
//  command change" yourself.
// =====================================================================
 
void faceSetMotion(FaceMotion motion)
{
    if (motionStateInitialized && motion == currentMotion)
        return;   // command hasn't changed -- don't fight the animation
 
    currentMotion = motion;
    motionStateInitialized = true;
 
    switch (motion) {
        case FACE_MOTION_STOP:
            // Stopped: bring the eyes back to a neutral centre, then hand
            // them over to idle drift so the face stays alive while parked.
            lookCenter();
            setIdleMode(true, idleInterval, idleVariation);
            break;
 
        case FACE_MOTION_FORWARD:
        case FACE_MOTION_BACKWARD:
            // Driving straight: idle drift would look wrong (eyes glancing
            // around while rolling forward), so switch it off and snap the
            // eyes back to centre.
            setIdleMode(false, idleInterval, idleVariation);
            lookCenter();
            break;
 
        case FACE_MOTION_LEFT:
            // Turning: idle drift off (it would otherwise fight this), eyes
            // ease over to the left. renderNeutralFace() already derives the
            // squint/shrink purely from how far eyeX has travelled, so this
            // automatically ends up with the same shape idle-drift glances
            // use -- just driven by a deliberate target instead of a random
            // one, and held there instead of drifting back on its own.
            setIdleMode(false, idleInterval, idleVariation);
            lookLeft();
            break;
 
        case FACE_MOTION_RIGHT:
            setIdleMode(false, idleInterval, idleVariation);
            lookRight();
            break;
    }
}
 
// =====================================================================
//  AUTO-BLINK / IDLE MODE CONFIGURATION (public API)
// =====================================================================
 
void setBlinkMode(bool enabled, unsigned long intervalMs, unsigned long variationMs)
{
    autoBlinkEnabled = enabled;
    autoBlinkInterval = intervalMs;
    autoBlinkVariation = variationMs;
    if (enabled) {
        nextAutoBlinkTime = millis() + intervalMs + random(variationMs > 0 ? variationMs : 1);
    }
}
 
void setIdleMode(bool enabled, unsigned long intervalMs, unsigned long variationMs)
{
    idleEnabled = enabled;
    idleInterval = intervalMs;
    idleVariation = variationMs;
    if (enabled) {
        nextIdleTime = millis() + intervalMs + random(variationMs > 0 ? variationMs : 1);
    }
}
 
// =====================================================================
//  MAIN UPDATE — call this from loop()
// =====================================================================
 
void faceUpdate()
{
    unsigned long now = millis();
    if (now - lastFrameTime < frameInterval)
        return;               // cap the frame rate, keeps loop() responsive
    lastFrameTime = now;
 
    updateAutoBlink();
    updateIdle();
 
    updateEyePosition();
 
    if (flickerActive) {
        updateFlicker();
    } else {
        updateBlinkEye(blinkAmountL, targetBlinkL, blinkActiveL, blinkHoldUntilL);
        updateBlinkEye(blinkAmountR, targetBlinkR, blinkActiveR, blinkHoldUntilR);
    }
 
    if (currentExpression == EXPR_HAPPY) {
        renderHappyFace();
    } else {
        renderNeutralFace();
    }
}
 
// =====================================================================
//  BEGIN
// =====================================================================
 
void faceBegin()
{
    Wire.begin(21,22);
    if(!display.begin(0x3C,true)){
        Serial.println("Display Failed");
        while(1);
    }
    display.clearDisplay();
 
    eyeX = 0;
    eyeY = baseEyeY;
    targetEyeX = 0;
    targetEyeY = baseEyeY;
 
    blinkAmountL = 0;
    blinkAmountR = 0;
    targetBlinkL = 0;
    targetBlinkR = 0;
    blinkActiveL = false;
    blinkActiveR = false;
 
    currentExpression = EXPR_NEUTRAL;
 
    // Robot boots up stationary, so start the eyes in the same idle-drift
    // state faceSetMotion(FACE_MOTION_STOP) would put them in.
    motionStateInitialized = false;
    faceSetMotion(FACE_MOTION_STOP);
 
    renderNeutralFace();
}
 