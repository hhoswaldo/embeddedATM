cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(freeRTOS_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add third party libraries
add_library(LCD_I2C STATIC lib/LCDI2C/LCD_I2C.c)
add_library(keypad_4x4 STATIC lib/keypad_4x4/keypad_4x4.c)
add_library(picoServo STATIC lib/picoServo/picoServo.c)

#Add executable
add_executable (main
    src/main.c
)

# Link required libraries to keypad_4x4
target_link_libraries(keypad_4x4
    pico_stdlib
    hardware_timer
    )

# Link required libraries to LCD_I2C
target_link_libraries(LCD_I2C
    pico_stdlib
    hardware_i2c
    hardware_timer
    pico_binary_info
    )

target_link_libraries(picoServo
    pico_stdlib
    hardware_pwm
    hardware_clocks
    pico_time
)

# Link required libraries to main
target_link_libraries(main
    LCD_I2C
    keypad_4x4
    picoServo
    hardware_gpio
    pico_time
)



# Enable serial communication
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# Create map/bin/hex, etc.
pico_add_extra_outputs(main)

add_custom_command(TARGET main
        POST_BUILD
        COMMAND arm-none-eabi-size main.elf
)