
set(DRIVERS_SRC
	drivers/eink.cpp
	drivers/inkplate_platform.cpp
	drivers/mcp23017.cpp
)

if(CONFIG_INKPLATE_6)
    list(APPEND DRIVERS_SRC drivers/eink_6.cpp)
endif()

if(CONFIG_INKPLATE_6_PLUS)
    list(APPEND DRIVERS_SRC drivers/eink_6.cpp)
endif()

if(CONFIG_INKPLATE_10)
    list(APPEND DRIVERS_SRC drivers/elink_10.cpp)
endif()

if(CONFIG_INKPLATE_BATTERY)
    list(APPEND DRIVERS_SRC drivers/battery.cpp)
endif()

if(CONFIG_INKPLATE_BACKLIGHT)
    list(APPEND DRIVERS_SRC drivers/backlight.cpp)
endif()

if(CONFIG_INKPLATE_INPUT_KEYS)
    list(APPEND DRIVERS_SRC drivers/press_keys.cpp)
endif()

if(CONFIG_INKPLATE_INPUT_TOUCH)
    list(APPEND DRIVERS_SRC drivers/touch_keys.cpp)
endif()

if(CONFIG_INKPLATE_RTC)
	list(APPEND DRIVERS_SRC drivers/PCF85063A.cpp)
endif()

set(GRAPHICAL_SRC
	graphical/adafruit_gfx.cpp
	graphical/graphics.cpp
	graphical/image.cpp
	graphical/image_bmp.cpp
	graphical/image_dither.cpp
	graphical/image_dither_color.cpp
	graphical/image_jpeg.cpp
	graphical/image_png.cpp
	graphical/image_utils.cpp
	graphical/inkplate.cpp
	graphical/pngle.cpp
	graphical/print.cpp
	graphical/shapes.cpp
	graphical/shapes_polygon.cpp
	graphical/stdlib_noniso.cpp
	graphical/tjpg_decoder.cpp
	graphical/tjpgd.cpp
)

set(SERVICES_SRC
	services/sd_card.cpp
	services/wire.cpp
)

set(INCLUDE_FOLDERS
	include/drivers
	include/fonts
	include/graphical
	include/services
	include/tools
)

idf_component_register(SRCS ${DRIVERS_SRC} ${GRAPHICAL_SRC} ${SERVICES_SRC}
                    INCLUDE_DIRS ${INCLUDE_FOLDERS}
					REQUIRES fatfs nvs_flash)
