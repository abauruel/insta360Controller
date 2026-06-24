#pragma once

#include <Arduino.h>

enum class ConnectionState : uint8_t
{
  DISCONNECTED,
  SCANNING,
  PAIRING,
  CONNECTED,
  ERROR
};

enum class CameraMode : uint8_t
{
  UNKNOWN,
  VIDEO,
  PHOTO,
  TIMELAPSE
};

enum class MenuAction : uint8_t
{
  CONNECT,
  RECORD_TOGGLE,
  CAPTURE_PHOTO,
  NEXT_MODE,
  SLEEP,
  WAKE,
  REFRESH_TELEMETRY,
  COUNT
};

struct Telemetry
{
  bool batteryAvailable = false;
  int batteryPercent = 0;

  bool videoRemainingAvailable = false;
  unsigned long videoRemainingSec = 0;

  bool photosRemainingAvailable = false;
  int photosRemaining = 0;
};

struct SystemModel
{
  ConnectionState connectionState = ConnectionState::DISCONNECTED;
  CameraMode cameraMode = CameraMode::UNKNOWN;
  String cameraModeLabel;
  bool isRecording = false;
  bool isSleeping = false;
  bool telemetrySupported = false;
  String feedback = "Pronto";
  Telemetry telemetry;
};

inline const char *connectionToText(ConnectionState state)
{
  switch (state)
  {
  case ConnectionState::DISCONNECTED:
    return "DISC";
  case ConnectionState::SCANNING:
    return "SCAN";
  case ConnectionState::PAIRING:
    return "PAIR";
  case ConnectionState::CONNECTED:
    return "CONN";
  case ConnectionState::ERROR:
    return "ERR";
  }
  return "UNK";
}

inline const char *modeToText(CameraMode mode)
{
  switch (mode)
  {
  case CameraMode::UNKNOWN:
    return "SYNC?";
  case CameraMode::VIDEO:
    return "VIDEO";
  case CameraMode::PHOTO:
    return "PHOTO";
  case CameraMode::TIMELAPSE:
    return "TIMELAPSE";
  }
  return "MODE?";
}

inline const char *actionToText(MenuAction action)
{
  switch (action)
  {
  case MenuAction::CONNECT:
    return "CONNECT";
  case MenuAction::RECORD_TOGGLE:
    return "REC TOGGLE";
  case MenuAction::CAPTURE_PHOTO:
    return "CAPTURE";
  case MenuAction::NEXT_MODE:
    return "NEXT MODE";
  case MenuAction::SLEEP:
    return "SLEEP";
  case MenuAction::WAKE:
    return "WAKE";
  case MenuAction::REFRESH_TELEMETRY:
    return "REFRESH";
  case MenuAction::COUNT:
    return "";
  }
  return "";
}
