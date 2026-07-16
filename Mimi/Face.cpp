#include "face.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Eye settings
const int eyeWidth = 36;
const int eyeHeight = 40;
const int eyeRadius = 8;
int leftEyeX = 18;
int rightEyeX = 74;
int eyeY = 8;

// CHEEKS
void cheeks()
{
    // Left cheek
    display.drawLine(8 + 2 + 2,eyeY + eyeHeight + 1,10 + 2 + 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(12 + 2 + 2,eyeY + eyeHeight + 1,14 + 2 + 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(16 + 2 + 2,eyeY + eyeHeight + 1,18 + 2 + 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(20 + 2 + 2,eyeY + eyeHeight + 1,22 + 2 + 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(24 + 2 + 2,eyeY + eyeHeight + 1,26 + 2 + 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(28 + 2 + 2,eyeY + eyeHeight + 1,30 + 2 + 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);

    // Right cheek
    display.drawLine(98 - 2 - 2,eyeY + eyeHeight + 1,102 - 2 - 3,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(102 - 2 - 2,eyeY + eyeHeight + 1,104 - 2 - 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(106 - 2 - 2,eyeY + eyeHeight + 1,108 - 2 - 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(110 - 2 - 2,eyeY + eyeHeight + 1,112 - 2 - 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(114 - 2 - 2,eyeY + eyeHeight + 1,116 - 2 - 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
    display.drawLine(118 - 2 - 2,eyeY + eyeHeight + 1,120 - 2 - 2,eyeY + eyeHeight + 3 + 1,SSD1306_WHITE);
}

// ------------------------
// Draw Eyes
// ------------------------
void drawEyes(int blinkAmount)
{
    display.clearDisplay();

    int currentHeight = eyeHeight - blinkAmount;
    // Minimum eye height when blinking
    if (currentHeight < 2)
        currentHeight = 2;

    // Anchor the bottom edge so the eye closes from the top down,
    // meeting at the bottom instead of shrinking toward the center.
    int y = eyeY + (eyeHeight - currentHeight);

    // Draw plain white eye shapes (base for every expression)
    display.fillRoundRect(leftEyeX, y,
                          eyeWidth, currentHeight,
                          eyeRadius, SSD1306_WHITE);
    display.fillRoundRect(rightEyeX, y,
                          eyeWidth, currentHeight,
                          eyeRadius, SSD1306_WHITE);

    display.display();
    cheeks();
}

// ------------------------
// BLINK
// ------------------------
void blink()
{
    for (int i = 0; i <= 38; i += 2)
    {
        drawEyes(i);
        delay(6);      // Fast closing
    }
    for (int i = 38; i >= 0; i -= 2)
    {
        drawEyes(i);
        delay(12);     // Slower opening
    }
}

// ------------------------
//  WINK
// ------------------------
void Wink()
{
    for (int blink = 0; blink <= 38; blink += 2)
    {
        display.clearDisplay();

        // Left eye stays open
        // Left eye (slightly squinted)
        display.fillRoundRect(leftEyeX,eyeY + 2,eyeWidth,eyeHeight - 4,eyeRadius + 3,SSD1306_WHITE);

        // Right eye closes
        int currentHeight = eyeHeight - blink;
        if (currentHeight < 2)
            currentHeight = 2;

        int y = eyeY + (eyeHeight - currentHeight);

        display.fillRoundRect(rightEyeX, y,
                              eyeWidth, currentHeight,
                              eyeRadius, SSD1306_WHITE);

        cheeks();
        display.display();
        delay(3);
    }

    delay(40);   // Wink hold

    for (int blink = 38; blink >= 0; blink -= 2)
    {
        display.clearDisplay();

        // Left eye stays open
        display.fillRoundRect(leftEyeX, eyeY,
                              eyeWidth, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        // Right eye opens
        int currentHeight = eyeHeight - blink;
        if (currentHeight < 2)
            currentHeight = 2;

        int y = eyeY + (eyeHeight - currentHeight);

        display.fillRoundRect(rightEyeX, y,
                              eyeWidth, currentHeight,
                              eyeRadius, SSD1306_WHITE);

        cheeks();
        display.display();
        delay(6);
    }
    drawEyes();      // Return to current expression
}

// ------------------------
// MOON EYES
// ------------------------
void moonEyes()
{
    display.clearDisplay();
    int lhx = leftEyeX + eyeWidth / 2;
    int hy = eyeY + eyeHeight / 2;
    int rhx = rightEyeX + eyeWidth / 2;   

    display.fillCircle(lhx, hy, 20, SSD1306_WHITE);
    display.fillRoundRect(leftEyeX, hy - 5, 35, 30, eyeRadius, SSD1306_BLACK);

    display.fillCircle(rhx, hy, 20, SSD1306_WHITE);
    display.fillRoundRect(rightEyeX, hy - 5, 35, 30, eyeRadius, SSD1306_BLACK);

    display.display();
    cheeks();
}

// ------------------------
// NEUTRAL
// ------------------------
void faceNoEmo()
{
    display.clearDisplay();
    drawEyes();
    delay(random(1500,4000));
    int r = random(100);
    if(r < 70)
    {
        flicker();
    } else {
        blink();
    }
}

// ------------------------
// HAPPY
// ------------------------
void faceHappy() 
{
    display.clearDisplay();
    moonEyes();
    display.drawCircleHelper(64, 48, 15, 0xC, SSD1306_WHITE);
}

// ------------------------
// LOOK RIGHT
// ------------------------
void LookRight()
{
    const int move = 8;

    for (int i = 0; i <= move; i++)
    {
        display.clearDisplay();

        // Eye becomes slightly narrower as it moves
        int w = eyeWidth - (i / 2);   // 36 -> 32
        int offset = (eyeWidth - w) / 2;

        display.fillRoundRect(leftEyeX + i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        display.fillRoundRect(rightEyeX + i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        cheeks();
        display.display();
        delay(10);
    }
    delay(180);
}

void RightCenter()
{
    const int move = 8;
    for (int i = move; i >= 0; i--)
    {
        display.clearDisplay();

        int w = eyeWidth - (i / 2);
        int offset = (eyeWidth - w) / 2;

        display.fillRoundRect(leftEyeX + i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        display.fillRoundRect(rightEyeX + i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        cheeks();
        display.display();
        delay(10);
    }
    drawEyes();
}

// ------------------------
// LOOK LEFT
// ------------------------
void LookLeft()
{
    const int move = 8;

    for (int i = 0; i <= move; i++)
    {
        display.clearDisplay();

        int w = eyeWidth - (i / 2);
        int offset = (eyeWidth - w) / 2;

        display.fillRoundRect(leftEyeX - i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        display.fillRoundRect(rightEyeX - i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        cheeks();
        display.display();
        delay(10);
    }

    delay(180);
}

void LeftCenter()
{
    const int move = 8;
    for (int i = move; i >= 0; i--)
    {
        display.clearDisplay();

        int w = eyeWidth - (i / 2);
        int offset = (eyeWidth - w) / 2;

        display.fillRoundRect(leftEyeX - i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        display.fillRoundRect(rightEyeX - i + offset, eyeY,
                              w, eyeHeight,
                              eyeRadius, SSD1306_WHITE);

        cheeks();
        display.display();
        delay(10);
    }

    drawEyes();
}

// ------------------------
// FLICKER
// ------------------------
void flicker()
{
    drawEyes(1);
    delay(15);
    drawEyes(3);
    delay(15);
    drawEyes(5);
    delay(15);
    drawEyes(3);
    delay(15);
    drawEyes(1);
    delay(15);
    drawEyes(0);
}

void faceBegin()
{
    Wire.begin();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    drawEyes();
    cheeks();
}