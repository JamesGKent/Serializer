#include <Serializer.h>

char id[20] = {0};
char test[20] = {0};

SerialServer srv(&Serial);

void setup() {
	Serial.begin(57600);
}

void loop() {
	if (srv.make_request("ID?", &id, sizeof(id)))
		Serial.println(id);
	if (srv.make_request("Test?", &test, sizeof(test)))
		Serial.println(test);
	delay(1000);
}