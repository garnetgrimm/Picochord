#pragma once

#include "pico/stdlib.h"
#include "chord.h"

constexpr uint8_t rows = 3;
constexpr uint8_t cols = 6;
//                              Db  Ab  Eb  Bb  F   C   G   D   A   E   B   F#
constexpr uint8_t noteMap[] = { 49, 56, 51, 58, 53, 48, 55, 50, 57, 52, 59, 54 };

struct Keypad {
	
	//this works as long as cols is <= 16
	uint16_t data[rows];
	uint8_t colRank[cols];
	
	Keypad();
	void set_keypad(uint8_t row, uint8_t col, bool stat);
	bool get_keypad(uint8_t row, uint8_t col);
	void col_rm_rank(int col);
	void col_add_rank(int col);
	Chord to_chord();	
	void update(Keypad currentState);
	void print();
};