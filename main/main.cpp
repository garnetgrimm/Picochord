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
#define OSCILLATORS 16

#include "chord.h"
#include "keypad.h"
#include "oscillator.h"
#include "strumpad.h"

const uint8_t audioPin = 10;
const uint8_t sLOADPin = 15;  //strumpad load pin
const uint8_t sCLKPin = 14;	 //strumpad clock pin
const uint8_t sINPin = 13;	 //strumpad input pin
//use some of the previously skipped pins. They're ok to use, the associated timer is just being utilized.
const uint8_t kLOADPin = 20; //keypad load pin
const uint8_t kCLKPin = 21;	 //keypad clock pin
const uint8_t kR1Pin = 27;	 //keypad row one pin
const uint8_t kR2Pin = 26;	 //keypad row two pin
const uint8_t kR3Pin = 22;	 //keypad row three pin
const uint8_t kR4Pin = 16;	 //keypad row one pin
const uint8_t kR5Pin = 18;	 //keypad row two pin
const uint8_t kR6Pin = 19;	 //keypad row three pin

uint8_t current_col = 0;
uint8_t current_pad = 0;

const float keypadLoadHz = 60.0f;
const float keypadClockHz = keypadClockHz * (float)cols;
const float strumLoadHz = 500.0f;

SquareWave wave;
Oscillator oscs[OSCILLATORS];

Chord chord = NoChord();

Keypad activePad;
Keypad finalPad;

Strumpad strumPad;

uint8_t level = 0;

bool mute_chord = true;

uint32_t strum_load_cnt = 0;
uint32_t strum_load_max = 15;
uint32_t strum_tick_time = 250;

bool blinky = false;

float midiFreq(int midiNum) {
	return 440.0f*powf(2.0f, ((float)midiNum - 69.0f) / 12.0f);
}

void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(audioPin));
    pwm_set_gpio_level(audioPin, level);
}

bool tick_oscillators(struct repeating_timer *t) {
	sink sum { 0.0f };
	if(!mute_chord) {
		for(int i = 0; i < 3; i++) {	
			sum += oscs[i].tick(false);
		}
	}
	for(int i = 3; i < 11; i++) {	
		sum += oscs[i].tick(true);
	}
	level = static_cast<int8_t>((20)*sum) + 0x7F;
	return true;
}

void set_chord(Chord c) {
	chord = c;
	if(chord.type == BLANK) {
		mute_chord = true;
		for(int i = 0; i < 3; i++) {
			oscs[i].set_freq(440);
		}
	} else {
		mute_chord = false;
		for(int i = 0; i < 3; i++) {
			float freq = midiFreq(chord[i % 3]);
			oscs[i].set_freq(freq);
		}
		for(int i = 3; i < OSCILLATORS; i++) {
			float freq = midiFreq(chord[i % 3]);
			int fscale = 1<<((i-3)/3);
			oscs[i].set_freq(freq*(float)fscale);
		}
	}
}

void tick_strumpad(void) {
	if(strum_load_cnt == 10) {
		gpio_put(sLOADPin, false);
		gpio_put(sCLKPin, false);
		gpio_put(sLOADPin, true);
	}
	
	if(strum_load_cnt < strum_load_max)  {
		strum_load_cnt++;
		return;
	}
	
	if(!gpio_get_out_level(sCLKPin)) {
		
		bool pad_on = gpio_get(sINPin) && (current_pad == 7);
		strumPad.set_strumpad(current_pad, pad_on);
		if(pad_on) {
			oscs[current_pad + 3].volume = sink(1.0f);
		}
		
		current_pad++;
		if(current_pad > 8) {
			current_pad = 0;
			gpio_put(sCLKPin, true);
			strum_load_cnt = 0;
			return;
		} else {
			gpio_put(sCLKPin, true);
		}
	} else {
		gpio_put(sCLKPin, false);
	}
}

