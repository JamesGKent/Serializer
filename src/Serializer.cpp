#include <Arduino.h>
#include <Serializer.h>

void _Serializer::pack(char buffer[], void* data, uint16_t size) {
    uint8_t checksum = 0;
	for (uint8_t i=0; i<headerlength; i++)
		buffer[i] = (char)255;
    memcpy(&buffer[headerlength], data, size);
    for (uint16_t i=headerlength; i<size+headerlength; i++)
        checksum ^= buffer[i];
    buffer[headerlength+size] = checksum;
	for (uint16_t i=headerlength+size+1; i<size+padding(); i++)
		buffer[i] = (char)0;
};

SerializerStatus _Serializer::unpack(char buffer[], void* data, uint16_t size) {
    uint16_t i;
    uint16_t index=0;
	// fix this for variable length header
    for (i=0; i<size+padding()-1; i++){ // check for header bytes
        if ((buffer[i] == (char)255) && (buffer[i+1] == (char)255)) {
            index = i+2;
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
		// fix this for variable length footer
		if ((buffer[index+1+size] == (char)0) && (buffer[index+2+size] == (char)0)) {
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
	return headerlength + checksumlength + footerlength;
}

_Serializer Serializer;

#if defined(HAVE_HWSERIAL0) || defined(HAVE_CDCSERIAL)
void _SerialServer::add_response(char request[], void* response, uint16_t size) {
	if (queue == NULL){
		queue = new response_t;
		queue->request = request;
		queue->response = response;
		queue->size = size;
		queue->next = NULL;
		rec_buf = new char[rec_buf_size];
	} else {
		response_t* resp = new response_t;
		resp->request = request;
		resp->response = response;
		resp->size = size;
		resp->next = NULL;
		
		response_t* last;
		response_t* next = queue;
		while (next != NULL) {
			last = next;
			next = (response_t*)last->next;
		}
		last->next = resp;
	}
}

bool _SerialServer::make_request(char request[], void* response, uint16_t size, uint32_t timeout) {
	Serial.write(request);
	Serial.write('\r');
	uint32_t start_time = millis();
	uint16_t tmp_buf_size = (size+Serializer.padding()) * 3;
	char tmp_buf[tmp_buf_size];
	memset(tmp_buf, 0, tmp_buf_size);
	uint16_t i=0;
	while ((millis() - start_time) < timeout) {
		if (Serial.available() > 0) {
			scratch = Serial.read();
			tmp_buf[i] = scratch;
			i++;
			if ((scratch == 0) && (i > 1)) {
				if (tmp_buf[i-2] == 0){ // footer conditions are met, try update variables
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

void _SerialServer::handle_requests() {
	while (Serial.available() > 0) {
		scratch = Serial.read();
		switch (scratch) {
			case '\r': // carraige return
			case '\n': // linefeed
				if (strlen(rec_buf) > 0) {
					if (queue != NULL) {
						response_t* last;
						response_t* next = queue;
						while (next != NULL) {
							last = next;
							if (strcmp(last->request, rec_buf) == 0) {
								char buf[last->size + Serializer.padding()];
								Serializer.pack(buf, last->response, last->size);
								Serial.write(buf, last->size + Serializer.padding());
							}
							next = (response_t*)last->next;
						}
					}
				}
				rec_index = 0;
				memset(rec_buf, 0, rec_buf_size);
				break;
			default:
				rec_buf[rec_index] = scratch;
				rec_index++;
				if (rec_index >= rec_buf_size) {
					rec_index = 0;
					memset(rec_buf, 0, rec_buf_size);
				}
		}
	}
}

_SerialServer SerialServer;
#endif

#if defined(HAVE_HWSERIAL1)
void _SerialServer1::add_response(char request[], void* response, uint16_t size) {
	if (queue == NULL){
		queue = new response_t;
		queue->request = request;
		queue->response = response;
		queue->size = size;
		queue->next = NULL;
		rec_buf = new char[rec_buf_size];
	} else {
		response_t* resp = new response_t;
		resp->request = request;
		resp->response = response;
		resp->size = size;
		resp->next = NULL;
		
		response_t* last;
		response_t* next = queue;
		while (next != NULL) {
			last = next;
			next = (response_t*)last->next;
		}
		last->next = resp;
	}
}

void _SerialServer1::handle_requests() {
	while (Serial1.available() > 0) {
		scratch = Serial1.read();
		switch (scratch) {
			case '\r': // carraige return
			case '\n': // linefeed
				if (strlen(rec_buf) > 0) {
					if (queue != NULL) {
						response_t* last;
						response_t* next = queue;
						while (next != NULL) {
							last = next;
							if (strcmp(last->request, rec_buf) == 0) {
								char buf[last->size + Serializer.padding()];
								Serializer.pack(buf, last->response, last->size);
								Serial1.write(buf, last->size + Serializer.padding());
							}
							next = (response_t*)last->next;
						}
					}
				}
				rec_index = 0;
				memset(rec_buf, 0, rec_buf_size);
				break;
			default:
				rec_buf[rec_index] = scratch;
				rec_index++;
				if (rec_index >= rec_buf_size) {
					rec_index = 0;
					memset(rec_buf, 0, rec_buf_size);
				}
		}
	}
}

_SerialServer1 SerialServer1;
#endif

#if defined(HAVE_HWSERIAL2)
void _SerialServer2::add_response(char request[], void* response, uint16_t size) {
	if (queue == NULL){
		queue = new response_t;
		queue->request = request;
		queue->response = response;
		queue->size = size;
		queue->next = NULL;
		rec_buf = new char[rec_buf_size];
	} else {
		response_t* resp = new response_t;
		resp->request = request;
		resp->response = response;
		resp->size = size;
		resp->next = NULL;
		
		response_t* last;
		response_t* next = queue;
		while (next != NULL) {
			last = next;
			next = (response_t*)last->next;
		}
		last->next = resp;
	}
}

void _SerialServer2::handle_requests() {
	while (Serial2.available() > 0) {
		scratch = Serial2.read();
		switch (scratch) {
			case '\r': // carraige return
			case '\n': // linefeed
				if (strlen(rec_buf) > 0) {
					if (queue != NULL) {
						response_t* last;
						response_t* next = queue;
						while (next != NULL) {
							last = next;
							if (strcmp(last->request, rec_buf) == 0) {
								char buf[last->size + Serializer.padding()];
								Serializer.pack(buf, last->response, last->size);
								Serial2.write(buf, last->size + Serializer.padding());
							}
							next = (response_t*)last->next;
						}
					}
				}
				rec_index = 0;
				memset(rec_buf, 0, rec_buf_size);
				break;
			default:
				rec_buf[rec_index] = scratch;
				rec_index++;
				if (rec_index >= rec_buf_size) {
					rec_index = 0;
					memset(rec_buf, 0, rec_buf_size);
				}
		}
	}
}

_SerialServer2 SerialServer2;
#endif

#if defined(HAVE_HWSERIAL3)
void _SerialServer3::add_response(char request[], void* response, uint16_t size) {
	if (queue == NULL){
		queue = new response_t;
		queue->request = request;
		queue->response = response;
		queue->size = size;
		queue->next = NULL;
		rec_buf = new char[rec_buf_size];
	} else {
		response_t* resp = new response_t;
		resp->request = request;
		resp->response = response;
		resp->size = size;
		resp->next = NULL;
		
		response_t* last;
		response_t* next = queue;
		while (next != NULL) {
			last = next;
			next = (response_t*)last->next;
		}
		last->next = resp;
	}
}

void _SerialServer3::handle_requests() {
	while (Serial3.available() > 0) {
		scratch = Serial3.read();
		switch (scratch) {
			case '\r': // carraige return
			case '\n': // linefeed
				if (strlen(rec_buf) > 0) {
					if (queue != NULL) {
						response_t* last;
						response_t* next = queue;
						while (next != NULL) {
							last = next;
							if (strcmp(last->request, rec_buf) == 0) {
								char buf[last->size + Serializer.padding()];
								Serializer.pack(buf, last->response, last->size);
								Serial3.write(buf, last->size + Serializer.padding());
							}
							next = (response_t*)last->next;
						}
					}
				}
				rec_index = 0;
				memset(rec_buf, 0, rec_buf_size);
				break;
			default:
				rec_buf[rec_index] = scratch;
				rec_index++;
				if (rec_index >= rec_buf_size) {
					rec_index = 0;
					memset(rec_buf, 0, rec_buf_size);
				}
		}
	}
}

_SerialServer3 SerialServer3;
#endif