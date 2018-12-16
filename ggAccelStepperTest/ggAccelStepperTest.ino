// Bounce.pde
// -*- mode: C++ -*-
//
// Make a single stepper bounce from one limit to another
//
// Copyright (C) 2012 Mike McCauley
// $Id: Random.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $

#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, 2, 5); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

void setup()
{  
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  // Change these to suit your stepper if you want
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(100000);
  const float steps_per_mm = 4*200*28/22/1.75f;
  stepper.moveTo(10.0*steps_per_mm);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop()
{
    // If at the end of travel go to the other end
    if (stepper.distanceToGo() == 0) {
      //delay(500);
      digitalWrite(13, stepper.currentPosition() >= 0);
      stepper.moveTo(-stepper.currentPosition());
      
    }
    stepper.run();
}
