#include "buttons.h"

#include "config.h"

static void setupButtonPin(int pin)
{
  // GPIOs 34..39 no ESP32 sao somente entrada e nao suportam pull-up interno.
  if (pin >= 34)
  {
    pinMode(pin, INPUT);
  }
  else
  {
    pinMode(pin, INPUT_PULLUP);
  }
}

void Buttons::begin()
{
  setupButtonPin(BTN_NEXT_PIN);
  setupButtonPin(BTN_SELECT_PIN);

  lastNextRaw_ = digitalRead(BTN_NEXT_PIN);
  lastSelectRaw_ = digitalRead(BTN_SELECT_PIN);
  lastNextStable_ = lastNextRaw_;
  lastSelectStable_ = lastSelectRaw_;
}

ButtonEvents Buttons::poll()
{
  ButtonEvents events;
  const unsigned long now = millis();

  const bool rawNext = digitalRead(BTN_NEXT_PIN);
  if (rawNext != lastNextRaw_)
  {
    lastNextRaw_ = rawNext;
    nextLastChangeMs_ = now;
  }
  if ((now - nextLastChangeMs_) >= BUTTON_DEBOUNCE_MS && rawNext != lastNextStable_)
  {
    lastNextStable_ = rawNext;
    if (lastNextStable_ == LOW)
    {
      events.nextPressed = true;
    }
  }

  const bool rawSelect = digitalRead(BTN_SELECT_PIN);
  if (rawSelect != lastSelectRaw_)
  {
    lastSelectRaw_ = rawSelect;
    selectLastChangeMs_ = now;
  }
  if ((now - selectLastChangeMs_) >= BUTTON_DEBOUNCE_MS && rawSelect != lastSelectStable_)
  {
    lastSelectStable_ = rawSelect;
    if (lastSelectStable_ == LOW)
    {
      events.selectPressed = true;
    }
  }

  return events;
}
