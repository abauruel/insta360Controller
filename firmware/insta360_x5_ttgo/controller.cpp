#include "controller.h"

void Controller::begin(Buttons *buttons, CameraProtocol *cameraProtocol, SystemModel *model)
{
  buttons_ = buttons;
  cameraProtocol_ = cameraProtocol;
  model_ = model;
}

void Controller::tick()
{
  if (!buttons_ || !cameraProtocol_ || !model_)
  {
    return;
  }

  const ButtonEvents events = buttons_->poll();

  if (events.nextPressed)
  {
    uint8_t next = static_cast<uint8_t>(selectedAction_);
    next = (next + 1) % static_cast<uint8_t>(MenuAction::COUNT);
    selectedAction_ = static_cast<MenuAction>(next);
    model_->feedback = String("Selecionado: ") + actionToText(selectedAction_);
  }

  if (events.selectPressed)
  {
    const bool ok = cameraProtocol_->execute(selectedAction_);
    if (!ok && model_->feedback.length() == 0)
    {
      model_->feedback = "Falha ao executar";
    }
  }
}
