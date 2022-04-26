#pragma once

#include "MidiNote.h"

struct Scale {
	MidiNote notes[13];
};

struct MajScale: public Scale {
	MajScale(int root);
};

struct MinScale: public Scale {
	MinScale(int root);
};

struct RootScale: public Scale {
	RootScale(int root);
};