/*
   Basic_partial_update example for e-radionica Inkplate 6
   For this example you will need only USB cable and Inkplate 6
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   In this example we will show  how to use partial update functionality of Inkplate 6 e-paper display.
   It will scroll text that is saved in char array
   NOTE: Partial update is only available on 1 Bit mode (BW) and it is not recommended to use it on first refresh after
   power up. It is recommended to do a full refresh every 5-10 partial refresh to maintain good picture quality.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by e-radionica.com
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging.hpp"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#include "inkplate.hpp"            //Include Inkplate library to the sketch
Inkplate display(DisplayMode::INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1-bit mode (BW)

static const char * TAG = "Main";

// Char array where you can store your text that will be scrolled.
#if defined(CONFIG_INKPLATE_6)
  const char text[] = "Clock test for Inkplate 6 e-paper display! :)";
  int max = 9;
#elif defined(CONFIG_INKPLATE_6_PLUS)
  const char text[] = "TClock test for Inkplate 6PLUS e-paper display! :)";
  int max = 30;
#else
  const char text[] = "Clock test for Inkplate 10 e-paper display! :)";
  int max = 50;
#endif

// This variable is used for moving the text (scrolling)
int offset;
int w, h;

void delay(int msec) { vTaskDelay(msec / portTICK_PERIOD_MS); }

void debug_time(struct tm *now) {
    ESP_LOGI(TAG, "Time: %d-%d-%d %d:%d:%d", now->tm_mday, now->tm_mon, now->tm_year + 1900,
        now->tm_hour, now->tm_min, now->tm_sec);
}
//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
    esp_sleep_source_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason)
    {
        case 1  : ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_IO"); break;
        case 2  : ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_CNTL"); break;
        case 3  : ESP_LOGI(TAG, "Wakeup caused by timer"); break;
        case 4  : ESP_LOGI(TAG, "Wakeup caused by touchpad"); break;
        case 5  : ESP_LOGI(TAG, "Wakeup caused by ULP program"); break;
        default : ESP_LOGI(TAG, "Wakeup was not caused by deep sleep"); break;
    }
}

// Variable that keeps count on how much screen has been partially updated
int n = 0;
void mainTask(void * param)
{

    display.begin();                    // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay();             // Clear frame buffer of display
    display.display();                  // Put clear image on display

    w = display.width();
    h = display.height();


    offset = w;

    print_wakeup_reason();
    ESP_LOGI(TAG, "Display size: width: %d, height: %d", w, h);

    display.setTextColor(BLACK, WHITE); // Set text color to be black and background color to be white
    display.setTextSize(3);             // Set text to be 4 times bigger than classic 5x7 px text
    display.setTextWrap(true);         // Disable text wraping
    display.setCursor(10, h / 2); // Set new position for text
    display.print(text);            // Write text at new position
    display.display();

    struct tm now;
    now.tm_year = 2021 - 1900;
    now.tm_mon = 12;
    now.tm_mday = 16;
    now.tm_hour = 19;
    now.tm_min = 24;
    now.tm_sec = 0;
    // rtc.set_time(&now);


    rtc_gpio_init(GPIO_NUM_39);
    rtc_gpio_set_direction(GPIO_NUM_39, RTC_GPIO_MODE_INPUT_ONLY);

    /*
    rtc.set_alarm(-1, -1, -1, -1, -1);
    if (!rtc.set_countdown(true, RTC::CNTDOWN_CLOCK_1HZ, 8, true, false) ){
        LOG_E("Unable to set countdown");
    }
    */
    rtc.setAlarm(2, -1, -1, -1, -1);

    for (uint8_t index = 0; index < 65; index ++) {
        rtc.readDateTime(&now);
        debug_time(&now);
        ESP_LOGI(TAG, "Time flag: %d", rtc_gpio_get_level(GPIO_NUM_39));
        delay(1000);
    }

    rtc.readDateTime(&now);
    debug_time(&now);


    ESP_LOGI(TAG, "deep sleep for 10 seconds");
//    inkplate_platform.deep_sleep_for(10);
    ESP_LOGI(TAG, "deep sleep end!");
    delay(20000);   // Delay between refreshes.
}


#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"
