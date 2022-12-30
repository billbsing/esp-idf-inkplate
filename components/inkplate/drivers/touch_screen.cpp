/**
 **************************************************
 * @file        Touch.h
 * @brief       Touch screen functionality for panels that support touch
 *
 *              https://github.com/e-radionicacom/Inkplate-Arduino-library
 *              For support, please reach over forums: forum.e-radionica.com/en
 *              For more info about the product, please check: www.inkplate.io
 *
 *              This code is released under the GNU Lesser General Public
 *License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html Please review the
 *LICENSE file included with this example. If you have any questions about
 *licensing, please contact techsupport@e-radionica.com Distributed as-is; no
 *warranty is given.
 *
 * @authors     @ e-radionica.com
 ***************************************************/

#include <esp_log.h>
#include "touch_screen.hpp"
#include "wire.hpp"
#include "graphics.hpp"


uint16_t _tsXResolution;
uint16_t _tsYResolution;

static const char *TAG = "touch_screen";

static volatile bool _tsFlag = false;

static void IRAM_ATTR touchscreenInterrupt(void * arg)
{
    _tsFlag = true;
}


/**
 * @brief       tsInit starts touchscreen and sets ts registers
 *
 * @param       uint8_t pwrState
 *              power state for touchScreen
 */
bool TouchScreen::setup(uint8_t pwrState)
{
//    Wire::enter();
//    Wire::leave();
    _mcp.set_direction(TOUCHSCREEN_EN, MCP23017::PinMode::OUTPUT);

    // Enable power to TS

    _mcp.digital_write(TOUCHSCREEN_EN, MCP23017::SignalLevel::LOW);
    // digitalWriteInternal(MCP23017_INT_ADDR, mcpRegsInt, TOUCHSCREEN_EN, LOW);

    _mcp.set_direction(TOUCHSCREEN_RTS, MCP23017::PinMode::OUTPUT);
    // pinModeInternal(MCP23017_INT_ADDR, mcpRegsInt, TOUCHSCREEN_RTS, OUTPUT);


    /*
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << TOUCHSCREEN_GPIO_INT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&gpio_conf);
    */
    //gpio_install_isr_service();
    gpio_intr_disable(TOUCHSCREEN_GPIO_INT);
    gpio_set_direction(TOUCHSCREEN_GPIO_INT, GPIO_MODE_INPUT);
    // pinMode(TOUCHSCREEN_INT, INPUT_PULLUP);
    gpio_set_intr_type(TOUCHSCREEN_GPIO_INT, GPIO_INTR_NEGEDGE);
    // gpio_isr_register(touchscreenInterrupt, NULL, 0 , NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(TOUCHSCREEN_GPIO_INT, touchscreenInterrupt, NULL);
    // attachInterrupt(TOUCHSCREEN_INT, tsInt, FALLING);
    gpio_intr_enable(TOUCHSCREEN_GPIO_INT);
    hardwareReset();
    if (!softwareReset()) {
        // gpio_isr_handler_remove(TOUCHSCREEN_GPIO_INT);
        // detachInterrupt(TOUCHSCREEN_INT);
        ESP_LOGE(TAG, "unable to reset");
        return false;
    }
    getResolution(&_tsXResolution, &_tsYResolution);
    setPowerState(pwrState);

    // tsInt();
    return true;
}


/**
 * @brief       touchInArea checks if touch occured in given rectangle area
 *
 * @param       int16_t x1
 *              rectangle top left corner x plane
 * @param       int16_t y1
 *              rectangle top left corner y plane
 * @param       int16_t w
 *              rectangle width
 * @param       int16_t h
 *              rectangle height
 *
 * @return      true if successful, false if failed
 */
bool TouchScreen::touchInArea(int16_t x1, int16_t y1, int16_t w, int16_t h)
{
    int16_t x2 = x1 + w, y2 = y1 + h;
    if (isAvailable()) {
        uint8_t n;
        uint16_t x[2], y[2];
        n = getData(x, y);

        if (n) {
            ESP_LOGI(TAG, "at: (%ld,%ld) (%ld,%ld) (%ld,%ld)", x, y, x1, y1, x2, y2 );
            _touchTime = esp_timer_get_time();
            _touchData = n;
            memcpy(_touchX, x, 2);
            memcpy(_touchY, y, 2);
        }
    }
    if (esp_timer_get_time() - _touchTime < 100) {
        // Serial.printf("%d: %d, %d - %d, %d\n", touchN, touchX[0], touchY[0],
        // touchX[1], touchY[1]);
        if (_touchData == 1 && BOUND(x1, _touchX[0], x2) && BOUND(y1, _touchY[0], y2)) {
            return true;
        }
        if (_touchData == 2 && ((BOUND(x1, _touchX[0], x2) && BOUND(y1, _touchY[0], y2)) ||
                            (BOUND(x1, _touchX[1], x2) && BOUND(y1, _touchY[1], y2)))) {

            return true;
        }
    }
    return false;
}

