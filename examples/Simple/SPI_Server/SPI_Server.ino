#include <Serializer.h>
#include <SPIStream.h>

SPIStream spistream = SPIStream();
SerialServerClass spiserver(spistream);

void setup() {
  SPI.begin();
}

void loop() {
	
}
