#define __SD_CARD__ 1
#include "sd_card.hpp"

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

bool
SDCard::mount()
{
  ESP_LOGI(TAG, "Setup SD card");

  static const gpio_num_t PIN_NUM_MISO = GPIO_NUM_12;
  static const gpio_num_t PIN_NUM_MOSI = GPIO_NUM_13;
  static const gpio_num_t PIN_NUM_CLK  = GPIO_NUM_14;
  static const gpio_num_t PIN_NUM_CS   = GPIO_NUM_15;


  // slot_config.gpio_cs = PIN_NUM_CS;

  // slot_config.gpio_miso = PIN_NUM_MISO;
  // slot_config.gpio_mosi = PIN_NUM_MOSI;
  // slot_config.gpio_sck  = PIN_NUM_CLK;
  // slot_config.gpio_cs   = PIN_NUM_CS;
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = SPI2_HOST;

  spi_bus_config_t bus_config = {
      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4000,
  };
  // host.flags = SDMMC_HOST_FLAG_SPI;
  // bus_config.host_id = SPI2_HOST;

  esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize bus.");
      return false;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = PIN_NUM_CS;
  slot_config.host_id = SPI2_HOST;

  esp_vfs_fat_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };

  sdmmc_card_t* card;
  ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem.");
    } else {
      ESP_LOGE(TAG, "Failed to setup the SD card (%s).", esp_err_to_name(ret));
    }
    return false;
  }

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);

  return true;
}
