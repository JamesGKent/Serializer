#include <Serializer.h>

struct { // struct with all variables to be sent/recieved
	uint8_t unsigned8int;
	int16_t signed16int;
	char char_array[20];
} mydata; // create a global instance of it

void setup() {
	Serial.begin(57600);
}

// used for sending/recieving
#define BUFFER_SIZE 128
char buf[BUFFER_SIZE];
uint8_t index;
char scratch;

void loop() {
	// read byte by byte until footer is recieved (0,0) so other functions can happen
	// gives the illusion of multi tasking
	if (Serial.available() > 0){
		scratch = Serial.read();
		buf[index] = scratch;
		index++;
		// this needs rework to account for variable footer size
		if ((scratch == 0) && (index > 1)) {
			if (buf[index-2] == 0){ // footer conditions are met, try update variables
				// this command only updates the object after header, checksum and footer are verified
				// so object can be reasonably assumed to always contain valid data
				int status = Serializer.unpack(buf, &mydata, sizeof(mydata));
				switch (status){
					case OK:
					Serial.print("unsigned8: "); Serial.println(mydata.unsigned8int);
					Serial.print("signed16: "); Serial.println(mydata.signed16int);
					Serial.print("char array: "); Serial.println(mydata.char_array);
					index = 0;
					memset(buf, 0, BUFFER_SIZE);
					break;
				case CHECKSUM_FAILED:
					Serial.println("Failed checksum");
					index = 0;
					memset(buf, 0, BUFFER_SIZE);
					break;
				case FOOTER_MISSING: // means packets clashed/corrupted
				case INCOMPLETE_PACKET: // means checksum in buffer is 0 but calculated isn't. indicates not recieved whole packet yet
					break;
				}
			}
		} else {
			if (index >= BUFFER_SIZE-1) { // stop overflow
				index = 0;
				memset(buf, 0, BUFFER_SIZE);
			}
		}
	}
	// we can use the values in the struct here
}