#include <Serializer.h>

char message[20] = "Test message";

void setup() {
	Serial.begin(57600);
	SerialServer.send_periodically(message, sizeof(message), 2500);
}

void loop() {
	SerialServer.handle_requests();
}