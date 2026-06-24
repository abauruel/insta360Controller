#include <Arduino.h>

#include "buttons.h"
#include "camera_protocol.h"
#include "config.h"
#include "controller.h"
#include "model.h"
#include "ui.h"

SystemModel model;
Buttons buttons;
CameraProtocol cameraProtocol;
Controller controller;
Ui ui;

void setup()
{
  Serial.begin(115200);
  delay(100);

  buttons.begin();
  cameraProtocol.begin(&model, REMOTE_IDENTIFIER);
  controller.begin(&buttons, &cameraProtocol, &model);
  ui.begin();

  model.feedback = "Pressione NEXT e SELECT";
}

void loop()
{
  cameraProtocol.tick();
  controller.tick();
  ui.render(model, controller.selectedAction());
}
