#include <Serializer.h>

char id[20] = {0};
char test[20] = {0};

SerialServer server(Serial);

void setup() {
	Serial.begin(57600);
}

void loop() {
	if (server.make_request("ID?", &id, sizeof(id)))
		Serial.println(id);
	if (server.make_request("Test?", &test, sizeof(test)))
		Serial.println(test);
	delay(1000);
}