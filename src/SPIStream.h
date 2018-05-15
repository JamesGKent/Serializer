#include <SPI.h>

#if defined(ARDUINO_AVR_MEGA2560)
	#define CHIP_SELECT 53
#elif defined(ARDUINO_AVR_MICRO) || defined(ARDUINO_AVR_LEONARDO)
	#define CHIP_SELECT 17
#elif defined(ARDUINO_AVR_NANO) ||  defined(ARDUINO_AVR_UNO)
	#define CHIP_SELECT 10
#else 
	#warning no default chip select
#endif

class SPIStream : public Stream {
	public:
		SPIStream(SPIClass& spi=SPI, uint32_t clock=4000000, uint8_t bitOrder=MSBFIRST, uint8_t dataMode=SPI_MODE0, int cs=CHIP_SELECT):_spi(spi),_spisettings(clock, bitOrder, dataMode),_cs(cs){};
		size_t write(const uint8_t *buffer, size_t size);
		size_t write(uint8_t data);
		int available();
		int read();
		int peek();
	private:
		SPIClass& _spi;
		SPISettings _spisettings;
		int _cs;
};
