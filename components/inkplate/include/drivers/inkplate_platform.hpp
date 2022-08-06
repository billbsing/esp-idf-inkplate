/*
inkplate_platform.hpp

Inkplate 6 Arduino library
David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html
Please review the LICENSE file included with this example.
If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#pragma once

#include <cstdint>

#include "non_copyable.hpp"

#include "mcp23017.hpp"
#include "battery.hpp"
#include "eink.hpp"
#include "eink_6.hpp"
#include "eink_10.hpp"
#include "press_keys.hpp"
#include "touch_keys.hpp"
#include "touch_screen.hpp"
#include "backlight.hpp"
#include "PCF85063A.hpp"

#if __INKPLATE_PLATFORM__
  MCP23017  mcp_int(0x20);
  #if defined(CONFIG_INKPLATE_BATTERY)
    Battery   battery(mcp_int);
  #endif
  #if defined(CONFIG_INKPLATE_INPUT_KEYS)
    PressKeys press_keys(mcp_int);
  #endif
  #if defined(CONFIG_INKPLATE_INPUT_TOUCH)
    TouchKeys touch_keys(mcp_int);
  #endif
  #if defined(CONFIG_INKPLATE_BACKLIGHT)
    Backlight backlight(mcp_int);
  #endif

  #if defined(CONFIG_INKPLATE_RTC)
    PCF85063A rtc;
  #endif


  #if defined(CONFIG_INKPLATE_6) || defined(CONFIG_INKPLATE_6_PLUS)
    EInk6     e_ink(mcp_int);
  #elif defined(CONFIG_INKPLATE_10)
    MCP23017  mcp_ext(0x22);
    EInk10    e_ink(mcp_int, mcp_ext);
  #else
    #error "One of CONFIG_INKPLATE_6, CONFIG_INPLATE_6_PLUS, CONFIG_INKPLATE_10 must be defined."
  #endif

  #if defined(CONFIG_INKPLATE_INPUT_TOUCH_SCREEN)
    TouchScreen touch_screen(mcp_int, e_ink);
  #endif


#else
  extern MCP23017  mcp_int;
  #if defined(CONFIG_INKPLATE_BATTERY)
    extern Battery   battery;
  #endif
  #if defined(CONFIG_INKPLATE_INPUT_KEYS)
    extern PressKeys press_keys;
  #endif
  #if defined(CONFIG_INKPLATE_INPUT_TOUCH)
    extern TouchKeys touch_keys;
  #endif
  #if defined(CONFIG_INKPLATE_INPUT_TOUCH_SCREEN)
    extern TouchScreen touch_screen;
  #endif
  #if defined(CONFIG_INKPLATE_BACKLIGHT)
    extern Backlight backlight;
  #endif
  #if defined(CONFIG_INKPLATE_RTC)
  extern PCF85063A rtc;
#endif


  #if defined(CONFIG_INKPLATE_6) || defined(CONFIG_INKPLATE_6_PLUS)
    extern EInk6     e_ink;
  #elif defined(CONFIG_INKPLATE_10)
    extern MCP23017  mcp_ext;
    extern EInk10    e_ink;
  #else
    #error "One of CONFIG_INKPLATE_6, CONFIG_INPLATE_6_PLUS, CONFIG_INKPLATE_10 must be defined."
  #endif
#endif

class InkPlatePlatform : NonCopyable
{
  private:
    static constexpr char const * TAG = "InkPlatePlatform";

    static InkPlatePlatform singleton;
    InkPlatePlatform() {};

  public:
    static inline InkPlatePlatform & get_singleton() noexcept { return singleton; }

    /**
     * @brief Setup the InkPlate Devices
     *
     * This method initialize the SD-Card, the e-Ink display, battery status, and the touchkeys
     * capabilities.
     *
     * @return true - All devices ready
     * @return false - Some device not initialized properly
     */
    bool setup();

    bool light_sleep(uint32_t minutes_to_sleep);
    void deep_sleep();
    #if defined(CONFIG_INKPLATE_RTC)
      void deep_sleep_on_rtc();
      void deep_sleep_for(uint16_t seconds);
      void deep_sleep_until(int8_t second, int8_t minute, int8_t hour, int8_t day, int8_t weekday);
    #endif

};

#if __INKPLATE_PLATFORM__
  InkPlatePlatform & inkplate_platform = InkPlatePlatform::get_singleton();
#else
  extern InkPlatePlatform & inkplate_platform;
#endif
