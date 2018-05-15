#include <SPIStream.h>

size_t SPIStream::write(const uint8_t *buffer, size_t size) {
	size_t n = 0;
	_spi.beginTransaction(_spisettings);
	digitalWrite(_cs, LOW);
	while (size--) {
		if (write(*buffer++)) n++;
		else break;
	}
	digitalWrite(_cs, HIGH);
	_spi.endTransaction();
	return n;
}

size_t SPIStream::write(uint8_t data) {
	_spi.transfer(data);
	return 1;
}

int SPIStream::available() {
	return 0;
}

int SPIStream::read() {
	return 0;
}

int SPIStream::peek() {
	return 0;
}
