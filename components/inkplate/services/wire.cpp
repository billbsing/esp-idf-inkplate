
// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#define __WIRE__ 1
#include "wire.hpp"
#include "logging.hpp"
#include "driver/i2c.h"

#include <esp_system.h>
#include <esp_check.h>
#include <cstring>

Wire Wire::singleton;
SemaphoreHandle_t Wire::mutex = nullptr;
StaticSemaphore_t Wire::mutex_buffer;
uint8_t Wire::cmdlink_buffer;

void
Wire::setup()
{
  ESP_LOGD(TAG, "Initializing...");

  if (!initialized) {

    mutex = xSemaphoreCreateMutexStatic(&mutex_buffer);

    i2c_config_t config;

    memset(&config, 0, sizeof(i2c_config_t));

    config.mode             = I2C_MODE_MASTER;
    config.scl_io_num       = GPIO_NUM_22;
    config.scl_pullup_en    = GPIO_PULLUP_DISABLE;
    config.sda_io_num       = GPIO_NUM_21;
    config.sda_pullup_en    = GPIO_PULLUP_DISABLE;
    // config.master.clk_speed = 1E6;
    config.master.clk_speed = 100000;
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    initialized = true;
  }
}

void
Wire::begin_transmission(uint8_t addr)
{
  if (!initialized) setup();

  if (initialized) {
    address = addr;
    index   = 0;
  }
}

void
Wire::end_transmission()
{
    i2c_cmd_handle_t cmd_link = NULL;
    esp_err_t ret = ESP_OK;
  if (initialized) {
    //ESP_LOGD(TAG, "Writing %d bytes to i2c at address 0x%02x.", index, address);

    // for (int i = 0; i < index; i++) {
    //   printf("%02x ", buffer[i]);
    // }
    // printf("\n");
    // fflush(stdout);

    cmd_link = i2c_cmd_link_create_static(Wire::cmdlink_buffer, CMD_HANDLER_BUFFER_SIZE);
    // cmd_link = i2c_cmd_link_create();
    ESP_GOTO_ON_ERROR(i2c_master_start(cmd_link), err, TAG, "i2c_master_start");
    ESP_GOTO_ON_ERROR(i2c_master_write_byte(cmd_link, (address << 1) | I2C_MASTER_WRITE, 1), err, TAG, "i2c_master_write_byte");
    if (index > 0) {
        ESP_GOTO_ON_ERROR(i2c_master_write(cmd_link, buffer, index, 1), err, TAG, "i2c_master_write");
    }
    ESP_GOTO_ON_ERROR(i2c_master_stop(cmd_link), err, TAG, "i2c_master_stop");
    ESP_GOTO_ON_ERROR(i2c_master_cmd_begin(I2C_NUM_0, cmd_link, 10000 / portTICK_PERIOD_MS), err, TAG, "i2c_master_cmd_begin");

    index = 0;
  }
  err:
    if (cmd_link != NULL) {
        // i2c_cmd_link_delete(cmd_link);
        i2c_cmd_link_delete_static(cmd_link);
    }

  // ESP_LOGD(TAG, "I2C Transmission completed.");
}

void
Wire::write(uint8_t val)
{
  if (initialized) {
    buffer[index++] = val;
    if (index >= BUFFER_LENGTH) index = BUFFER_LENGTH - 1;
  }
}

uint8_t
Wire::read()
{
  if (!initialized || (index >= size_to_read)) return 0;
  return buffer[index++];
}

esp_err_t
Wire::request_from(uint8_t addr, uint8_t size)
{
    esp_err_t ret = ESP_ERR_INVALID_STATE;
    i2c_cmd_handle_t cmd_link = NULL;
  if (!initialized) setup();

  if (initialized) {
    if (size == 0) return ESP_OK;
    cmd_link = i2c_cmd_link_create_static(Wire::cmdlink_buffer, CMD_HANDLER_BUFFER_SIZE);

    // i2c_cmd_handle_t cmd_link = i2c_cmd_link_create();
    ESP_GOTO_ON_ERROR(i2c_master_start(cmd_link), err, TAG, "i2c_master_start");
    ESP_GOTO_ON_ERROR(i2c_master_write_byte(cmd_link, (addr << 1) | I2C_MASTER_READ, true), err, TAG, "i2c_master_write_byte");

    if (size > 1) {
        ESP_GOTO_ON_ERROR(i2c_master_read(cmd_link, buffer, size - 1, I2C_MASTER_ACK), err, TAG, "i2c_master_read");
    }
    ESP_GOTO_ON_ERROR(i2c_master_read_byte(cmd_link, buffer + size - 1, I2C_MASTER_LAST_NACK), err, TAG, "i2c_master_read_byte");

    ESP_GOTO_ON_ERROR(i2c_master_stop(cmd_link), err, TAG, "i2c_master_stop");

    ESP_GOTO_ON_ERROR(i2c_master_cmd_begin(I2C_NUM_0, cmd_link, 1000 / portTICK_PERIOD_MS), err, TAG, "i2c_master_cmd_begin");
err:
  if (cmd_link != NULL) {
      // i2c_cmd_link_delete(cmd_link);
      i2c_cmd_link_delete_static(cmd_link);
  }

    size_to_read = size;
    index = 0;

    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Unable to complete request_from: %s.", esp_err_to_name(result));
    }
  }
  return ret;
}
