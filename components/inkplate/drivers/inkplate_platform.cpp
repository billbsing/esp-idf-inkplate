#define __INKPLATE_PLATFORM__ 1
#include "inkplate_platform.hpp"

#include "driver/rtc_io.h"
#include "logging.hpp"

#include "wire.hpp"
#include "mcp23017.hpp"
#include "esp.hpp"
#include "eink.hpp"
#include "eink_6.hpp"
#include "battery.hpp"
#include "sd_card.hpp"
#include "press_keys.hpp"
#include "touch_keys.hpp"
#include "backlight.hpp"

#include "esp_sleep.h"

InkPlatePlatform InkPlatePlatform::singleton;

bool
InkPlatePlatform::setup()
{
  wire.setup();

  // Setup the display
  if (!e_ink.setup()) return false;

  #if defined(CONFIG_INKPLATE_BATTERY)
    // Battery
    if (!battery.setup()) return false;
  #endif

  #if defined(CONFIG_INKPLATE_INPUT_KEYS)
    // Setup Press keys
    if (!press_keys.setup()) return false;
  #endif

  #if defined(CONFIG_INKPLATE_INPUT_TOUCH)
    // Setup Touch keys
    if (!touch_keys.setup()) return false;
  #endif

  #if defined(CONFIG_INKPLATE_INPUT_TOUCH_SCREEN)
    // Setup Touch screen
    if (!touch_screen.setup(1)) return false;
  #endif

  #if defined(CONFIG_INKPLATE_BACKLIGHT)
    // Setup backlight
    if (!backlight.setup()) return false;
  #endif

  #if defined(CONFIG_INKPLATE_SD_CARD)
    // Mount and check the SD Card
    SDCard::mount();
  #endif

  // Good to go
  return true;
}

/**
 * @brief Start light sleep
 *
 * Will be waken up at the end of the time requested or when a key is pressed.
 *
 * @param minutes_to_sleep Wait time in minutes
 * @return true The timer when through the end
 * @return false A key was pressed
 */
bool
InkPlatePlatform::light_sleep(uint32_t minutes_to_sleep)
{
  esp_err_t err;

  if ((err = esp_sleep_enable_timer_wakeup(minutes_to_sleep * 60e6)) != ESP_OK) {
    LOG_E("Unable to program Light Sleep wait time: %d", err);
  }
  else if ((err = esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1)) != ESP_OK) {
    LOG_E("Unable to set ext0 WakeUp for Light Sleep: %d", err);
  }
  else if ((err = esp_light_sleep_start()) != ESP_OK) {
    LOG_E("Unable to start Light Sleep mode: %d", err);
  }

  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
}

void
InkPlatePlatform::deep_sleep()
{
  esp_err_t err;

  if ((err = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER)) != ESP_OK) {
    LOG_E("Unable to disable Sleep wait time: %d", err);
  }
  if ((err = esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1)) != ESP_OK) {
    LOG_E("Unable to set ext0 WakeUp for Deep Sleep: %d", err);
  }
  esp_deep_sleep_start();
}


#if defined(CONFIG_INKPLATE_RTC)
void
InkPlatePlatform::deep_sleep_on_rtc() {
    esp_err_t err;
    if ((err = esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0)) != ESP_OK) {
      LOG_E("Unable to set ext1 WakeUp for Deep Sleep: %d", err);
    }
    esp_deep_sleep_start();
}

void
InkPlatePlatform::deep_sleep_for(uint16_t seconds)
{
    enum PCF85063A::CountdownSrcClock sourceClock = PCF85063A::TIMER_CLOCK_1HZ;
    if ( seconds > 255) {
        seconds = seconds / 60;
        sourceClock = PCF85063A::TIMER_CLOCK_1PER60HZ;
    }
    rtc.timerSet(sourceClock, seconds, true, false);
    deep_sleep_on_rtc();
}
void
InkPlatePlatform::deep_sleep_until(int8_t second, int8_t minute, int8_t hour, int8_t day, int8_t weekday)
{
    rtc.setAlarm(second, minute, hour, day, weekday);
    deep_sleep_on_rtc();
}
#endif