void tick_keypad(void) {
	bool clkStat = gpio_get_out_level(kCLKPin);	
	
	if(clkStat) {
		activePad.set_keypad(0, 5 - (current_col - 2), gpio_get(kR1Pin));
		activePad.set_keypad(1, 5 - (current_col - 2), gpio_get(kR2Pin));
		activePad.set_keypad(2, 5 - (current_col - 2), gpio_get(kR3Pin));
		activePad.set_keypad(0, current_col - 2 + 6, gpio_get(kR4Pin));
		activePad.set_keypad(1, current_col - 2 + 6, gpio_get(kR5Pin));
		activePad.set_keypad(2, current_col - 2 + 6, gpio_get(kR6Pin));
		current_col++;
		
		gpio_put(kLOADPin, false);
	}
	
	gpio_put(kCLKPin, !clkStat);
	
	if(current_col >= cols) {
		current_col = 0;
		gpio_put(kLOADPin, true);
		
		finalPad.update(activePad);	
		
		Chord detChord = finalPad.to_chord();		
		if(detChord.type != chord.type || detChord.root != chord.root) {
			set_chord(detChord);
		};
	}
}

bool input_ISR(struct repeating_timer *t) {
	tick_keypad();
	tick_strumpad();
	return true;
}

int main(void) {
	
    stdio_init_all();
	set_chord(MajChord(noteMap[0]));
	
	const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
	
	//strumpad control signals
	gpio_init(sLOADPin);
	gpio_init(sCLKPin);
	gpio_init(sINPin);

	gpio_set_dir(sLOADPin, GPIO_OUT);
	gpio_set_dir(sCLKPin, GPIO_OUT);
	gpio_set_dir(sINPin, GPIO_IN);
	
	//keypad control signals
	gpio_init(kLOADPin);
	gpio_init(kCLKPin);
	gpio_init(kR1Pin);
	gpio_init(kR2Pin);
	gpio_init(kR3Pin);
	gpio_init(kR4Pin);
	gpio_init(kR5Pin);
	gpio_init(kR6Pin);
	
    gpio_set_dir(kLOADPin, GPIO_OUT);
	gpio_set_dir(kCLKPin,  GPIO_OUT);
	gpio_set_dir(kR1Pin,   GPIO_IN);
	gpio_set_dir(kR2Pin,   GPIO_IN);
	gpio_set_dir(kR3Pin,   GPIO_IN);	
	gpio_set_dir(kR4Pin,   GPIO_IN);
	gpio_set_dir(kR5Pin,   GPIO_IN);
	gpio_set_dir(kR6Pin,   GPIO_IN);	

    set_sys_clock_khz(176000, true); 
    gpio_set_function(audioPin, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(audioPin);

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
    pwm_config_set_clkdiv(&config, 8.0f); 
    pwm_config_set_wrap(&config, 275); 
    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(audioPin, 0);
	
	for(int i = 0; i < OSCILLATORS; i++) {
		oscs[i].set_wave(&wave);
		oscs[i].volume = sink(0.0f);
	}
	
	struct repeating_timer tim1;
	add_repeating_timer_us(-static_cast<int32_t>(SAMPLE_PERIOD_MICRO), tick_oscillators, NULL, &tim1);
	
	struct repeating_timer tim2;
	add_repeating_timer_us(strum_tick_time, input_ISR, NULL, &tim2);
	
	Chord dummyChord[4];
	int dummyStrumProg[13] = { 4, 11, 2, 7, 3, 0, 10, 5, 8, 9, 12, 6, 1 };
		
	dummyChord[0] = Chord::makeChord(60, MAJOR);
	dummyChord[1] = Chord::makeChord(64, MINOR);
	dummyChord[2] = Chord::makeChord(65, MAJOR);
	dummyChord[3] = Chord::makeChord(62, MINOR);
	
	oscs[0].volume = sink(0.5f);
	oscs[1].volume = sink(0.5f);
	oscs[2].volume = sink(0.5f);
	
	mute_chord = false;
	
	while(1) {
		printf("\033[2J");
		finalPad.print();
		strumPad.print();
		//blinky = !blinky;
		//gpio_put(LED_PIN, blinky);
		sleep_ms(100);
		/*
		for(int i = 0; i < 13; i++) {
			int strumPad = dummyStrumProg[i]+3;
			oscs[strumPad].volume = sink(1.0f);
			
			printf("\033[2J");	
			printf("%d %d\r\n", chord.root, chord.type);
			finalPad.print();
			
			sleep_ms(500);
		}
		*/
	}
	
}
