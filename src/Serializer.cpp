#include <Arduino.h>
#include <Serializer.h>

void _Serializer::pack(Stream &port, void* data, uint16_t size) {
	uint8_t checksum = 0;
	for (uint8_t i=0; i<headlength; i++)
		port.write((char)255);
	char* buf = (char*)data;
	port.write(buf, size);
	for (uint16_t i=0; i<size; i++) {
		checksum ^= buf[i];
	}
	port.write(checksum);
	for (uint8_t i=0; i<footlength; i++)
		port.write((uint8_t)0);
}

void _Serializer::pack(char buffer[], void* data, uint16_t size) {
    uint8_t checksum = 0;
	for (uint8_t i=0; i<headlength; i++)
		buffer[i] = (char)255;
    memcpy(&buffer[headlength], data, size);
    for (uint16_t i=headlength; i<size+headlength; i++)
        checksum ^= buffer[i];
    buffer[headlength+size] = checksum;
	for (uint16_t i=headlength+size+1; i<size+padding(); i++)
		buffer[i] = (char)0;
};

SerializerStatus _Serializer::unpack(char buffer[], void* data, uint16_t size) {
	uint16_t i, j;
	uint16_t index=0;
	bool headervalid;
	for (i=0; i<size+padding()-1; i++){ // check for header bytes
		headervalid = true;
		for (j=0; j<headlength; j++) {
			if (buffer[i+j] != (char)255) {
				headervalid = false;
				break;
			}
		}
		if (headervalid) {
			index = i+headlength;
			break;
		}
	}
	if (index==0)
		return INCOMPLETE_PACKET;
	uint8_t checksum=0;
	for (uint16_t i=index; i<size+index; i++){
		checksum ^= buffer[i];
	}

	if (checksum == buffer[index+size]) {
		bool footervalid = true;
		for (j=index+size+1; j<index+size+1+footlength; j++) {
			if (buffer[j] != (char)0) {
				footervalid = false;
				break;
			}
		}
		if (footervalid) {
			memcpy(data, &buffer[index], size);
			return OK;
		}
		return FOOTER_MISSING;
	} else if (buffer[index+size] == (char)0){
		return INCOMPLETE_PACKET;
	}
	return CHECKSUM_FAILED;
}

uint8_t _Serializer::padding() {
	return headlength + checksumlength + footlength;
}

void _Serializer::headerlength(uint8_t headerlength) {
	headlength = headerlength;
}

uint8_t _Serializer::headerlength() {
	return headlength;
}

void _Serializer::footerlength(uint8_t footerlength) {
	footlength = footerlength;
}

uint8_t _Serializer::footerlength() {
	return footlength;
}

_Serializer Serializer;

SerialServerClass::SerialServerClass(Stream &port) {
	ser = &port;
}

void SerialServerClass::resize_recieve_buffer(char request[]) {
	uint16_t len = strlen(request);
	if ((len*2)>rec_buf_size) {
		rec_buf_size = len*2;
		if (rec_buf != NULL)
			delete rec_buf;
	}
	if (rec_buf == NULL)
		rec_buf = new char[rec_buf_size];
}

void SerialServerClass::resize_recieve_buffer(uint16_t size) {
	rec_buf_size = size;
	if (rec_buf != NULL)
		delete rec_buf;
	if (rec_buf == NULL)
		rec_buf = new char[rec_buf_size];
}

response_t* SerialServerClass::_add_response(char request[], uint16_t size, bool startswith) {
	response_t* resp = new response_t;
	resp->request = request;
	resp->size = size;
	resp->startswith = startswith;
	resp->voidfunction = NULL;
	resp->charfunction = NULL;
	resp->next = NULL;
	if (queue == NULL){
		queue = resp;
	} else {
		response_t* last;
		response_t* next = queue;
		while (next != NULL) {
			last = next;
			next = (response_t*)last->next;
		}
		last->next = resp;
	}
	return resp;
}

void SerialServerClass::add_response(char request[], void* response, uint16_t size, bool startswith) {
	response_t* resp = _add_response(request, size, startswith);
	resp->response = response;
	resize_recieve_buffer(request);
}

void SerialServerClass::add_response(char request[], void (*function)(void), bool startswith) {
	response_t* resp = _add_response(request, 0, startswith);
	resp->voidfunction = function;
	resize_recieve_buffer(request);
}

void SerialServerClass::add_response(char request[], void (*function)(char *), bool startswith) {
	response_t* resp = _add_response(request, 0, startswith);
	resp->charfunction = function;
	resize_recieve_buffer(request);
}

periodical_t* SerialServerClass::add_periodical() {
	periodical_t* per = new periodical_t;
	per->next = NULL;
	if (periodicals == NULL){
		periodicals = per;
	} else {
		periodical_t* last;
		periodical_t* next = periodicals;
		while (next != NULL) {
			last = next;
			next = (periodical_t*)last->next;
		}
		last->next = per;
	}
	return per;
}

