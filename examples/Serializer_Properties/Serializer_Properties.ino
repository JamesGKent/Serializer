#include <Serializer.h>

void setup() {
	Serial.begin(57600);
	uint8_t headlen = Serializer.headerlength(); // get current headerlength, default 2 (recommended minimum)
	Serializer.headerlength(5); // set new headerlength
	uint8_t footlen = Serializer.footerlength(); // get current footerlength, default 2 (recommended minimum)
	Serializer.footerlength(5); // set new footerlength
}

void loop() {
	
}