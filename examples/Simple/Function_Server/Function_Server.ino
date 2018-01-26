#include <Serializer.h>

void myfunction() {
	Serial.println("Test function");
}

void setup() {
	Serial.begin(57600);
	SerialServer.add_response("function", myfunction);
}

void loop() {
	SerialServer.handle_requests();
}