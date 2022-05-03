# Tell CMake where to find the executable source file
add_executable(oscillator
    main.cpp
	oscillator.cpp
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(oscillator)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(oscillator pico_stdlib hardware_clocks)

# Enable usb output, disable uart output
pico_enable_stdio_usb(oscillator 1)
pico_enable_stdio_uart(oscillator 0)