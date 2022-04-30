#include "keypad.h"
#include <stdio.h>

Keypad::Keypad() {
    for(int row = 0; row < rows; row++) {
        for(int col = 0; col < 16; col++) {
            set_keypad(row, col, 0);
        }
    }
	for(int col = 0; col < cols; col++) {
		colRank[col] = col;
	}
}

void Keypad::set_keypad(uint8_t row, uint8_t col, bool stat) {
	if(stat) {
		data[row] |= (1 << col);
	} else {
		data[row] &= ~(1 << col);
	}
}

bool Keypad::get_keypad(uint8_t row, uint8_t col) {
	return ((data[row] & (1 << col)) != 0);
}

void Keypad::col_rm_rank(int col) {
	uint8_t currRank = colRank[col];
	uint8_t maxRank = cols;
	for(int c = 0; c < cols; c++){
		if(colRank[c] > currRank) {
			if(colRank[c] > maxRank) maxRank = colRank[c];
			colRank[c]--;
		}
	}
	colRank[col] = maxRank-1;
}

void Keypad::col_add_rank(int col) {
	uint8_t currRank = colRank[col];
	for(int c = 0; c < cols; c++){
		if(colRank[c] < currRank) colRank[c]++;
	}
	colRank[col] = 0;
}

Chord Keypad::to_chord() {
	int col;
	for(col = 0; col < cols; col++) {
		if(colRank[col] == 0) break;
	}
	
	//get midi from note pressed
	int root = noteMap[col];
	int chordType = 0;
	
	//determine chord pressed for active note
	for(int row = 0; row < rows; row++) {
		if(get_keypad(row, col)) {
			chordType |= (1 << row);
		}
	}
	
	return Chord::makeChord(root, static_cast<ChordType>(chordType));
}


void Keypad::update(Keypad currentState) {
	for(int col = 0; col < cols; col++) {
		bool colNowActive = false;
		bool colWasActive = false;
		for(int row = 0; row < rows; row++) {
			colNowActive |= currentState.get_keypad(row, col);
			colWasActive |= get_keypad(row, col);
			set_keypad(row, col, currentState.get_keypad(row, col));
		}
		if(colNowActive && !colWasActive) col_add_rank(col);
		if(colWasActive && !colNowActive) col_rm_rank(col);
	}
}

void Keypad::print() {
	for(int row = 0; row < rows; row++) {
		for(int col = 0; col < cols; col++) {
			if(get_keypad(row, col)) {
				printf("1");
			} else {
				printf("0");
			}
		}
		printf("\r\n");
	}
}