/**
 * @brief       tsWriteRegs writes data to touchscreen registers
 *
 * @param       uint8_t _addr
 *              touchscreen register address
 * @param       uint8_t *_buff
 *              buffer to write into touchscreen registers
 * @param       uint8_t _size
 *              number of bytes to write
 *
 * @return      returns 1 on successful write, 0 on fail
 */
uint8_t TouchScreen::writeRegs(uint8_t addr, const uint8_t *buff, uint8_t size)
{
    Wire::enter();
    wire.begin_transmission(addr);
    for (uint8_t i = 0 ; i < size; i ++) {
        wire.write(buff[i]);
    }
    wire.end_transmission();
    Wire::leave();
    return true;
}

/**
 * @brief       tsReadRegs returns touchscreen registers content
 *
 * @param       uint8_t _addr
 *              touchscreen register address
 * @param       uint8_t *_buff
 *              buffer to write touchscreen register content into
 * @param       uint8_t _size
 *              number of bytes to read
 */
void TouchScreen::readRegs(uint8_t addr, uint8_t *buff, uint8_t size)
{
    Wire::enter();
    wire.request_from(addr, size);
    for ( uint8_t i = 0; i < size ; i ++) {
        buff[i] = wire.read();
    }
    Wire::leave();
}

/**
 * @brief       tsHardwareReset resets ts hardware
 */
void TouchScreen::hardwareReset()
{
    Wire::enter();
    _mcp.digital_write(TOUCHSCREEN_RTS, MCP23017::SignalLevel::LOW);
    vTaskDelay(  20 / portTICK_PERIOD_MS );
    _mcp.digital_write(TOUCHSCREEN_RTS, MCP23017::SignalLevel::HIGH);
    vTaskDelay(  20 / portTICK_PERIOD_MS );
    Wire::leave();
/*
    digitalWriteInternal(MCP23017_INT_ADDR, mcpRegsInt, TS_RTS, LOW);
    delay(15);
    digitalWriteInternal(MCP23017_INT_ADDR, mcpRegsInt, TS_RTS, HIGH);
    delay(15);
*/
}

/**
 * @brief       tsSoftwareReset resets toucscreen software
 *
 * @return      true if successful, false if failed
 */
bool TouchScreen::softwareReset()
{
    const char hello_packet[4] = {0x55, 0x55, 0x55, 0x55};
    const uint8_t soft_rst_cmd[] = {0x77, 0x77, 0x77, 0x77};

    if ( !writeRegs(TOUCH_SCREEN_ADDRESS, soft_rst_cmd, 4)) {
        ESP_LOGW(TAG, "unable to send data");
        return false;
    }
    uint8_t rb[4];
    uint16_t timeout = 100;
    while (!_tsFlag && timeout > 0) {
        vTaskDelay( 10 / portTICK_PERIOD_MS );
        // delay(1);
        timeout--;
    }
    if (timeout > 0)
        _tsFlag = true;
    Wire::enter();
    wire.request_from(0x15, 4);
    for (uint8_t i = 0 ; i < 4; i ++) {
        rb[i] = wire.read();
    }
    Wire::leave();
    _tsFlag = false;
    if (memcmp(rb, hello_packet, 4) != 0) {
        ESP_LOGW(TAG, "bad hello packet");
        return false;
    }
    return true;
}


/**
 * @brief       tsShutdown turns off touchscreen power
 */
void TouchScreen::shutdown()
{
    Wire::enter();
    _mcp.digital_write(TOUCHSCREEN_EN, MCP23017::SignalLevel::HIGH);
    Wire::leave();

//    digitalWriteInternal(MCP23017_INT_ADDR, mcpRegsInt, TOUCHSCREEN_EN, HIGH);
}

/**
 * @brief       tsGetRawData gets touchscreen register content
 *
 * @param       uint8_t *b
 *              pointer to store register content
 */
void TouchScreen::getRawData(uint8_t *buffer)
{
    Wire::enter();
    wire.request_from(TOUCH_SCREEN_ADDRESS, 8);
    for (uint8_t i = 0 ;i < 8; i ++) {
        buffer[i] = wire.read();
    }
    Wire::leave();
}

/**
 * @brief       tsGetXY gets x and y plane values
 *
 * @param       uint8_t *_d
 *              pointer to register content of touchscreen register (data must
 * be adapted, cant use raw data)
 * @param       uint16_t *x
 *              pointer to store x plane data
 * @param       uint16_t *y
 *              pointer to store y plane data
 */
void TouchScreen::getXY(uint8_t *data, uint16_t *x, uint16_t *y)
{
    *x = *y = 0;
    *x = (data[0] & 0xf0);
    *x <<= 4;
    *x |= data[1];
    *y = (data[0] & 0x0f);
    *y <<= 8;
    *y |= data[2];
}

