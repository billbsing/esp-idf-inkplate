
#include "backlight.hpp"
#include "wire.hpp"


bool
Backlight::setup()
{
  Wire::enter();
  mcp.set_direction(BACKLIGHT_EN, MCP23017::PinMode::OUTPUT);
  Wire::leave();
  return true;
}

void
Backlight::setBacklight(uint8_t _v)
{
  Wire::enter();
  wire.begin_transmission(0x5C >> 1);
  wire.write(0);
  wire.write(63 - (_v & 0b00111111));
  wire.end_transmission();
  Wire::leave();
}

void
Backlight::backlight(bool _e)
{
  Wire::enter();
  if (_e) {
    mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::HIGH);
  }
  else {
    mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::LOW);
  }
  Wire::leave();
}
