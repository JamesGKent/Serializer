#include <Serializer.h>

SerialServer server(Serial);

void myfunction() {
	Serial.println("Test function");
}

void setup() {
	Serial.begin(57600);
	server.add_response("function", myfunction);
}

void loop() {
	server.handle_requests();
}