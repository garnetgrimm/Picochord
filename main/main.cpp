#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "chord.h"
#include "keypad.h"

//all pwm pins with different frequencys or phase offset need to be one pin apart
const uint8_t o1Pin = 0; 	//oscillator one pin
const uint8_t o2Pin = 1; 	//oscillator two pin
const uint8_t o3Pin = 2; 	//oscillator three pin
const uint8_t sLOADPin = 4; 	//strumpad load pin
const uint8_t sCLKPin = 6;	//strumpad clock pin
//use some of the previously skipped pins. They're ok to use, the associated timer is just being utilized.
const uint8_t kLOADPin = 1;	//keypad load pin
const uint8_t kCLKPin = 3;	//keypad clock pin
const uint8_t kR1Pin = 5;	//keypad row one pin
const uint8_t kR2Pin = 7;	//keypad row two pin
const uint8_t kR3Pin = 9;	//keypad row three pin

uint8_t current_col = 0;

const float keypadLoadHz = 60.0f;
const float keypadClockHz = keypadClockHz * (float)cols;
const float strumLoadHz = 500.0f;

Chord activeChord = NoChord();

Keypad activePad;
Keypad finalPad;

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

bool tick_keypad(struct repeating_timer *t) {
	
	bool clkStat = gpio_get_out_level(kCLKPin);	
	
	if(clkStat) {
		activePad.set_keypad(0, current_col, gpio_get(kR1Pin));
		activePad.set_keypad(1, current_col, gpio_get(kR2Pin));
		activePad.set_keypad(2, current_col, gpio_get(kR3Pin));
		current_col++;
		
		gpio_put(kLOADPin, false);
	}
	
	gpio_put(kCLKPin, !clkStat);
	
	if(current_col > cols) {
		current_col = 0;
		gpio_put(kLOADPin, true);
		
		finalPad.update(activePad);
		
		activeChord = finalPad.to_chord();
	
		//set oscillators
		confPWM(o1Pin, midiFreq(activeChord[0]), 0.50f, -1.0f);
		confPWM(o2Pin, midiFreq(activeChord[1]), 0.50f, -1.0f);
		confPWM(o3Pin, midiFreq(activeChord[2]), 0.50f, -1.0f);	
		
		printf("\033[2J");	
		printf("%d %d\r\n", activeChord.root, activeChord.type);
		finalPad.print();
	}
	
	return true;
}

int main() 
{
	stdio_init_all();
	
	//keypad control signals
	gpio_init(kLOADPin);
	gpio_init(kCLKPin);
	gpio_init(kR1Pin);
	gpio_init(kR2Pin);
	gpio_init(kR3Pin);
	
    gpio_set_dir(kLOADPin, GPIO_OUT);
	gpio_set_dir(kCLKPin,  GPIO_OUT);
	gpio_set_dir(kR1Pin,   GPIO_IN);
	gpio_set_dir(kR2Pin,   GPIO_IN);
	gpio_set_dir(kR3Pin,   GPIO_IN);
	
	//strumpad control signals
	confPWM(sLOADPin, strumLoadHz, 0.25f, 0.0f);
	confPWM(sCLKPin, strumLoadHz, 0.10f, 180.0f);
	
	struct repeating_timer timer;
	add_repeating_timer_us(-1000, tick_keypad, NULL, &timer);
	
	while (true) {
		//sleep_ms(1000);
    }
}