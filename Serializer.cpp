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

response_t* SerialServerClass::add_response() {
	if (queue == NULL){
		queue = new response_t;
		queue->next = NULL;
		return queue;
	} else {
		response_t* resp = new response_t;
		response_t* last;
		response_t* next = queue;
		while (next != NULL) {
			last = next;
			next = (response_t*)last->next;
		}
		last->next = resp;
		resp->next = NULL;
		return resp;
	}
}

void SerialServerClass::add_response(char request[], void* response, uint16_t size) {
	response_t* resp = add_response();
	resp->request = request;
	resp->response = response;
	resp->size = size;
	if ((strlen(request)*2)>rec_buf_size)
		rec_buf_size = strlen(request)*2;
		if (rec_buf != NULL)
			delete rec_buf;
	if (rec_buf == NULL)
		rec_buf = new char[rec_buf_size];
}

void SerialServerClass::add_response(char request[], void (*function)(void)) {
	response_t* resp = add_response();
	resp->request = request;
	resp->size = 0;
	resp->function = function;
	if ((strlen(request)*2)>rec_buf_size)
		rec_buf_size = strlen(request)*2;
		if (rec_buf != NULL)
			delete rec_buf;
	if (rec_buf == NULL)
		rec_buf = new char[rec_buf_size];
}

periodical_t* SerialServerClass::add_periodical() {
	if (periodicals == NULL){
		periodicals = new periodical_t;
		periodicals->next = NULL;
		return periodicals;
	} else {
		periodical_t* per = new periodical_t;
		periodical_t* last;
		periodical_t* next = periodicals;
		while (next != NULL) {
			last = next;
			next = (periodical_t*)last->next;
		}
		last->next = per;
		per->next = NULL;
		return per;
	}
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
			// fix this for variable length footer
/*			if ((scratch == 0) && (i > 1)) {
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
			}*/
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
				if (strlen(rec_buf) > 0) {
					if (queue != NULL) {
						response_t* last;
						response_t* next = queue;
						while (next != NULL) {
							last = next;
							if (strcmp(last->request, rec_buf) == 0) {
								if (last->size == 0) {
									(*last->function)();
								} else {
									Serializer.pack(*ser, last->response, last->size);
								}
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