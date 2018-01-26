
enum SerializerStatus{
	OK,
	CHECKSUM_FAILED,
	FOOTER_MISSING,
	INCOMPLETE_PACKET
};

class _Serializer {
public:
    void pack(char buffer[], void* data, uint16_t size);
    SerializerStatus unpack(char buffer[], void* data, uint16_t size);
	
	uint8_t padding();
	
	uint8_t headerlength = 2;
	uint8_t footerlength = 2;
	
private:
	uint8_t checksumlength = 1;
};

extern _Serializer Serializer;

typedef struct {
	char* request;
	void* response;
	void (*function)(void);
	uint16_t size;
	void* next;
} response_t;

class SerialServerClass {
public:
	SerialServerClass(Stream &port);
	void add_response(char request[], void* response, uint16_t size);
	void add_response(char request[], void (*function)(void));
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
private:
	response_t* add_response(); // adds empty response to the queue
	
	Stream* ser; // reference to the port in use
	response_t* queue=NULL; // list of all the responses
	uint16_t rec_buf_size = 32; // initial buffer size
	char* rec_buf; // pointer to buffer
	
	uint16_t rec_index;
	char scratch;
};

#if defined(HAVE_HWSERIAL0) || defined(HAVE_CDCSERIAL)
extern SerialServerClass SerialServer;
#endif

#if defined(HAVE_HWSERIAL1)
extern SerialServerClass SerialServer1;
#endif

#if defined(HAVE_HWSERIAL2)
extern SerialServerClass SerialServer2;
#endif

#if defined(HAVE_HWSERIAL3)
extern SerialServerClass SerialServer3;
#endif