#include <Serializer.h>

void myfunction() {
	Serial.println("Test function");
}

void myfunction2(char request[]) {
	Serial.println("recieved request:");
	Serial.println(request);
}

void setup() {
	Serial.begin(57600);
	SerialServer.add_response("function", myfunction);
	SerialServer.add_response("function2", myfunction2, true); // looks for strings beginning with "function2"
}

void loop() {
	SerialServer.handle_requests();
}