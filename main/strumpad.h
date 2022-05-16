#pragma once

#include "pico/stdlib.h"

constexpr uint8_t pads = 8;

struct Strumpad {
	
	//this works as long as cols is <= 16
	uint16_t data;
	
	Strumpad();
	void set_strumpad(uint8_t pad, bool stat);
	bool get_strumpad(uint8_t pad);
	void print();
};