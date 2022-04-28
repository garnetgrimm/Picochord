#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "chord.h"

float strumFreq = 261.63f;
float duty = 0.0f;
float timeSincePress = 0.0f;
float updateTimeMs = 10.0f;

void confPWM(int pin, float f_pwm, float duty, float phase) {
	
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
	if(phase > -0.0001f) {
		pwm_set_counter(slice_num, (uint32_t)(top*(phase/360.0f))); //set phase
	}
	pwm_set_enabled(slice_num, true); // let's go!
}

float midiFreq(int midiNum) {
	return 440.0f*powf(2.0f, ((float)midiNum - 69.0f) / 12.0f);
}

int main() 
{
	float keypadLoadHz = 2000.0f;
	float keypadClockHz = keypadClockHz * 16.0f;
	float strumLoadHz = 500.0f;
	Chord activeChord;
	
	activeChord = NoChord();
	
	//control signals
	confPWM(6, strumLoadHz, 0.25f, 0.0f);
	confPWM(8, strumLoadHz, 0.10f, 180.0f);
	confPWM(10, keypadLoadHz, 0.05f, 0.0f);
	confPWM(12, keypadClockHz, 0.5f, 5.0f);
	
	Chord dummyChord[4];
	int dummyStrumProg[4] = { 5, 7, 9, 12 };
		
	dummyChord[0] = Chord::makeChord(60 - 12, MAJOR);
	dummyChord[1] = Chord::makeChord(64 - 12, MINOR);
	dummyChord[2] = Chord::makeChord(65 - 12, MAJOR);
	dummyChord[3] = Chord::makeChord(62 - 12, MINOR);
	
	while(1) {
		for(int i = 0; i < 4; i++) {
			activeChord = dummyChord[i];
			confPWM(0, midiFreq(activeChord[0]), 0.50f, -1.0f);
			confPWM(2, midiFreq(activeChord[1]), 0.50f, -1.0f);
			confPWM(4, midiFreq(activeChord[2]), 0.50f, -1.0f);
			sleep_ms(1000);
		}
	}
	
	return 0;
}
