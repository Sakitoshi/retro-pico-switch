#include "N64Controller.h"

#include <string.h>
#include <cmath>

#include "Controller.pio.h"
#include "pico/stdlib.h"

void N64Controller::init() {
  updateState();
  initController();
  writeRumble(0xee);
  writeRumble(0x80);
  setRumble(false);
}

void N64Controller::initController() {
  uint8_t request[1] = {0x00};
  uint8_t response[3];
  transfer(request, sizeof(request), response, sizeof(response));
}

void N64Controller::updateState() {
  uint8_t request[1] = {0x01};
  transfer(request, sizeof(request), _controllerState, _sizeofControllerState);
}

void N64Controller::writeRumble(uint8_t writeVal) {
  uint8_t writeRequest[35];
  writeRequest[0] = 0x03;
  writeRequest[1] = 0x80;
  writeRequest[2] = 0x01;
  memset(writeRequest + 3, writeVal, 32);
  uint8_t writeResponse[1];
  transfer(writeRequest, sizeof(writeRequest), writeResponse,
           sizeof(writeResponse));

  uint8_t readRequest[3] = {0x02, 0x80, 0x01};
  uint8_t readResponse[32];
  transfer(readRequest, sizeof(readRequest), readResponse,
           sizeof(readResponse));
}

void N64Controller::setRumble(bool rumble) {
  _rumble = rumble;
  uint8_t request[35];
  request[0] = 0x03;
  request[1] = 0xc0;
  request[2] = 0x1b;
  memset(request + 3, _rumble ? 0x01 : 0x00, 32);
  uint8_t response[1];
  transfer(request, sizeof(request), response, sizeof(response));
}

