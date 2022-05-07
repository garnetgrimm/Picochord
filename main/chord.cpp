#include "chord.h"

Chord::Chord() {
	this->type = BLANK;
	this->root = 0;
}

Chord::Chord(int root, ChordType type) {
	this->type = type;
	this->root = root;
}

Chord Chord::makeChord(int root, ChordType type) {
	switch(type) {
		case MAJOR:
			return MajChord(root);
		case MINOR:
			return MinChord(root);
		case SEVEN:
			return SevChord(root);
		case MAJSEV:
			return MajSevChord(root);
		case MINSEV:
			return MinSevChord(root);
		case AUGMEN:
			return AugChord(root);
		case DIMIN:
			return DimChord(root);
		default:
			return NoChord();
	}
}

//000
NoChord::NoChord() : Chord() {
	notes[0] = 0;
	notes[1] = 0;
	notes[2] = 0;
}

//001
SevChord::SevChord(int root) : Chord(root, SEVEN) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 9;
}

//010
MinChord::MinChord(int root) : Chord(root, MINOR) {
	notes[0] = root + 0;
	notes[1] = root + 3;
	notes[2] = root + 7;
}

//011
MinSevChord::MinSevChord(int root) : Chord(root, MINSEV) {
	notes[0] = root + 0;
	notes[1] = root + 3;
	notes[2] = root + 9;
}

//100
MajChord::MajChord(int root) : Chord(root, MAJOR) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 7;
}

//101
MajSevChord::MajSevChord(int root) : Chord(root, MAJSEV) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 10;
}

//110
DimChord::DimChord(int root) : Chord(root, DIMIN) {
	notes[0] = root + 0;
	notes[1] = root + 3;
	notes[2] = root + 6;
}

//111
AugChord::AugChord(int root) : Chord(root, AUGMEN) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 8;
}