#pragma once

#include <Arduino.h>

#include "model.h"

class CameraProtocol
{
public:
  void begin(SystemModel *model, const char *remoteIdentifier);
  void tick();
  bool execute(MenuAction action);

private:
  enum class Backend : uint8_t
  {
    SIMULATED,
    BLE_STUB
  };

  enum class CommandId : uint8_t
  {
    RECORD_TOGGLE,
    CAPTURE_PHOTO,
    NEXT_MODE,
    SLEEP,
    WAKE,
    REFRESH_TELEMETRY
  };

  void startConnectFlow();
  void cycleMode();
  void updateMockTelemetry();
  bool dispatchCommand(CommandId command);
  bool dispatchSimulated(CommandId command);
  bool dispatchBleStub(CommandId command);
  bool sendBleCommandWithAck(const uint8_t *payload, size_t payloadLen, uint8_t ackOpcode, const char *label);
  bool bleEnsureConnected();
  bool bleDiscoverCharacteristics();
  bool bleHasConfiguredUuids() const;
  void bleDisconnect();

  SystemModel *model_ = nullptr;
  const char *remoteIdentifier_ = nullptr;

  Backend backend_ = Backend::SIMULATED;

  bool connectInProgress_ = false;
  unsigned long connectStartMs_ = 0;
  bool bleInitialized_ = false;

  unsigned long lastTelemetryTickMs_ = 0;
};
