#include "movement.h"
#include "motor.h"
#include "face.h"

void Stop()
{
  stopMotors();
  faceSetMotion(FACE_MOTION_STOP);
}

void Forward()
{
  moveforward();
  faceSetMotion(FACE_MOTION_FORWARD);
}

void Reverse()
{
  movereverse();
  faceSetMotion(FACE_MOTION_BACKWARD);
}

void Left()
{
  moveleft();
  faceSetMotion(FACE_MOTION_LEFT);
}

void Right()
{
  moveright();
  faceSetMotion(FACE_MOTION_RIGHT);
}

void SpinC()
{
  mspinC();
  faceSetMotion(FACE_MOTION_RIGHT);
}

void SpinCC()
{
  mspinCC();
  faceSetMotion(FACE_MOTION_LEFT);
}

void Explore() 
{
  int choice = random(6);

  switch(choice){

    case 0:
      Forward();
      delay(random(600,1800));
      break;

    case 1:
      Left();
      delay(random(300,800));
      break;

    case 2:
      Right();
      delay(random(300,800));
      break;

    case 3:
      Reverse();
      delay(400);
      break;

    case 4:
     SpinC();
     delay(random(300,800));
     break;
    
    case 5:
    SpinCC();
    delay(random(300,800));
    break;

  }
  Stop();
}