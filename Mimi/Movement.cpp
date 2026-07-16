#include "movement.h"
#include "motor.h"
#include "face.h"

void Stop()
{
  stopMotors();
  faceNoEmo();
}

void Forward()
{
  moveforward();
  faceHappy();
}

void Reverse()
{
  movereverse();
  faceHappy();
}

void Left()
{
  moveleft();
  LookLeft();

  // if(stopMotors())    // ***On hold
  // LeftCentre();

}

void Right()
{
  moveright();
  LookRight();

  // if(stopMotors())    // ***On hold
  // RightCentre();

}

void SpinC()
{
  mspinC();
  faceHappy();
}

void SpinCC()
{
  mspinCC();
  faceHappy();
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