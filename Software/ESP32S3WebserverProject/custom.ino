// Customize the webserver for your project using this portion of the webserver sketch


// ------------------------- Library section -------------------------
// Libraries
// Put any libraries necessary for your customizations here
// Sample: TFT_eSPI
//#include <TFT_eSPI.h>
//#define GFXFF 1

// ------------------------- Global section -------------------------
// Global
// Put any global variables, #defines, structure definitions, etc.
// necessary for your customization here
// Sample: A pushbutton
// #define button 42                // The pushbutton is on pin 42
// bool buttonPress = false;        // Will be changed to true within the ISR



// ------------------------- ISR section -------------------------
// Interrupt service routines
// Put any ISR necessary for your customization here
// Example: 
// void IRAM_ATTR ISR_button(){buttonPress = true;}       // ISR to handle button presses. IRAM_ATTR keeps it in memory. 



// ------------------------- Setup section -------------------------
// Setup
// Put any additional setup requirements for your customization here
void customEarlySetup(){
// customEarlySetup will be called before most of the server resources are
// made available. Use customEarlySetup for things that may need to be done
// more or less immediately upon boot, such as ensuring that a buzzer is not
// on or blanking a display (maybe display a flash screen)
}

void customLateSetup(){
// customLateSetup will be called after all of the basic webserver setup is
// complete which means that things like local time, the SD card, Telegram,
// etc. would already be initialized and available within your customization
  // Sample:
  // pinMode(button, INPUT_PULLUP);
}



// ------------------------- Webpage section -------------------------
void customPages(){
// Custom web pages
// Put any custom pages required for your customization here
// customPages() will be called during webserver setup
// Some pages may require no setup at all as they may be picked up by the standard server.serveStatic routine if they are in the location defined for stdFiles
// (though the full name would be required. Like "/sensors.htm").
// Only add them here if you are getting page not found errors when accessing your pages

  // Sample sensor data page
/*  server.on("/sensor", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SD, "/sensors.htm","text/html");           // Send page specifically from SD card
    // request->send(LittleFS, "/sensors.htm","text/html");   // Send page specifically from LittleFS (Flash)
    // See ESPAsyncWebServer documentation for further possibilities
  });
*/
}



// ------------------------- Status section -------------------------
void customServerStatus(char stat[]){
// Use customServerStatus to add any messages to the server status that
// may be required. Remember to end messages with a newline ("\n").
// Note, the standard status set by the server is usually less than
// 300 characters long. The status may NEVER be more than 999 characters
// long so keep that in mind when creating your own messages!

  // Sample:
  //   char buff[25];
  //   sprintf(buff, "Backup power is %s\n", digitalRead(32) ?  "on", "off");
  //   strcat(stat, buff);
}



// ------------------------- Loop section -------------------------
bool customLoop(bool didOne){
// Put any additional programming to be executed within the loop of the
// webserver here. For example, you may take sensor readings, send Telegram
// alerts, etc.
// customLoop will be called each pass through the loop
// Where possible, do not execute your own code if the parameter didOne is true
// in order to prevent slowing down the loop. (didOne true means a process required by
// the basic web server has been performed so some CPU time within the loop is already used)
// Be sure to set the customDidOne to true if you process any of your own procedures
// that should prevent other routines within the main loop from processing in the
// same iteration of the loop.
// The customLoop will be called AFTER higher priority processing within the webserver loop
// (such as handling OTA, clients, polling for Telegram messages, etc.) but before handling lower priorities
// (such as time synchronization)

  // REALLY high priority custom routines should always be processed
  // Put such processing prior to if(didOne || customDidOne)

  // Sample: Check water sensor
  // if(digitalRead(waterSensor)){
  //   waterAlarm();
  //   customDidOne = true;
  // }

  if(didOne || customDidOne)                            // The rest of this is no priority
    return customDidOne;                                // So get out of here if anything has already been processed
    
  // Sample low priority process
  // if((currentTime.tm_mon == 5)                       // If it is May
  //  &&(currentTime.tm_mday == 31)){                   // 31st
  //   sendBirthdayMsg();
  //   customDidOne = true;
  // }

  return customDidOne;                                // Let WebServer know if we did a process
}



