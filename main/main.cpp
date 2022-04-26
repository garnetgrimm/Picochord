#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

void confPWM(int pin, float f_pwm, float duty) {
	
	gpio_set_function(pin, GPIO_FUNC_PWM); // Tell GPIO 0 it is allocated to the PWM
	uint slice_num = pwm_gpio_to_slice_num(pin); // get PWM slice for GPIO 0 (it's slice 0)
	
	// set frequency
	// determine top given Hz - assumes free-running counter rather than phase-correct
	uint32_t f_sys = clock_get_hz(clk_sys); // typically 125'000'000 Hz
	float divider = f_sys / 1000000UL;  // let's arbitrarily choose to run pwm clock at 1MHz
	pwm_set_clkdiv(slice_num, divider); // pwm clock should now be running at 1MHz
	uint32_t top =  (uint32_t)(1000000.0f/f_pwm - 1.0f); // TOP is u16 has a max of 65535, being 65536 cycles
	pwm_set_wrap(slice_num, top);

	// set duty cycle
	uint16_t level = (uint16_t)(((float)top+1.0f) * duty - 1.0f); // calculate channel level from given duty cycle in %
	pwm_set_chan_level(slice_num, 0, level); 
	
	pwm_set_enabled(slice_num, true); // let's go!
}

int main() 
{
	confPWM(0, 261.63f, 0.50f);
	confPWM(2, 329.63f, 0.50f);
	confPWM(4, 392.00f, 0.50f);
	for(;;);
	return 0;
}
