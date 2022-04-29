#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "chord.h"

//all pwm pins with different frequencys or phase offset need to be one pin apart
const uint8_t o1Pin = 0; 	//oscillator one pin
const uint8_t o2Pin = 1; 	//oscillator two pin
const uint8_t o3Pin = 2; 	//oscillator three pin
const uint8_t sLPin = 4; 	//strumpad load pin
const uint8_t sCLKPin = 6;	//strumpad clock pin
const uint8_t kLPin = 8;	//keypad load pin
const uint8_t kCPin = 10;	//keypad clock pin
//use some of the previously skipped pins. They're ok to use, the associated timer is just being utilized.
const uint8_t kR1Pin = 1;	//keypad row one pin
const uint8_t kR2Pin = 3;	//keypad row two pin
const uint8_t kR3Pin = 5;	//keypad row three pin

const float keypadLoadHz = 2000.0f;
const float keypadClockHz = keypadClockHz * 16.0f;
const float strumLoadHz = 500.0f;

Chord activeChord = NoChord();

//represent each button with 1 bit (12 buttons so last 4 bits unused)
// 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
// 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
// 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
uint16_t keypad[3];

				    //Db  Ab  Eb  Bb  F   C   G   D   A   E   B   F#
uint8_t noteMap[] = { 49, 56, 51, 58, 53, 48, 55, 50, 57, 52, 59, 54 };

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

uint8_t get_active_keypad_column() {
	//the column being checked can be calculated
	//there is a direct relationship between the timer for the keypad load and the keypad clock
	uint16_t slice_num = pwm_gpio_to_slice_num(kLPin);
	uint32_t top =  (uint32_t)(1000000.0f/keypadLoadHz - 1.0f);
	uint32_t count = pwm_get_counter(slice_num);
	return 16*(count/top);
}
 
Chord keypad_to_chord() {
	//if multiple notes are being pressed, pick the highest note
	int max_note = 0;
	for(int row = 0; row < 3; row++) {
		for(int col = 0; col < 16; col++) {
			if(keypad[row] & (1 << col) && col > max_note) max_note = col;
		}
	}
	
	//get midi from note pressed
	int root = noteMap[max_note];
	int chordType = 0;
	
	//determine chord pressed for active note
	for(int row = 0; row < 3; row++) {
		if(keypad[row] & (1 << max_note)) {
			chordType |= (1<<row);
		}
	}
	
	return Chord::makeChord(root, static_cast<ChordType>(chordType));
}
 
void row_event(uint gpio, uint32_t events) {
    uint8_t col = 0;
	uint8_t row = 0;
	switch(gpio) {
		case kR1Pin:
			row = 0;
			break;
		case kR2Pin:
			row = 1;
			break;
		case kR3Pin:
			row = 2;
			break;
	}
	col = get_active_keypad_column();
	
	if(events & (1 << 3)) { //edge rise
		keypad[row] |= (1 << col);
	}
	
	if(events & (1 << 2)) { //edge fall
		keypad[row] &= ~(1 << col);
	}
	
	activeChord = keypad_to_chord();
	
	//set oscillators
	confPWM(o1Pin, midiFreq(activeChord[0]), 0.50f, -1.0f);
	confPWM(o2Pin, midiFreq(activeChord[1]), 0.50f, -1.0f);
	confPWM(o3Pin, midiFreq(activeChord[2]), 0.50f, -1.0f);
}

int main() 
{
	
	//control signals
	confPWM(sLPin, strumLoadHz, 0.25f, 0.0f);
	confPWM(sCLKPin, strumLoadHz, 0.10f, 180.0f);
	confPWM(kLPin, keypadLoadHz, 0.05f, 0.0f);
	confPWM(kCPin, keypadClockHz, 0.5f, 5.0f);
	
	//keypress events
	gpio_set_irq_enabled_with_callback(1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &row_event);
	gpio_set_irq_enabled_with_callback(3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &row_event);
	gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &row_event);
	
	while(1) {};
}
