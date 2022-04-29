#pragma once

#include "pico/stdlib.h"

enum ChordType { BLANK, SEVEN, MINOR, MINSEV, MAJOR, MAJSEV, DIMIN, AUGMEN };

struct Chord {
	uint8_t notes[3];
	uint8_t operator [](int i) const { return notes[i]; }
	uint8_t & operator [](int i) { return notes[i]; }
	static Chord makeChord(int root, ChordType type);
};

struct MajChord: public Chord {
	MajChord(int root);
};

struct MinChord: public Chord {
	MinChord(int root);
};

struct SevChord: public Chord {
	SevChord(int root);
};

struct NoChord: public Chord {
	NoChord();
};

struct MajSevChord: public Chord {
	MajSevChord(int root);
};

struct MinSevChord: public Chord {
	MinSevChord(int root);
};

struct DimChord: public Chord {
	DimChord(int root);
};

struct AugChord: public Chord {
	AugChord(int root);
};