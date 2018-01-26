# Serializer
arduino serialization library for transfering variables between arduinos

this library implements two classes:  
### _Serializer (default instance is Serializer)
which has the methods pack and unpack
these can be used directly (look at the advanced examples)
however the suggested way is to use the second class.

### SerialServerClass
this takes a serial class in its constructor:
```
SerialServerClass server(Serial);
```
So it can be used on any serial port.
default instances will be created to match the serial objects available:
```
SerialServer
SerialServer1
SerialServer2
SerialServer3
```
Also works with AltSoftSerial (see the simple AltSoftSerial_Server example)  
It may also work with SoftwareSerial, NeoSWSerial but these are not tested.
#### Methods:
##### add_response method:
this has two overloads, one that will send a reply and one that calls a function when that sequence is recieved.
```
void add_response(char request[], void* response, uint16_t size);
```
where the first argument is the string that should be recieved (terminated by carraige return, this is added by library)  
the second is a pointer to the object you want to send.
and the third is it's size (the size of a pointer will always be the same, so we need the size of the actual object)

to use is as simple as:
```
char id[20] = "I'm a demo"; // note that this must be a global variable and cannot itself be a pointer
SerialServer.add_response("ID?", id, sizeof(id));
```

to send more complex data or several variables at once:
```
struct {
uint8_t var1;
int32_t var2;
char array[50];
} data;
SerialServer.add_response("Data?", data, sizeof(data));
```
so this automatically takes into account the size of the object to be sent.

and to recieve simply declare the variable in exactly the same way and call make_request:
```
struct {
uint8_t var1;
int32_t var2;
char array[50];
} data;
bool succeeded = SerialServer.make_request("Data?", data, sizeof(data));
```
there is also an optional timeout argumentat the end set to a default of 1000ms
