#include "strumpad.h"
#include <stdio.h>

Strumpad::Strumpad() {
    data = 0;
}

void Strumpad::set_strumpad(uint8_t pad, bool stat) {
	if(stat) {
		data |= (1 << pad);
	} else {
		data &= ~(1 << pad);
	}
}

bool Strumpad::get_strumpad(uint8_t pad) {
	return ((data & (1 << pad)) != 0);
}

void Strumpad::print() {
	for(int pad = 0; pad < pads; pad++) {
		if(get_strumpad(pad)) {
			printf("1");
		} else {
			printf("0");
		}
	}
	printf("\r\n");
}