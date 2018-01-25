
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

#if defined(HAVE_HWSERIAL0) || \
	defined(HAVE_CDCSERIAL) || \
	defined(HAVE_HWSERIAL1) || \
	defined(HAVE_HWSERIAL2) || \
	defined(HAVE_HWSERIAL3)
typedef struct {
	char* request;
	void* response;
	void* function;
	uint16_t size;
	void* next;
} response_t;

class _SerialServerBase {
public:
	response_t* add_response();
	void add_response(char request[], void* response, uint16_t size);
	void add_response(char request[], void* function);
	
	response_t* queue=NULL;
	
	uint16_t rec_buf_size = 32;
	char* rec_buf;
	uint16_t rec_index;
	char scratch;
};
#endif

#if defined(HAVE_HWSERIAL0) || \
	defined(HAVE_CDCSERIAL)
class _SerialServer : public _SerialServerBase {
public:
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
};

extern _SerialServer SerialServer;
#endif

#if defined(HAVE_HWSERIAL1)
class _SerialServer1 : public _SerialServerBase {
public:
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
};

extern _SerialServer1 SerialServer1;
#endif

#if defined(HAVE_HWSERIAL2)
class _SerialServer2 : public _SerialServerBase {
public:
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
};

extern _SerialServer2 SerialServer2;
#endif

#if defined(HAVE_HWSERIAL3)
class _SerialServer3 : public _SerialServerBase {
public:
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
};

extern _SerialServer3 SerialServer3;
#endif