void N64Controller::getSwitchReport(SwitchReport *switchReport) {
  initController();
  updateState();

  if (N64_MASK_START & _controllerState[0] && N64_MASK_C_DOWN & _controllerState[1] && N64_MASK_C_LEFT & _controllerState[1]) {
      if (timeHeld == threeSeconds && cMode == 0) cMode = 1;
      timeHeld++;
  } else if (N64_MASK_START & _controllerState[0] && N64_MASK_C_UP & _controllerState[1] && N64_MASK_C_RIGHT & _controllerState[1]) {
      if (timeHeld == threeSeconds && cMode == 1) cMode = 0;
      timeHeld++;
  } else if (N64_MASK_START & _controllerState[0] && N64_MASK_C_DOWN & _controllerState[1] && N64_MASK_C_RIGHT & _controllerState[1]) {
      if (timeHeld == threeSeconds && allStarsMode == 0) allStarsMode = 1;
      timeHeld++;
  } else if (N64_MASK_START & _controllerState[0] && N64_MASK_C_UP & _controllerState[1] && N64_MASK_C_LEFT & _controllerState[1]) {
      if (timeHeld == threeSeconds && allStarsMode == 1) allStarsMode = 0;
      timeHeld++;
  } else if (N64_MASK_START & _controllerState[0] && N64_MASK_DPAD_UP & _controllerState[0]) {
      if (timeHeld == threeSeconds && stickMode == 0) stickMode = 1;
      timeHeld++;
  } else if (N64_MASK_START & _controllerState[0] && N64_MASK_DPAD_DOWN & _controllerState[0]) {
      if (timeHeld == threeSeconds && stickMode == 1) stickMode = 0;
      timeHeld++;
  } else {
     timeHeld = 0;
  }

  switchReport->buttons[0] =
      (N64_MASK_R & _controllerState[1] ? SWITCH_MASK_R : 0) |
      (N64_MASK_A & _controllerState[0] ? SWITCH_MASK_A : 0) |
      (N64_MASK_B & _controllerState[0] && allStarsMode == 0 ? SWITCH_MASK_B : 0) |
      (N64_MASK_C_DOWN & _controllerState[1] && cMode == 1 && allStarsMode == 0 ? SWITCH_MASK_X : 0) |
      (N64_MASK_C_LEFT & _controllerState[1] && cMode == 1 ? SWITCH_MASK_Y : 0) |
      (N64_MASK_C_UP & _controllerState[1] && N64_MASK_C_DOWN & _controllerState[1] && cMode == 0 && allStarsMode == 0 ? SWITCH_MASK_X : 0) |
      (N64_MASK_C_LEFT & _controllerState[1] && N64_MASK_C_RIGHT & _controllerState[1] && cMode == 0 ? SWITCH_MASK_Y : 0) |
      (N64_MASK_B & _controllerState[0] && allStarsMode == 1 ? SWITCH_MASK_X : 0) |
      (N64_MASK_C_DOWN & _controllerState[1] && cMode == 1 && allStarsMode == 1 ? SWITCH_MASK_B : 0) |
      (N64_MASK_C_UP & _controllerState[1] && N64_MASK_C_DOWN & _controllerState[1] && cMode == 0 && allStarsMode == 1 ? SWITCH_MASK_B : 0);

  switchReport->buttons[1] =
      (N64_MASK_RESET & _controllerState[1] ? SWITCH_MASK_HOME : 0) |
      (N64_MASK_START & _controllerState[0] ? SWITCH_MASK_PLUS : 0) |
      (N64_MASK_L & _controllerState[1] && N64_MASK_R & _controllerState[1] && N64_MASK_Z & _controllerState[0] ? SWITCH_MASK_MINUS : 0) |
      (N64_MASK_C_UP & _controllerState[1] && cMode == 1 ? SWITCH_MASK_L3 : 0) |
      (N64_MASK_C_RIGHT & _controllerState[1] && cMode == 1 ? SWITCH_MASK_R3 : 0);

  switchReport->buttons[2] =
      (N64_MASK_L & _controllerState[1] ? SWITCH_MASK_L : 0) |
      (N64_MASK_Z & _controllerState[0] ? SWITCH_MASK_ZL : 0) |
      (N64_MASK_DPAD_UP & _controllerState[0] ? SWITCH_HAT_UP : 0) |
      (N64_MASK_DPAD_DOWN & _controllerState[0] ? SWITCH_HAT_DOWN : 0) |
      (N64_MASK_DPAD_LEFT & _controllerState[0] ? SWITCH_HAT_LEFT : 0) |
      (N64_MASK_DPAD_RIGHT & _controllerState[0] ? SWITCH_HAT_RIGHT : 0);

  // Scale for joystick insensitivity if needed
  uint16_t lx = convertToSwitchJoystick(_controllerState[2], &_minX, &_maxX);
  uint16_t ly = convertToSwitchJoystick(_controllerState[3], &_minY, &_maxY);
  switchReport->l[0] = lx & 0xff;
  switchReport->l[1] = ((ly & 0xff) << 4) | (lx >> 8);
  switchReport->l[2] = ly >> 4;

  uint16_t rx;
  uint16_t ry;
  rx =
      (N64_MASK_C_RIGHT & _controllerState[1] && cMode == 0 ? SWITCH_JOYSTICK_MAX : SWITCH_JOYSTICK_MIN) |
      (N64_MASK_C_LEFT & _controllerState[1] && cMode == 0 ? SWITCH_JOYSTICK_MIN : SWITCH_JOYSTICK_MID);
  ry =
      (N64_MASK_C_UP & _controllerState[1] && cMode == 0 ? SWITCH_JOYSTICK_MAX : SWITCH_JOYSTICK_MIN) |
      (N64_MASK_C_DOWN & _controllerState[1] && cMode == 0 ? SWITCH_JOYSTICK_MIN : SWITCH_JOYSTICK_MID);
  switchReport->r[0] = rx & 0xff;
  switchReport->r[1] = ((ry & 0xff) << 4) | (rx >> 8);
  switchReport->r[2] = ry >> 4;

  return;
}

uint16_t N64Controller::convertToSwitchJoystick(int8_t axisPos, double *minAxis,
                                                double *maxAxis) {
  double unscaledAxisPos = axisPos / (double)N64_JOYSTICK_MAX;
  double scaledAxisPos = getScaledAnalogAxis(unscaledAxisPos, minAxis, maxAxis);
  double recalcAxisPos;
  recalcAxisPos = (stickMode == 0 ? scaledAxisPos : 
                  (scaledAxisPos >= 0.005 ? copysign(pow(fabs(scaledAxisPos), 0.6), scaledAxisPos) : scaledAxisPos));
  return recalcAxisPos * SWITCH_JOYSTICK_MID + SWITCH_JOYSTICK_MID + 1;
}
