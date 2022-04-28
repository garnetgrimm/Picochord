# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    main.cpp
	chord.cpp
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_gpio hardware_clocks hardware_pwm hardware_adc)

# Enable usb output, disable uart output
# pico_enable_stdio_usb(${PROJECT_NAME} 1)
# pico_enable_stdio_uart(${PROJECT_NAME} 0)