// ------------------------- Websocket section -------------------------
void customWS_EVT(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
// Put responses to custom web socket requests here
// customWS_EVT will be called when a websocket data request is received that is NOT
// handled by the standard webserver processes themselves.
// For further details on the parameters, see the documentation for the ESP Asynchronous Web Server library
// For samples of handling messages, see the documentation
// NOTE: This protocol assumes that the request actually has a parameter named "request" that identifies
// what is being requested.

  switch(type){
  case WS_EVT_DATA:               // The EVT_DATA type is probably the only one you need to handle
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->opcode == WS_TEXT){  // Note: Applications will only see WS_TEXT and WS_BINARY.
      char operand[25];  // Assuming a maximum operand length of 24 plus terminating 0. Increase if necessary. (Or use shorter names!)
      bool JSON = data[0]=='{'; // Rudimentary test to determine if the message is in JSON format
      if(JSON){
        char* reqAddr = strstr((char*)data,"request");                // Does the data contain "request"
        if((reqAddr != NULL) && (reqAddr - (char*)data < 10)){        // within the first ten characters? If so..
          JsonDocument doc;                                           // Allocate the JSON document
          DeserializationError Jerror = deserializeJson(doc, data);   // Test if parsing succeeds and deserialize the JSON document
          if(!Jerror){                                                // Test if parsing succeeded
            const char* request = doc["request"];
            // Sample: a request "setVal"
            // if(strcmp(request,"setVal") == 0){
            //   doSetVal(doc);   // Additional parameters are in doc.
            // }
          }
        }
      } else {
        getOperand(operand, (char*)data, sizeof(operand));
      }
    }
    break;
  }
}

#if ((UseTelegram == TelegramSend) || (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive))
// ------------------------- Telegram section -------------------------
bool customTelegramRcvd(int botNum, UniversalTelegramBot bot, struct telegramMessage tMsg){
// Put handling of received Telegram Messages here. Be sure to set UseTelegram in customize.h to enable receiving Telegram messages!
// botNum is the index of the receiving bot in Bots
// bot is the bot itself

/*struct telegramMessage{
  String text;                                  // The actual UTF-8 text of the message. Commands begin with /. For example, /lightOff
  String chat_id;                               // The chat_id.
  String chat_title;                            // Title, for supergroups, channels and group chats
  String from_id;                               // The chat_id of the person sending the message
  String from_name;                             // The first name of the person sending the message
  String date;                                  // Date the message was sent (in Unix time)
  String type;                                  // "channel_post", "callback_query", "edited_message", "message"
  String file_caption;                          // If hasDocument, the message caption
  String file_path;                             // If hasDocument, the file path of the document
  String file_name;                             // If hasDocument, the file name of the document
  bool hasDocument;                             // Does this message contain a document?
  long file_size;                               // If hasDocument, the size of the document
  float longitude;                              // Longitude as defined by sender
  float latitude;                               // Latitude as defined by sender
  int update_id;                                // Internal use. Message id of last received message.
  int message_id;                               // Unique message identifier inside this chat
  int reply_to_message_id;                      // If this message is a reply to a message, the id of the message being replied to
  String reply_to_text;                         // If this message is a reply to a message, the text of the message being replied to
  String query_id;                              // Only given when type = "callback_query". Then it is the message_id
};*/

  // See documentation for examples.  
  bool msgHandled = false;
  switch(botNum){                              // Depending on which bot received the message
    case 0:                                    // If it was bot 0
      // Sample: 
      // if(tMsg.text == "/power"){
      //   sendPowerMsg(tMsg);                 // Additional info (such as WHO sent the message) is in tMsg.
      //   msgHandled = true;
      // }
      break;
  }
  return msgHandled;
}
#endif
