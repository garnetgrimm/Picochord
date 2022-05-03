#include "oscillator.h"

float decay = 0.5f;
float maxDecay = 1.0f;

FOscillator::FOscillator() {
	set_freq(440.0f);
}

FOscillator::FOscillator(float f) {
	set_freq(f);
}

void FOscillator::set_freq(float f) {
	frequency = f;
	playRate = (f*(float)RESOLUTION)/((float)SAMPLE_RATE);
	decayRate = decay*((float)SAMPLE_RATE);
}

uint8_t FOscillator::tick(bool fade) {
	position += playRate;
	if(position > (float)RESOLUTION) position -= RESOLUTION;
	
	if(fade && volume > 0.0f) volume -= decayRate;
	
	return (uint8_t)(255.0f*wave[(int)position]*volume);
}

SquareFOSC::SquareFOSC(float f) : FOscillator(f) {
	int i = 0;
	for(i = 0; i < RESOLUTION/2; i++) {
		wave[i] = 1.0f;
	}
	for(i = RESOLUTION/2; i < RESOLUTION; i++) {
		wave[i] = 0.0f;
	}
}

SOscillator::SOscillator() {
	set_freq(440.0f);
}

SOscillator::SOscillator(float f) {
	set_freq(f);
}

float SOscillator::get_freq() { 
	return frequency;
}

void SOscillator::set_freq(float f) {
	frequency = f;
	playRate = sink((f*(float)RESOLUTION)/((float)SAMPLE_RATE));
	decayRate = sink((maxDecay*decay)/((float)SAMPLE_RATE));
}

uint8_t SOscillator::tick(bool fade) {
	position += playRate;
	if(position > RES_SINK) position -= RES_SINK;
	if(fade && volume > zero) volume -= decayRate;
	uint16_t wave_index = static_cast<int>(position);
	return static_cast<uint8_t>(255*wave[wave_index]*volume);
}

SquareSOSC::SquareSOSC(float f) : SOscillator(f) {
	int i = 0;
	for(i = 0; i < RESOLUTION/2; i++) {
		wave[i] = sink(1.0f);
	}
	for(i = RESOLUTION/2; i < RESOLUTION; i++) {
		wave[i] = sink(0.0f);
	}
}