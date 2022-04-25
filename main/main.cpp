#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

typedef uint16_t u16;
typedef uint32_t u32;

void confPWM(int pin, u32 f_pwm, u16 duty) {
	gpio_set_function(pin, GPIO_FUNC_PWM); // Tell GPIO 0 it is allocated to the PWM
	uint slice_num = pwm_gpio_to_slice_num(pin); // get PWM slice for GPIO 0 (it's slice 0)
	
	// set frequency
	// determine top given Hz - assumes free-running counter rather than phase-correct
	u32 f_sys = clock_get_hz(clk_sys); // typically 125'000'000 Hz
	float divider = f_sys / 1000000UL;  // let's arbitrarily choose to run pwm clock at 1MHz
	pwm_set_clkdiv(slice_num, divider); // pwm clock should now be running at 1MHz
	u32 top =  1000000UL/f_pwm -1; // TOP is u16 has a max of 65535, being 65536 cycles
	pwm_set_wrap(slice_num, top);

	// set duty cycle
	u16 level = (top+1) * duty / 100 -1; // calculate channel level from given duty cycle in %
	pwm_set_chan_level(slice_num, 0, level); 
	
	pwm_set_enabled(slice_num, true); // let's go!
}

int main() 
{
	confPWM(0, 440, 50);
	confPWM(2, 392, 50);
	confPWM(4, 261, 50);
	for(;;);
	return 0;
}
