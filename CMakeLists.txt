set(PROJECT ntp_rtc_lcd_clock)
set(PICO_BOARD pico_w)
cmake_minimum_required(VERSION 3.12)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(${PROJECT} C CXX ASM)
pico_sdk_init()

add_executable(ntp_rtc_lcd_clock_background
        ntp_rtc_lcd_clock.c 
        hd44780_lcd_api.c 
        )
target_compile_definitions(ntp_rtc_lcd_clock_background PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(ntp_rtc_lcd_clock_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
target_link_libraries(ntp_rtc_lcd_clock_background
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        hardware_i2c  
        hardware_rtc 
        )

pico_add_extra_outputs(ntp_rtc_lcd_clock_background)

pico_enable_stdio_usb(ntp_rtc_lcd_clock_background 0)
pico_enable_stdio_uart(ntp_rtc_lcd_clock_background 1)