/**
 * @brief       tsGetData checks x, y position and returns number of fingers on
 * screen
 *
 * @param       uint16_t *xPos
 *              pointer to store x position of finger
 * @param       uint16_t *yPos
 *              pointer to store y position of finger
 *
 * @return      returns number of fingers currently on screen
 *
 * @note        touch screen doesn't return data for two fingers when fingers
 * are align at the y axis, or one above another
 */
uint8_t TouchScreen::getData(uint16_t *xPos, uint16_t *yPos)
{
    uint8_t raw[8];
    uint16_t xRaw[2], yRaw[2];
    uint8_t fingers = 0;
    _tsFlag = false;
    getRawData(raw);
    for (int i = 0; i < 8; i++) {
        if (raw[7] & (1 << i))
            fingers++;
    }
    uint16_t inplate_width = _eInk.get_width();
    uint16_t inplate_height = _eInk.get_height();

    for (int i = 0; i < 2; i++) {
        getXY((raw + 1) + (i * 3), &xRaw[i], &yRaw[i]);
        if (xRaw[i] != 0 || yRaw[i] != 0) {
            ESP_LOGI(TAG, "raw %d %d:%d", i, xRaw[i], yRaw[i]);            
        }
        // uint8_t rotation = Inkplate::getRotation();
        uint8_t rotation = 2;
        switch (rotation)
        {
        case 0:
            yPos[i] = ((xRaw[i] * inplate_height - 1) / _tsXResolution);
            xPos[i] = inplate_width - 1 - ((yRaw[i] * inplate_width - 1) / _tsYResolution);
            break;
        case 1:
            xPos[i] = ((xRaw[i] * inplate_height - 1) / _tsXResolution);
            yPos[i] = ((yRaw[i] * inplate_width - 1) / _tsYResolution);
            break;
        case 2:
            yPos[i] = inplate_height - 1 - ((xRaw[i] * inplate_height - 1) / _tsXResolution);
            xPos[i] = ((yRaw[i] * inplate_width - 1) / _tsYResolution);
            break;
        case 3:
            xPos[i] = inplate_height - 1 - ((xRaw[i] * inplate_height - 1) / _tsXResolution);
            yPos[i] = inplate_width - 1 - ((yRaw[i] * inplate_width - 1) / _tsYResolution);
            break;
        }
    }
    return fingers;
}

/**
 * @brief       tsGetResolution gets touchscreen resolution for x and y
 *
 * @param       uint16_t *xRes
 *              pointer to store x resolution
 * @param       uint16_t *yRes
 *              pointer to store y resolution
 */
void TouchScreen::getResolution(uint16_t *xRes, uint16_t *yRes)
{
    const uint8_t cmd_x[] = {0x53, 0x60, 0x00, 0x00}; // Get x resolution
    const uint8_t cmd_y[] = {0x53, 0x63, 0x00, 0x00}; // Get y resolution
    uint8_t rec[4];
    writeRegs(TOUCH_SCREEN_ADDRESS, cmd_x, 4);
    readRegs(TOUCH_SCREEN_ADDRESS, rec, 4);
    ESP_LOGI(TAG, "res x: %ld, %ld, %ld, %ld", rec[0], rec[1], rec[2], rec[4]);
    *xRes = ((rec[2])) | ((rec[3] & 0xf0) << 4);
    ESP_LOGI(TAG, "x: %d", *xRes);
    writeRegs(TOUCH_SCREEN_ADDRESS, cmd_y, 4);
    readRegs(TOUCH_SCREEN_ADDRESS, rec, 4);
    ESP_LOGI(TAG, "res y: %ld, %ld, %ld, %ld", rec[0], rec[1], rec[2], rec[4]);
    *yRes = ((rec[2])) | ((rec[3] & 0xf0) << 4);
    ESP_LOGI(TAG, "y: %d", *yRes);
    _tsFlag = false;
}

/**
 * @brief       tsSetPowerState sets power state of touchscreen
 *
 * @param       uint8_t _s
 *              touchscreen power state to be set (0 or 1)
 */
void TouchScreen::setPowerState(uint8_t stat)
{
    stat &= 1;
    uint8_t powerStateReg[] = {0x54, 0x50, 0x00, 0x01};
    powerStateReg[1] |= (stat << 3);
    writeRegs(TOUCH_SCREEN_ADDRESS, powerStateReg, 4);
}

/**
 * @brief       tsGetPowerState checks if touchscreen is powered up
 *
 * @return      touchscreen power state, 1 if powered, 0 if not
 */
uint8_t TouchScreen::getPowerState()
{
    const uint8_t powerStateReg[] = {0x53, 0x50, 0x00, 0x01};
    uint8_t buf[4];
    writeRegs(TOUCH_SCREEN_ADDRESS, powerStateReg, 4);
    _tsFlag = false;
    readRegs(TOUCH_SCREEN_ADDRESS, buf, 4);
    return (buf[1] >> 3) & 1;
}

/**
 * @brief       tsAvailable checks for touch screen functionality
 *
 * @return      tsflag, 1 for available touchscreen, 0 if not
 */
bool TouchScreen::isAvailable()
{
    return _tsFlag;
}
