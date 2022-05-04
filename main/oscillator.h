#ifndef __OSCILLATOR__
#define __OSCILLATOR__

#include "fpm/fixed.h"

using namespace fpm;
using sink = fixed<std::int32_t, std::int64_t, 20>;

constexpr uint32_t RESOLUTION = 1024;
constexpr uint32_t SAMPLE_PERIOD_MICRO = 50;
constexpr uint32_t SAMPLE_FREQ = 1/(SAMPLE_PERIOD_MICRO*1e-6);

class Wave {
protected:
	sink data[RESOLUTION];
public:
	Wave();
	sink get_data(uint32_t pos);
};

class SquareWave : public Wave {
public:
	SquareWave();
};

class SinWave : public Wave {
public:
	SinWave();
};

class Oscillator {
protected:
	Wave* wave = nullptr;
	sink position { 0.0f };
	float frequency = 440.0f;
	sink playRate { 0.0f };
	sink decayRate { 0.0f };
	sink RES_SINK { RESOLUTION };
	sink zero { 0.0f };
public:
	sink volume { 0.0f };
	Oscillator();
	void set_wave(Wave* w);
	void set_freq(float f);
	float get_freq();
	sink tick(bool fade);
};

#endif