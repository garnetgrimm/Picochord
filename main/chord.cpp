#include "chord.h"

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
NoChord::NoChord() {
	notes[0] = 0;
	notes[1] = 0;
	notes[2] = 0;
}

//001
SevChord::SevChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 9;
}

//010
MinChord::MinChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 3;
	notes[2] = root + 7;
}

//011
MinSevChord::MinSevChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 3;
	notes[2] = root + 9;
}

//100
MajChord::MajChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 7;
}

//101
MajSevChord::MajSevChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 10;
}

//110
DimChord::DimChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 3;
	notes[2] = root + 6;
}

//111
AugChord::AugChord(int root) {
	notes[0] = root + 0;
	notes[1] = root + 4;
	notes[2] = root + 8;
}