#include "oscillator.h"
#include <stdio.h>

float decay = 0.5f;
float maxDecay = 1.0f;


Wave::Wave() {
	for(int i = 0; i < RESOLUTION; i++) {
		data[i] = sink(0.0f);
	}
}

sink Wave::get_data(uint32_t pos) {
	return data[pos];
}

SquareWave::SquareWave() {
	for(int i = 0; i < RESOLUTION/2; i++) {
		data[i] = sink(1.0f);
	}
	for(int i = RESOLUTION/2; i < RESOLUTION; i++) {
		data[i] = sink(0.0f);
	}
}

SinWave::SinWave() {
	for(int i = 0; i < RESOLUTION; i++) {
		float x = ((float)i/(float)RESOLUTION)*2.0*M_PI;
		float y = sin(x) + 1.0f;
		data[i] = sink(y);
	}
}


Oscillator::Oscillator() {
	set_freq(440.0f);
}

float Oscillator::get_freq() { 
	return frequency;
}

void Oscillator::set_freq(float f) {
	frequency = f;
	playRate = sink(f*((float)RESOLUTION)/((float)SAMPLE_FREQ));
	decayRate = sink((maxDecay*decay)/((float)SAMPLE_FREQ));
}

void Oscillator::set_wave(Wave* w) {
	wave = w;
}

sink Oscillator::tick(bool fade) {
	if(wave == nullptr) return zero;
	position += playRate;
	if(position > RES_SINK) position -= RES_SINK;
	if(fade && volume > zero) volume -= decayRate;
	return wave->get_data(static_cast<uint32_t>(position))*volume;
}