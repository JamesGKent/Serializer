#include <Serializer.h>

// this struct must be the same in the sending and recieving sketch
struct { // struct with all variables to be sent/recieved
	uint8_t unsigned8int;
	int16_t signed16int;
	char char_array[20]; // cannot use pointers here
} mydata; // create a global instance of it

void setup() {
	Serial.begin(57600);
	
	mydata.unsigned8int = 0;
	mydata.signed16int = 0;
	sprintf(mydata.char_array, "Test data"); // copy into array, rather than assign
}

void loop() {
	mydata.unsigned8int++; // increment
	mydata.signed16int+=10; // increment by 10

	Serializer.pack(Serial, &mydata, sizeof(mydata));
	delay (1000);
}