# NTP Time/Date Decoder for RPi PICO-W

> Display NTP decoded Date & Time on HD44780 16x2 LCD 


## Hardware
> RPi PICO-W
 
> LCD I2C connects to PICO-W default GPIO4 (SDA) and GPIO5 (SCK)

## Building
> export PICO_SDK_PATH=<PATH TO PICO SDK>

> mkdir -p build

> cd build

> cmake -DPICO_SDK_PATH=${PICO_SDK_PATH} -DPICO_BOARD=pico_w -DWIFI_SSID="SSID" -DWIFI_PASSWORD="PASSWORD" ..

> make

## Copy to PICO-W
> Connect PICO-W to PC using USB connection

> Power-on PICO-W holding BOOTSEL

> Release BOOTSEL when PICO-W mounted

> On Linux: "cp build/ntp_rtc_lcd_clock_background.uf2 <mount path>/RPI-RP2/main.uf2"

> PICO-W will boot and should connect to Wi-Fi and display NTP time from NTP server
 




