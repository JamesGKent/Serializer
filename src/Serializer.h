
enum SerializerStatus{
	OK,
	CHECKSUM_FAILED,
	FOOTER_MISSING,
	INCOMPLETE_PACKET
};

class _Serializer {
public:
	void pack(Stream &port, void* data, uint16_t size);
    void pack(char buffer[], void* data, uint16_t size);
    SerializerStatus unpack(char buffer[], void* data, uint16_t size);
	
	uint8_t padding();
	
	void headerlength(uint8_t headerlength);
	uint8_t headerlength();
	void footerlength(uint8_t footerlength);
	uint8_t footerlength();
	
private:
	uint8_t headlength = 2;
	uint8_t footlength = 2;
	uint8_t checksumlength = 1;
};

extern _Serializer Serializer;

typedef struct {
	int8_t ID;
	char* request;
	bool startswith;
	void* response;
	void (*voidfunction)(void);
	void (*charfunction)(char *);
	uint16_t size;
	bool enabled;
	void* next;
} response_t;

typedef struct {
	void* response;
	uint16_t size;
	uint32_t period;
	uint32_t last_sent;
	void* next;
} periodical_t;

class SerialServerClass {
public:
	SerialServerClass(Stream &port);
	uint8_t add_response(const char request[], void* response, uint16_t size, bool startswith=false, bool enabled=true);
	uint8_t add_response(char request[], void* response, uint16_t size, bool startswith=false, bool enabled=true);
	uint8_t add_response(const char request[], void (*function)(void), bool startswith=false, bool enabled=true);
	uint8_t add_response(char request[], void (*function)(void), bool startswith=false, bool enabled=true);
	uint8_t add_response(const char request[], void (*function)(char *), bool startswith=false, bool enabled=true);
	uint8_t add_response(char request[], void (*function)(char *), bool startswith=false, bool enabled=true);
	void enable_response(uint8_t ID, bool enabled);
	uint8_t num_responses();
	bool make_request(const char request[], void* response, uint16_t size, uint32_t timeout=1000);
	bool make_request(char request[], void* response, uint16_t size, uint32_t timeout=1000);
	bool send_periodically(void* response, uint16_t size, uint32_t period);
	bool send(void* response, uint16_t size);
	bool recieve(void* response, uint16_t size, uint32_t timeout=1000);
	void handle_requests();
	void list_responses();
	bool obey_backspace();
	void obey_backspace(bool obey=false);
	void resize_recieve_buffer(uint16_t size);
private:
	void resize_recieve_buffer(char request[]);
	response_t* _add_response(char request[], uint16_t size, bool startswith, bool enabled); // adds empty response to the queue
	
	periodical_t* add_periodical(); 
	
	bool _obey_backspace=false;
	
	Stream* ser; // reference to the port in use
	response_t* queue=NULL; // list of all the responses
	periodical_t* periodicals=NULL; // list of messages to send periodically
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