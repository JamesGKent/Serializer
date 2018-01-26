#include <Serializer.h>

char id[20] = "I'm a demo";
char test[20] = "Test data";

SerialServer server(Serial);

void setup() {
	Serial.begin(57600);
	// tell serial server to look for requests in the format:
	// (request string, variable to return, size of variable) // just use sizeof here
	server.add_response("ID?", id, sizeof(id));
	server.add_response("Test?", test, sizeof(test));
}

void loop() {
	server.handle_requests();
}