#include <Serializer.h>

SerialServer srv(&Serial);

void myfunction() {
	Serial.println("Test function");
}

void setup() {
	Serial.begin(57600);
	srv.add_response("function", myfunction);
}

void loop() {
	srv.handle_requests();
}