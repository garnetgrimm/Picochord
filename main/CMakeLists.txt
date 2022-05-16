# Tell CMake where to find the executable source file
add_executable(main
    main.cpp
	keypad.cpp
	strumpad.cpp
	chord.cpp
	oscillator.cpp
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(main)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(main pico_stdlib hardware_gpio hardware_clocks hardware_pwm hardware_adc)

# Enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)