SerialWS, a library for the ESP32 providing Serial support over Websocket
See SerialWS.htm for an example of using a webpage as a serial monitor

Instantiation: Instantiate a SerialWS object globally. For example with SerialWS serialWS;

Configuration: Configuration is handled by the SerialWS.begin() method.  For example: serialWS.begin(&server, 1024, 64);

Parameters:
    *server is a pointer to an AsyncWebServer object.
    TxBufferSize is an unsigned int 16 indicating how many bytes the transmit/send buffer should hold. The size has been internally limited to 16K in SerialWS.h
    RxBufferSize is an unsigned int 16 indicating how many bytes the receive buffer should hold. The size has been internally limited to 16K in SerialWS.h.
    NOTE: If the receive buffer fills up, data will be lost as incoming new data cannot be stored.


Methods:

void end();
Use end() to stop SerialWS. The SerialWS websocket handler will be removed from the server and the Rx and Tx buffers will be released (memory returned to heap).

int available();
Returns the number of data bytes in the receive buffer.

int awaitingSend();
Returns the number of bytes in the transmission buffer awaiting transmission. Will return 0 if not buffering.

int availableForWrite();
Returns the number of bytes remaining in the transmit buffer. Will return 0 if not buffering.

int peek();
Returns the ASCII code of the next byte in the receive buffer. The byte remains in the buffer! Returns -1 if there is no data in the buffer.

int read();
Returns the ASCII code of the next byte in the receive buffer. The byte is removed from the buffer. Returns -1 if there is no data in the buffer.

int read(uint8_t *buffer, int size);
int read(char * buffer, int size)
Reads read up to size/length bytes from the receive buffer. Bytes are removed from the buffer. The number of bytes read will be returned.

void send();
Immediately sends any data that may be in the transmit buffer. Call this method once a series of prints have been finished.

void flush();              // Flushes (empties, clears) both the receive and transmit buffers.
void flushRecv();          // Flushes (empties, clears) the receive buffer.
void flushSend();          // Flushes (empties, clears) the transmit/send buffer.
Flushes (empties, clears) the buffer(s). No processing of any data will be done!

void cleanupClients();
Clean up old websockets clients. This is recommended/required by the ESPAsyncWebServer library. This function should be called periodically from within loop().

size_t write(uint8_t b);
Write a single byte.

Variants of writing a single byte using other than uint8_t numeric types are:
size_t write(unsigned long n){return write((uint8_t) n);}
size_t write(long n){return write((uint8_t) n);}
size_t write(unsigned int n){return write((uint8_t) n);}
size_t write(int n){return write((uint8_t) n);}

size_t write(uint8_t *buffer, size_t size);
Write a block of bytes.

Variants of block writing to handle data types other than uint8_t (char arrays, Strings, ...) as well as writing single bytes that are not uint8_t (write(i) where i is an integer, long, ...) are:
size_t write(const char * buffer, size_t size){return write((uint8_t*) buffer, size);}
size_t write(const char * s){return write((uint8_t*) s, strlen(s));}

Because SerialWS inherits from the standard Stream class, other commands are still valid, such as:
serialWS.printf("There are %i bytes waiting in the send buffer.", serialWs.awaitingSend());
Or, serialWS.println(1234); or serialWS.print(1234.56);

Just remember that you need to specifically send the data once you have printed everything to SerialWS!
When the buffer is full or when SerialWS.send() is executed, all the data in the send buffer will be broadcast to any/all clients with a websocket connection to SerialWS.