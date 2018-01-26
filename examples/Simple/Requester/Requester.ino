#include <Serializer.h>

char id[20] = {0};
char test[20] = {0};

void setup() {
	Serial.begin(57600);
}

void loop() {
	if (SerialServer.make_request("ID?", id, sizeof(id)))
		Serial.println(id);
	if (SerialServer.make_request("Test?", test, sizeof(test)))
		Serial.println(test);
	delay(1000);
}