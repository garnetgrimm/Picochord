#include "Scale.h"

MajScale::MajScale(int root) {
	for(int octave = 0; octave < 3; octave++) {
		notes[0 + octave*3] = MidiNote(root + 0 + octave*12);
		notes[1 + octave*3] = MidiNote(root + 4 + octave*12);
		notes[2 + octave*3] = MidiNote(root + 7 + octave*12);
	}
	notes[12] = MidiNote(root + 36);
}

MinScale::MinScale(int root) {
	for(int octave = 0; octave < 3; octave++) {
		notes[0 + octave*3] = MidiNote(root + 0 + octave*12);
		notes[1 + octave*3] = MidiNote(root + 3 + octave*12);
		notes[2 + octave*3] = MidiNote(root + 7 + octave*12);
	}
	notes[12] = MidiNote(root + 36);
}

RootScale::RootScale(int root) {
	for(int octave = 0; octave < 3; octave++) {
		notes[0 + octave*3] = MidiNote(root + 0 + octave*12);
		notes[1 + octave*3] = MidiNote(root + 0 + octave*12);
		notes[2 + octave*3] = MidiNote(root + 0 + octave*12);
	}
	notes[12] = MidiNote(root + 36);
}
