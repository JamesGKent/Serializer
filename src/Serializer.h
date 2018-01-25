
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

class SerialServer {
public:
	SerialServer(Stream* port=NULL);
	void add_response(char request[], void* response, uint16_t size);
	void add_response(char request[], void (*function)(void));
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
private:
	response_t* add_response();
	
	Stream* ser;
	response_t* queue=NULL;
	uint16_t rec_buf_size = 32;
	char* rec_buf;
	uint16_t rec_index;
	char scratch;
};