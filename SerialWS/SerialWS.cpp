#include "Arduino.h"
#include <ESPAsyncWebServer.h>                 // Provides asynchronous web server support for up to 8 clients
#include "SerialWS.h"

SerialWS::SerialWS(){}

AsyncWebServer *_server;                       // Server on which SerialWS is being served
AsyncWebSocket _ws("/SerialWS");               // Websocket on which SerialWS will be served
size_t _rxBufferSize;                          // Size of Rx buffer (receive buffer). Should always be a multiple of 4
unsigned char * _rx_buffer;                    // Pointer to Rx buffer of size _rxBufferSize
volatile size_t _rx_buffer_head;               // Index of Rx buffer head (Where next received byte will be stored)
volatile size_t _rx_buffer_tail;               // Index of Rx buffer tail (Where next byte from buffer will be read)

// begin must be called to initiate SerialWS.
// AsyncWebServer *server: A pointer to the main webserver SerialWS will be hosted on.
// uint16_t RxBufferSize: Size, in bytes, for the receive buffer. Data received via SerialWS will be placed here for later peek, read, etc.
// uint16_t TxBufferSize: Size, in bytes, for the transmit buffer. Transmits are usually buffered in order to prevent multiple mini writes.
void SerialWS::begin(AsyncWebServer *server, uint16_t TxBufferSize, uint16_t RxBufferSize){
  _rxBufferSize   = RxBufferSize;              // Size of Rx buffer (receive buffer). Should always be a multiple of 4
  _txBufferSize   = TxBufferSize;              // Size of Tx buffer (transmit buffer). Should always be a multiple of 4
  _rx_buffer_head = 0;                         // Index of Rx buffer head (Where next received byte will be stored)
  _rx_buffer_tail = 0;                         // Index of Rx buffer tail (Where next byte from buffer will be read)
  _tx_buffer_head = 0;                         // Index of Tx buffer head (Where next byte to be transmitted will be stored)
  _tx_buffer_tail = 0;                         // Index of Tx buffer tail (Where next byte to be transmitted from buffer will be read)
  _timeout = 1;                                // The Serial class has a timeout for receiving bytes. We don't need it because we receive in blocks.
 
  if(_rxBufferSize > SerialWSmaxBuffer)_rxBufferSize = SerialWSmaxBuffer;
  if(_txBufferSize > SerialWSmaxBuffer)_txBufferSize = SerialWSmaxBuffer;

  if(_rxBufferSize  > 0) _rx_buffer = (unsigned char*)malloc(_rxBufferSize);     // Pointer to Rx buffer of size _rxBufferSize
  if(_txBufferSize  > 0) _tx_buffer = (unsigned char*)malloc(_txBufferSize);     // Pointer to Tx buffer of size _txBufferSize
  _server = server;                                              // Server on which SerialWS is being served
  _ws.onEvent(onSerialWsEvent);                                  // Server will call function onSerialWsEvent when SerialWS websocket events occur
  _server->addHandler(&_ws);
}

// end can be called to stop SerialWS
// SerialWS websocket handler will be removed from the server,
// the Rx and Tx buffers will be released (memory returned to heap)
// and all internal variables reset
void SerialWS::end(){
  _server->removeHandler(&_ws);                                    // Remove the SerialWS websocket handler from the server
  if(_rxBufferSize  > 0) free(_rx_buffer);                         // Release Rx buffer memory back to the heap
  if(_txBufferSize  > 0) free(_tx_buffer);                         // Release Tx buffer memory back to the heap
  _txBufferSize = 0;
  _rxBufferSize = 0;
  _rx_buffer_head = 0;
  _rx_buffer_tail = 0;
  _tx_buffer_head = 0;
  _tx_buffer_tail = 0;
}

// Returns the number of bytes in the receive buffer
int SerialWS::available(void){
  int retVal = 0;
  if(_rxBufferSize > 0)
    retVal = (unsigned int)(_rxBufferSize + _rx_buffer_head - _rx_buffer_tail) % _rxBufferSize;
  return retVal;
}

// Returns the number of bytes in the transmission buffer awaiting transmission. Will return 0 if not buffering.
int SerialWS::awaitingSend(void){
  int retVal = 0;
  if(_txBufferSize > 0)
    retVal = (unsigned int)(_txBufferSize + _tx_buffer_head - _tx_buffer_tail) % _txBufferSize;
  return retVal;
}

// Returns the number of bytes remaining in the transmit buffer. Will return 0 if not buffering.
int SerialWS::availableForWrite(void){
  int retVal = 0;
  if(_txBufferSize > 0)
    retVal = _txBufferSize - awaitingSend();
  return retVal;
}

