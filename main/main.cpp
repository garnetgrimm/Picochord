#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/irq.h"  // interrupts
#include "hardware/sync.h" // wait for interrupt 
 
// Audio PIN is to match some of the design guide shields. 
#define AUDIO_PIN 28  // you can change this to whatever you like
#define OSCILLATORS 16

#include "chord.h"
#include "keypad.h"
#include "oscillator/oscillator.h"

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

SOscillator oscs[OSCILLATORS];

Chord chord = NoChord();

Keypad activePad;
Keypad finalPad;

long uptime = 0;

float midiFreq(int midiNum) {
	return 440.0f*powf(2.0f, ((float)midiNum - 69.0f) / 12.0f);
}

void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));
	sink sum { 0.0f };
	for(int i = 0; i < 3; i++) {	
		sum += oscs[i].tick(false);
	}
	for(int i = 3; i < OSCILLATORS; i++) {	
		sum += oscs[i].tick(true);
	}
	uint8_t level = static_cast<uint16_t>(sum/OSCILLATORS);
	if(level > 0xFF) level = 0xFF;
    pwm_set_gpio_level(AUDIO_PIN, level);
}

bool count(struct repeating_timer *t) {
	uptime++;
	return true;
}

void updateOsc() {
	for(int i = 0; i < OSCILLATORS; i++) {
		float freq = midiFreq(chord[i % 3]);
		int fscale = 1<<(i/3);
		oscs[i] = SquareSOSC(freq*(float)fscale);
		oscs[i].volume = sink(0.0f);
	}
	oscs[0].volume = sink(0.5f);
	oscs[1].volume = sink(0.5f);
	oscs[2].volume = sink(0.5f);
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
		
		chord = finalPad.to_chord();
		updateOsc();
		
		printf("\033[2J");	
		printf("%d %d\r\n", chord.root, chord.type);
		finalPad.print();
	}
	
	return true;
}


int main(void) {
	
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
	
	chord = MajChord(60-12);
	updateOsc();

    set_sys_clock_khz(176000, true); 
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    // Setup PWM interrupt to fire when PWM cycle is complete
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    // set the handle function above
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Setup PWM for audio output
    pwm_config config = pwm_get_default_config();
    //Base clock 176,000,000 Hz divide by wrap 250 then the clock divider further divides
    //to set the interrupt rate. 
    //11 KHz is fine for speech. Phone lines generally sample at 8 KHz
    //So clkdiv should be as follows for given sample rate
	//16.0 for 5.5kHz
	//8.0f for 11 KHz
    //4.0f for 22 KHz
    //2.0f for 44 KHz etc
    pwm_config_set_clkdiv(&config, 16.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);
	
	
	Chord dummyChord[4];
	int dummyStrumProg[4] = { 5, 7, 9, 12 };
		
	dummyChord[0] = Chord::makeChord(60 - 12, MAJOR);
	dummyChord[1] = Chord::makeChord(64 - 12, MINOR);
	dummyChord[2] = Chord::makeChord(65 - 12, MAJOR);
	dummyChord[3] = Chord::makeChord(62 - 12, MINOR);
	
	while(1) {
		for(int i = 0; i < 4; i++) {
			chord = dummyChord[i];
			updateOsc();
			for(int i = 0; i < 4; i++) {
				oscs[dummyStrumProg[i]+3].volume = sink(1.0f);
				printf("%d chime %f\r\n", i, oscs[i].get_freq());
				sleep_ms(250);
			}
			sleep_ms(1000);
		}
	}
	
	//struct repeating_timer timer;
	//add_repeating_timer_us(-1000, tick_keypad, NULL, &timer);
}
