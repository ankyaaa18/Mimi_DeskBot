#include "motor.h"

// Motor Pins
#define AIN1 25   // left
#define AIN2 26   // left
#define BIN1 27   // right
#define BIN2 14   // right

// PWM Channels
#define CH_AIN1 0
#define CH_AIN2 1
#define CH_BIN1 2
#define CH_BIN2 3

// PWM Settings
#define PWM_FREQ 1000
#define PWM_RES 8
#define MOTOR_SPEED 150

// FORWARD
void moveforward() {

  ledcWrite(CH_AIN1, MOTOR_SPEED);
  ledcWrite(CH_AIN2, 0);

  ledcWrite(CH_BIN1, MOTOR_SPEED);
  ledcWrite(CH_BIN2, 0);

}

// REVERSE
void movereverse() {

  ledcWrite(CH_AIN1, 0);
  ledcWrite(CH_AIN2, MOTOR_SPEED);

  ledcWrite(CH_BIN1, 0);
  ledcWrite(CH_BIN2, MOTOR_SPEED);

}

// RIGHT
void moveright() {

  ledcWrite(CH_AIN1, MOTOR_SPEED);
  ledcWrite(CH_AIN2, 0);

  ledcWrite(CH_BIN1, 0);
  ledcWrite(CH_BIN2, MOTOR_SPEED);

}

// LEFT
void moveleft() {

  ledcWrite(CH_AIN1, 0);
  ledcWrite(CH_AIN2, MOTOR_SPEED);

  ledcWrite(CH_BIN1, MOTOR_SPEED);
  ledcWrite(CH_BIN2, 0);

}

// SPIN CLOCKWISE
void mspinC()
{
    // Left motor forward
    ledcWrite(CH_AIN1, MOTOR_SPEED);
    ledcWrite(CH_AIN2, 0);

    // Right motor reverse
    ledcWrite(CH_BIN1, 0);
    ledcWrite(CH_BIN2, MOTOR_SPEED);
}

// SPIN COUNTERCLOCKWISE
void mspinCC()
{
    // Left motor reverse
    ledcWrite(CH_AIN1, 0);
    ledcWrite(CH_AIN2, MOTOR_SPEED);

    // Right motor forward
    ledcWrite(CH_BIN1, MOTOR_SPEED);
    ledcWrite(CH_BIN2, 0);
}

// STOP
void stopMotors() {

  ledcWrite(CH_AIN1, 0);
  ledcWrite(CH_AIN2, 0);

  ledcWrite(CH_BIN1, 0);
  ledcWrite(CH_BIN2, 0);

}

void motorBegin()
{
  // Configure PWM channels
  ledcSetup(CH_AIN1, PWM_FREQ, PWM_RES);
  ledcSetup(CH_AIN2, PWM_FREQ, PWM_RES);
  ledcSetup(CH_BIN1, PWM_FREQ, PWM_RES);
  ledcSetup(CH_BIN2, PWM_FREQ, PWM_RES);

  // Attach pins
  ledcAttachPin(AIN1, CH_AIN1);
  ledcAttachPin(AIN2, CH_AIN2);
  ledcAttachPin(BIN1, CH_BIN1);
  ledcAttachPin(BIN2, CH_BIN2);

  stopMotors();
}