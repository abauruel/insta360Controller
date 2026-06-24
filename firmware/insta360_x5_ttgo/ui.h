#pragma once

#include <Arduino.h>

#include "model.h"

class Ui
{
public:
  void begin();
  void render(const SystemModel &model, MenuAction selectedAction);

private:
  unsigned long lastDrawMs_ = 0;
};
