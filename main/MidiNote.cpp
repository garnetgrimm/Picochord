#include <algorithm>
#include <math.h>
#include "MidiNote.h"

int MidiNote::nameToNum(string midiName) {
	string nameStr;
	string octStr;
	for(int i = 0; i < midiName.size(); i++) {
		if(isdigit(midiName[i])) {
			octStr += midiName[i];
		} else {
			nameStr += midiName[i];
		}
	}
	auto it = find(NOTE_NAMES.begin(), NOTE_NAMES.end(), nameStr);
	if(it != NOTE_NAMES.end()) {
		int index = it - NOTE_NAMES.begin();
		return index + (stoi(octStr) + 1) * 12;
	} else {
		return -1;
	}
}

MidiNote MidiNote::findFirstGreater(MidiNote min, string type) {
	while (min.getRawName().compare(type) != 0) min = min + MidiNote(1);
	return min;
}

string MidiNote::numToName(int midiNote) {
	return NOTE_NAMES[midiNote % 12] + to_string((midiNote / 12) - 1);
}

int MidiNote::getOctave() {
	return (noteNumber / 12) - 1;
}

string MidiNote::getRawName() {
	return NOTE_NAMES[noteNumber % 12];
}

float MidiNote::getFrequency() {
	return 440.0f*powf(2.0f, ((float)noteNumber - 69.0f) / 12.0f);
}

MidiNote::MidiNote() {
	noteNumber = 60;
}

MidiNote::MidiNote(string name) {
	noteNumber = nameToNum(name);
}

MidiNote::MidiNote(int num) {
	noteNumber = num;
}

MidiNote::MidiNote(MidiNote& other) {
	noteNumber = other.noteNumber;
}

MidiNote::MidiNote(const MidiNote& other) {
	this->noteNumber = other.noteNumber;
}

MidiNote operator+(const MidiNote& a, const MidiNote& b) {
	return MidiNote(a.noteNumber + b.noteNumber);
}

MidiNote operator-(const MidiNote& a, const MidiNote& b) {
	return MidiNote(a.noteNumber - b.noteNumber);
}

bool operator<(const MidiNote& a, const MidiNote& b)
{
	return a.noteNumber < b.noteNumber;
}

bool operator==(const MidiNote& a, const MidiNote& b)
{
	return a.noteNumber == b.noteNumber;
}

MidiNote::operator int() {
	return noteNumber;
}