bool SerialServerClass::make_request(char request[], void* response, uint16_t size, uint32_t timeout) {
	ser->write(request);
	ser->write('\r');
	return recieve(response, size, timeout);
}

bool SerialServerClass::send_periodically(void* response, uint16_t size, uint32_t period) {
	periodical_t* per = add_periodical();
	per->response = response;
	per->size = size;
	per->period = period;
	per->last_sent = 0;
}

bool SerialServerClass::send(void* response, uint16_t size) {
	Serializer.pack(*ser, response, size);
}

bool SerialServerClass::recieve(void* response, uint16_t size, uint32_t timeout) {
	uint32_t start_time = millis();
	uint16_t tmp_buf_size = (size+Serializer.padding()) * 3;
	char tmp_buf[tmp_buf_size];
	memset(tmp_buf, 0, tmp_buf_size);
	uint16_t i=0;
	while ((millis() - start_time) < timeout) {
		if (ser->available() > 0) {
			scratch = ser->read();
			tmp_buf[i] = scratch;
			i++;
			if (i > Serializer.footerlength()) {
				bool footervalid = true;
				for (uint16_t j=0; j<Serializer.footerlength(); j++) {
					if (tmp_buf[i-j] != 0) {
						footervalid = false;
					}
				}
				if (footervalid) {
					// this command only updates the object after header, checksum and footer are verified
					// so object can be reasonably assumed to always contain valid data
					int status = Serializer.unpack(tmp_buf, response, size);
					switch (status){
					case OK:
						return true;
					case INCOMPLETE_PACKET: // means checksum in buffer is 0 but calculated isn't. indicates not recieved whole packet yet
						break;
					case CHECKSUM_FAILED:
						return false;
					case FOOTER_MISSING:
						return false;
					}
					if (i >= tmp_buf_size) {
						return false;
					}
				}
			}
		}
	}
	return false;
}

void SerialServerClass::handle_requests() {
	while (ser->available() > 0) {
		scratch = ser->read();
		switch (scratch) {
		case '\r': // carraige return
		case '\n': // linefeed
			rec_buf[rec_index] = (char)0;
			if (strlen(rec_buf) > 0) {
				if (queue != NULL) {
					response_t* last;
					response_t* next = queue;
					while (next != NULL) {
						last = next;
						if (last->startswith) {
							if (strncmp(last->request, rec_buf, strlen(last->request)) == 0) {
								if (last->size == 0) {
									if (last->charfunction != NULL)
										(*last->charfunction)(rec_buf);
									else if (last->voidfunction != NULL)
										(*last->voidfunction)();
								} else {
									Serializer.pack(*ser, last->response, last->size);
								}
								break;
							}
						} else {
							if (strcmp(last->request, rec_buf) == 0) {
								if (last->size == 0) {
									if (last->charfunction != NULL)
										(*last->charfunction)(rec_buf);
									else if (last->voidfunction != NULL)
										(*last->voidfunction)();
								} else {
									Serializer.pack(*ser, last->response, last->size);
								}
								break;
							}
						}
						next = (response_t*)last->next;
					}
				}
			}
			rec_index = 0;
			break;
		case '\b':
			if (_obey_backspace) {
				if (rec_index > 0) // prevent underflow
					rec_index--;
				rec_buf[rec_index] = (char)0;
				break;
			} // else flow into default
		default:
			rec_buf[rec_index] = scratch;
			rec_index++;
			if (rec_index >= rec_buf_size)
				rec_index = 0;
		}
	}
	periodical_t* last;
	periodical_t* next = periodicals;
	while (next != NULL) {
		last = next;
		if ((millis() - last->last_sent) > last->period) {
			Serializer.pack(*ser, last->response, last->size);
			last->last_sent = millis();
		};
		next = (periodical_t*)last->next;
	}
}

bool SerialServerClass::obey_backspace() {
	return _obey_backspace;
}

void SerialServerClass::obey_backspace(bool obey) {
	_obey_backspace = obey;
}

void SerialServerClass::list_responses() {
	response_t* last;
	response_t* next = queue;
	ser->println("Listing responses:");
	while (next != NULL) {
		last = next;
		ser->print("request: ");
		ser->println(last->request);
		next = (response_t*)last->next;
	}
}

#if defined(HAVE_HWSERIAL0) || defined(HAVE_CDCSERIAL)
SerialServerClass SerialServer(Serial);
#endif

#if defined(HAVE_HWSERIAL1)
SerialServerClass SerialServer1(Serial1);
#endif

#if defined(HAVE_HWSERIAL2)
SerialServerClass SerialServer2(Serial2);
#endif

#if defined(HAVE_HWSERIAL3)
SerialServerClass SerialServer3(Serial3);
#endif