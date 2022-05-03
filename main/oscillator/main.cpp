#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "oscillator.h"

long uptime = 0;


bool count(struct repeating_timer *t) {
	uptime++;
	return true;
}

int main(void) {
	
    stdio_init_all();
	
	FOscillator fpulse;
	SOscillator spulse;
	const long REPEATS = 100000;
	
	struct repeating_timer timer;
	add_repeating_timer_ms(-1, count, NULL, &timer);
	
	int iteration = 0;
	
	long x = 0;
	
	sleep_ms(5000);
	
	while(1) {
		printf("\033[2J");
		printf("trial: %d\r\n", iteration);
		
		x = 0;
		uptime = 0;
		for(int i = 0; i < REPEATS; i++) {
			x += fpulse.tick(true);
		}
		printf("Time taken by float: \r\n");
		printf("%lu ms (%f us per)\r\n", uptime, uptime*1000/(float)REPEATS);
		printf("result - %lu\r\n", x);
		
		x = 0;
		uptime = 0;
		for(int i = 0; i < REPEATS; i++) {
			x += spulse.tick(true);
		}
		printf("Time taken by sink: \r\n");
		printf("%lu ms (%f us per)\r\n", uptime, uptime*1000/(float)REPEATS);
		printf("result - %lu\r\n", x);
		
		iteration++;
		sleep_ms(5000);
	}
}
