#ifndef N64Controller_h
#define N64Controller_h

#include "Controller.h"

class N64Controller : public Controller {
 public:
  N64Controller(InitParams *initParams) : Controller(initParams, 4){};
  void init();
  void getSwitchReport(SwitchReport *switchReport);
  void setRumble(bool rumble);

 private:
  void updateState();
  uint16_t convertToSwitchJoystick(int8_t axisPos, double *minAxis,
                                   double *maxAxis);
  void initController();
  void writeRumble(uint8_t writeVal);
  double _maxX = 0.5;
  double _minX = -0.5;
  double _maxY = 0.5;
  double _minY = -0.5;
  uint16_t timeHeld = 0;
  uint8_t cMode = 0;
  uint8_t allStarsMode = 0;
};

#define N64_JOYSTICK_MAX 0x80
#define N64_C_OFFSET 16

// First byte
#define N64_MASK_A (0x1 << 7)
#define N64_MASK_B (0x1 << 6)
#define N64_MASK_Z (0x1 << 5)
#define N64_MASK_START (0x1 << 4)
#define N64_MASK_RESET (0x1 << 7)
#define N64_MASK_L (0x1 << 5)
#define N64_MASK_R (0x1 << 4)

#define N64_MASK_DPAD_UP (0x1 << 3)
#define N64_MASK_DPAD_RIGHT 0x1
#define N64_MASK_DPAD_DOWN (0x1 << 2)
#define N64_MASK_DPAD_LEFT (0x1 << 1)

#define N64_MASK_C_UP (0x1 << 3)
#define N64_MASK_C_RIGHT 0x1
#define N64_MASK_C_DOWN (0x1 << 2)
#define N64_MASK_C_LEFT (0x1 << 1)

#ifdef SWITCH_BLUETOOTH
#define threeSeconds 600
#else
#define threeSeconds 300
#endif

#endif
