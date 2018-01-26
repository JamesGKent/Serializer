#include <AltSoftSerial.h>
#include <Serializer.h>

AltSoftSerial AltSerial;
SerialServerClass server(AltSerial);

void myfunction() {
	AltSerial.println("Test function");
}

void setup() {
	AltSerial.begin(19200);
	server.add_response("function", myfunction);
}

void loop() {
	server.handle_requests();
}
