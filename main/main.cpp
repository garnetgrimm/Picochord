#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "MidiNote.h"
#include "Scale.h"

float strumFreq = 261.63f;
float duty = 0.0f;
float timeSincePress = 0.0f;
float updateTimeMs = 10.0f;

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

float precToFreq(float perc) {
	//float cMaj[] = { 0.0f, 0.0f, 0.0f, 0.0f, 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25 };
	float cMaj[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 523.25f, 587.33f, 659.25f, 698.46f, 783.99f, 880.00f, 987.77f, 1046.50f };
	int idx = (int)(perc*(8.0f+5.0f));
	return cMaj[idx];
}

enum PressType { ERROR, MAJOR, MINOR, SEV, MAJSEV, MINSEV, AUGMEN, DIMINISH };

struct OmniButton {
	MidiNote root;
	PressType pressType;
	Scale scale;
	MidiNote chord[3];
	
	OmniButton() {
		this->root = 60;
		this->pressType = MAJOR;
		
		chord[0] = scale.notes[0];
		chord[1] = scale.notes[2];
		chord[2] = scale.notes[4];
	}
	
	OmniButton(int root, PressType pressType) {
		this->root = root;
		this->pressType = pressType;
		
		if(pressType == MAJOR) {
			scale = MajScale(root);
		} else if(pressType == MINOR) {
			scale = MinScale(root);
		} else {
			scale = RootScale(root);
		}
		
		chord[0] = scale.notes[0];
		chord[1] = scale.notes[1];
		chord[2] = scale.notes[2];
	}
};

int main() 
{
	OmniButton dummyPress[4];
	
	dummyPress[0] = OmniButton(60 - 12, MAJOR);
	dummyPress[1] = OmniButton(64 - 12, MINOR);
	dummyPress[2] = OmniButton(65 - 12, MAJOR);
	dummyPress[3] = OmniButton(62 - 12, MINOR);
	
	int dummyStrumProg[4] = { 5, 7, 9, 12 };
	
	while(1) {
		for(int i = 0; i < 4; i++) {
			OmniButton press = dummyPress[i];
			confPWM(0, press.chord[0].getFrequency(), 0.50f);
			confPWM(2, press.chord[1].getFrequency(), 0.50f);
			confPWM(4, press.chord[2].getFrequency(), 0.50f);
			for(int n = 0; n < 4; n++) {
				confPWM(0, press.scale.notes[dummyStrumProg[n]].getFrequency(), 0.5f);
				sleep_ms(1000/4);
			}
		}
	}

	/*	
    adc_init();
    adc_gpio_init(26); // Make sure GPIO is high-impedance, no pullups etc
    adc_select_input(0); // Select ADC input 0 (GPIO26)

    while (1) {
		
		//avg a bunch of adc
		float avgVal = 0.0f;
		for(int i = 0; i < 100; i++) {
			float adcVal = ((float)adc_read()) / 4095.0f;
			avgVal += adcVal;
		}
		avgVal /= 100.0f;
		float freq = precToFreq(avgVal);
		
		//play note
		if(freq < 0.1f) {
			//10ms
			timeSincePress += updateTimeMs*0.001f;
		} else {
			timeSincePress = 0.0f;
			strumFreq = freq;
		}
		
		float decayTime = 1.0f; //in seconds
		float maxDuty = 0.5f;
		float periods = 5.0f;
		if(timeSincePress < decayTime) {
			float ampScale = (decayTime-timeSincePress)/decayTime;
			float timeScale = (M_PI*(periods-1.0f)*timeSincePress)/decayTime;
			duty = fabs(maxDuty*ampScale*cos(timeScale));
		} else {
			duty = 0.0f;
		}
		
		confPWM(0, strumFreq, duty);
		
        sleep_ms((int)updateTimeMs);
    }
	*/
	while(1) {};
	
	return 0;
}
