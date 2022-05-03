#ifndef __OSCILLATOR__
#define __OSCILLATOR__

#include "fpm/fixed.h"

using namespace fpm;
using sink = fixed<std::int32_t, std::int64_t, 20>;

constexpr uint32_t RESOLUTION = 1024;
constexpr uint32_t SAMPLE_RATE = 11000*16;

class FOscillator {
protected:
	float wave[RESOLUTION];
	float position = 0.0f;
	float frequency = 440.0f;
	float playRate = 0.0f;
	float decayRate = 0.0f;
public:
	float volume = 1.0f;
	FOscillator();
	FOscillator(float f);
	void set_freq(float f);
	float get_freq();
	uint8_t tick(bool fade);
};

class SquareFOSC : public FOscillator {
public:
	SquareFOSC(float f);
};

class SOscillator {
protected:
	sink wave[RESOLUTION];
	sink position { 0.0f };
	float frequency = 440.0f;
	sink playRate { 0.0f };
	sink decayRate { 0.0f };
	sink RES_SINK { RESOLUTION };
	sink zero { 0.0f };
public:
	sink volume { 1.0f };
	SOscillator();
	SOscillator(float f);
	void set_freq(float f);
	float get_freq();
	uint8_t tick(bool fade);
};

class SquareSOSC : public SOscillator {
public:
	SquareSOSC(float f);
};

#endif