// Returns the ASCII code of the next byte in the receive buffer. The byte remains in the buffer! Returns -1 if there is no byte in the buffer.
int SerialWS::peek(void){
  int retVal = -1;
  if((_rxBufferSize > 0) && !(_rx_buffer_head == _rx_buffer_tail)){
    retVal = (int)_rx_buffer[_rx_buffer_tail];
  }
  return retVal;
}

// Returns the ASCII code of the next byte in the receive buffer. The byte is removed from the buffer. Returns -1 if there is no byte in the buffer.
int SerialWS::read(void){
  int retVal = peek();
  if(retVal >= 0)
    _rx_buffer_tail = (_rx_buffer_tail + 1) % _rxBufferSize;
  return retVal;
}

// Reads up to size/length bytes from the receive buffer. Bytes are removed from the buffer. The number of bytes read will be returned.
int SerialWS::read(uint8_t *buffer, int size){
  int cnt;
  int c;
  for(cnt = 0; cnt < size; cnt++){
    c=read();
    if(c >= 0){
      buffer[cnt] = (uint8_t) c;
    }else
      break;
  }
  return cnt;
}

// Clears the buffer(s). No processing of any data will be done!
void SerialWS::flush(void){
  flushRecv();
  flushSend();
}
void SerialWS::flushRecv(void){
  if(_rxBufferSize  > 0){
    _rx_buffer_head = 0;
    _rx_buffer_tail = 0;
  }
}
void SerialWS::flushSend(void){
  if(_txBufferSize  > 0){
    _tx_buffer_head = 0;
    _tx_buffer_tail = 0;
  }
}

void SerialWS::cleanupClients(void){
  _ws.cleanupClients();
}

void SerialWS::send(){
 if(!(_tx_buffer_head == _tx_buffer_tail)){
   char tmpBuff[_txBufferSize];                                    // New buffer (because our ring buffer is not necessarily ordered)
   int i = 0;
   while(!(_tx_buffer_head == _tx_buffer_tail)){
     tmpBuff[i] = _tx_buffer[_tx_buffer_tail];
     _tx_buffer_tail = (_tx_buffer_tail + 1) % _txBufferSize;
     i++;
   }
   _ws.textAll(tmpBuff, i);
   flushSend();
  }
}

// checkBuffer will test if the buffer is full. If full, it will send it to all clients and reset th buffer.
void SerialWS::checkBuffer(){
  int w = (_tx_buffer_head + 1) % _txBufferSize;
  if(w == _tx_buffer_tail){                                        //  If buffer is full
    send();
  }
}

// Write a single byte
size_t SerialWS::write(uint8_t b){
  if(_txBufferSize  > 0){                                          // If buffering
    checkBuffer();
    _tx_buffer[_tx_buffer_head] = b;
    _tx_buffer_head = (_tx_buffer_head + 1) % _txBufferSize;
  }else{                                                           // Not buffering. Direct write! (bad idea as bad performance to broadcast single bytes!)
    _ws.textAll(&b, 1);
  }
  return 1;                                                        // One byte written
}

// Write a block of bytes
size_t SerialWS::write(uint8_t *buffer, size_t size){
  if(_txBufferSize  > 0){                                          // If buffering
    for(int i =0; i < size; i++){
      checkBuffer();
      _tx_buffer[_tx_buffer_head] = buffer[i];
      _tx_buffer_head = (_tx_buffer_head + 1) % _txBufferSize;
    }
  }else{                                                           // Not buffering. Direct write!
    _ws.textAll(buffer, size);
  }
  return size;
}

// Handles SerialWS web socket events
void SerialWS::onSerialWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  switch(type){
  case WS_EVT_CONNECT:
    break;
  case WS_EVT_DISCONNECT:
    break;
  case WS_EVT_ERROR:
    break;
  case WS_EVT_PONG:
    break;
  case WS_EVT_DATA:
    // Data has been received by the SerialWS websocket
    if(_rxBufferSize > 0){                                         // Else, if there is a receive buffer
      int index;
      for(int i = 0; i < len; i++){                                // Loop through each byte received
        index = (_rx_buffer_head + 1) % _rxBufferSize;             // Calculate a new ring buffer head
        if(index == _rx_buffer_tail)                               // If new head = tail, buffer is full so the data can't be stored.
          break;                                                   // So leave the loop, abandoning the rest of the data.
        _rx_buffer[_rx_buffer_head] = data[i];                     // Store the data byte in the buffer
        _rx_buffer_head = index;                                   // Remember the new head.
      }
    }
  }
}
