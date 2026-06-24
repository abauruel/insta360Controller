#pragma once

#include "buttons.h"
#include "camera_protocol.h"
#include "model.h"

class Controller
{
public:
  void begin(Buttons *buttons, CameraProtocol *cameraProtocol, SystemModel *model);
  void tick();

  MenuAction selectedAction() const { return selectedAction_; }

private:
  Buttons *buttons_ = nullptr;
  CameraProtocol *cameraProtocol_ = nullptr;
  SystemModel *model_ = nullptr;

  MenuAction selectedAction_ = MenuAction::CONNECT;
};
