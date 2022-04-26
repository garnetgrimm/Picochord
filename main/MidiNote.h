#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <string>

using namespace std;

static constexpr int NOTES_PER_OCTAVE = 12;
static std::array<std::string, NOTES_PER_OCTAVE> NOTE_NAMES = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

class MidiNote {
	int noteNumber = 0;
public:	
	static int nameToNum(string midiName);
	static string numToName(int midiNote);

	static MidiNote findFirstGreater(MidiNote min, string type);

	MidiNote();
	MidiNote(string name);
	MidiNote(int num);
	MidiNote(MidiNote& other);
	MidiNote(const MidiNote& other);

	int getOctave();
	string getRawName();
	float getFrequency();

	friend MidiNote operator+(const MidiNote& a, const MidiNote& b);
	friend MidiNote operator-(const MidiNote& a, const MidiNote& b);
	friend bool operator<(const MidiNote& a, const MidiNote& b);
	friend bool operator==(const MidiNote& a, const MidiNote& b);

	operator int();
};

typedef vector<MidiNote> Chord;
typedef vector<Chord> DegreeList;