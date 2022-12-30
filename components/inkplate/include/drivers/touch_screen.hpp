
#pragma once

#include "mcp23017.hpp"
#include "eink.hpp"

#define TOUCH_SCREEN_ADDRESS        0x15
#define TOUCHSCREEN_GPIO_INT        GPIO_NUM_36          // 36

#define BOUND(_a, _x, _b)       ((_a < _x) && (_x < _b))

class TouchScreen
{
  public:
    TouchScreen(MCP23017 &mcp, EInk &eInk) : _mcp(mcp), _eInk(eInk) {}
    bool setup(uint8_t);

    bool touchInArea(int16_t, int16_t, int16_t, int16_t);
    void hardwareReset();
    bool softwareReset();
    bool init(uint8_t);
    void shutdown();
    void getRawData(uint8_t *);
    void getXY(uint8_t *, uint16_t *, uint16_t *);
    uint8_t getData(uint16_t *, uint16_t *);
    void getResolution(uint16_t *, uint16_t *);
    void setPowerState(uint8_t);
    uint8_t getPowerState();
    bool isAvailable();

  protected:
      void readRegs(uint8_t, uint8_t *, uint8_t);
      uint8_t writeRegs(uint8_t, const uint8_t *, uint8_t);

  private:
    MCP23017 & _mcp;
    EInk & _eInk;

    uint8_t _touchData;
    uint16_t _touchX[2];
    uint16_t _touchY[2];
    uint64_t _touchTime = 0;
    const MCP23017::Pin TOUCHSCREEN_EN = MCP23017::Pin::IOPIN_12;

    const MCP23017::Pin TOUCHSCREEN_RTS = MCP23017::Pin::IOPIN_10;
};
