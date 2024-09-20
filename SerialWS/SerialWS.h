#pragma once
#include "Arduino.h"
#include <ESPAsyncWebServer.h>    // Provides asynchronous web server support for up to 8 clients

#define SerialWSmaxBuffer 16384   // Maximum buffer size for transmit and receive buffers.

class SerialWS: public Stream{
public:
    SerialWS();

    // begin must be called to initiate SerialWS.
    // AsyncWebServer *server: A pointer to the main webserver SerialWS will be hosted on.
    // uint16_t RxBufferSize: Size, in bytes, for the receive buffer. Data received via SerialWS will be placed here for later peek, read, etc.
    // uint16_t TxBufferSize: Size, in bytes, for the transmit buffer. Transmits are usually buffered in order to prevent multiple mini writes.
    void begin(AsyncWebServer *server, uint16_t TxBufferSize = 256, uint16_t RxBufferSize = 256);

    // end can be called to stop SerialWS
    // SerialWS websocket handler will be removed from the server and
    // the Rx and Tx buffers will be released (memory returned to heap)
    void end();

    // Returns the number of bytes in the receive buffer
    int available(void);

    // Returns the number of bytes in the transmission buffer awaiting transmission. Will return 0 if not buffering.
    int awaitingSend(void);

    // Returns the number of bytes remaining in the transmit buffer. Will return 0 if not buffering.
    int availableForWrite(void);

    // Returns the ASCII code of the next byte in the receive buffer. The byte remains in the buffer! Returns -1 if there is no byte in the buffer.
    int peek(void);

    int read(void);
    // Returns the ASCII code of the next byte in the receive buffer. The byte is removed from the buffer. Returns -1 if there is no byte in the buffer.
    int read(uint8_t *buffer, int size);

    // Reads read up to size/length bytes from the receive buffer. Bytes are removed from the buffer. The number of bytes read will be returned.
    int read(char * buffer, int size){
        return read((uint8_t*) buffer, size);
    }

    // Sends any data that may be in the transmit buffer
    void send();

    // Clears the buffer(s). No processing of any data will be done!
    void flush(void);
    void flushRecv(void);
    void flushSend(void);

    // Clean up old clients. This is required by the ESPAsyncWebServer library...
    void cleanupClients(void);

    // Write a single byte
    size_t write(uint8_t);

    // Write a block of bytes
    size_t write(uint8_t *buffer, size_t size);

    // Variants of block writing to handle data types other than uint8_t (char arrays, Strings, ...)
    // as well as writing single bytes that are not uint8_t (write(i) where i is an integer, long, ...)
    inline size_t write(const char * buffer, size_t size){return write((uint8_t*) buffer, size);}
    inline size_t write(const char * s){return write((uint8_t*) s, strlen(s));}
    inline size_t write(unsigned long n){return write((uint8_t) n);}
    inline size_t write(long n){return write((uint8_t) n);}
    inline size_t write(unsigned int n){return write((uint8_t) n);}
    inline size_t write(int n){return write((uint8_t) n);}

private:
    void checkBuffer();                                      // Used internally. Checks if buffer is full and sends it if it is.
    size_t _txBufferSize;                                    // Size of Tx buffer (transmit buffer). Should always be a multiple of 4
    volatile size_t _tx_buffer_head;                         // Index of Tx buffer head (Where next byte to be transmitted will be stored)
    volatile size_t _tx_buffer_tail;                         // Index of Tx buffer tail (Where next byte to be transmitted from buffer will be read)
    unsigned char * _tx_buffer;                              // Pointer to Tx buffer of size _txBufferSize
    static void onSerialWsEvent(AsyncWebSocket * , AsyncWebSocketClient * , AwsEventType , void * , uint8_t *, size_t );
};
