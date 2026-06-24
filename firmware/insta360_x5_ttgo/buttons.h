#pragma once

#include <Arduino.h>

struct ButtonEvents
{
  bool nextPressed = false;
  bool selectPressed = false;
};

class Buttons
{
public:
  void begin();
  ButtonEvents poll();

private:
  bool lastNextStable_ = true;
  bool lastSelectStable_ = true;
  bool lastNextRaw_ = true;
  bool lastSelectRaw_ = true;
  unsigned long nextLastChangeMs_ = 0;
  unsigned long selectLastChangeMs_ = 0;
};
