#pragma once
#include "Arduino.h"

struct Servo {
  int angle = 90;
  void attach(int) {}
  void write(int a) { angle = a; }
  int read() const { return angle; }
};
