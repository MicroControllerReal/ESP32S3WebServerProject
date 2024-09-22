// Appendix B – Sample Customization For A Water Detection Alarm
// Appendix C –Telegram Messaging
// Appendix D – Sample Customization To Add Graphics
// Customize the webserver for your project using this portion of the webserver sketch

// In customize.h, you will need to have set UseTelegram to TelegramSendReceive so that we can both send and receive Telegram messages. You will have to have entered your bot token (from Telegrams bot father when you created the bot) and your chat id (that identifies YOU as the Telegram user). You will also have to define how often Telegram should be checked for new messages. (Don’t set this too low! It does cause a certain load on the server and your local WiFi and will use badwidth from your Internet Provider!)  Telegram messages will be ignored if they are not from  one of the two chat ids defined in customize.h! There are a lot of people out there trying to break into your bot and this provides a certain level of security.
// Also setup the Telegram Bots, CHAT_ID and TGRECEIVEFREQ
//  Bot Bots[] = {
//    {"9999999999:AAA9aABc_dZZaaZ9aZzA9z9A9abC9z-Z9aa",true,0}
//  };
//#define CHAT_ID     "888888888"       // Primary Telegram user.
//#define TGRECEIVEFREQ 60              // How many seconds between polling


// ------------------------- Library section -------------------------
// Libraries
// Put any libraries necessary for your customizations here
#include <TFT_eSPI.h>
#define GFXFF 1

// The TFT_eSPI library needs settings to be made in its user_setup.h. For example:
//#define ILI9488_DRIVER
//#define TFT_MOSI 11    
//#define TFT_MISO 12
//#define TFT_SCLK 13
//#define TFT_CS   10    // Chip select control pin
//#define TFT_DC   16    // Data Command control pin
//#define TFT_RST   7    // Reset pin 
//#define TFT_BL   17    // LED back-light
//#define TOUCH_CS  6    // Chip select pin (T_CS) of touch screen

// ------------------------- Global section -------------------------
// Global
// Put any global variables, #defines, structure definitions, etc.
// necessary for your customization here
#define waterDetect 39  // NOTE: Attach an external 10K resistor to pull this pin to 3.3V!
#define waterAlarm  40
#define waterPump   41
bool volatile waterDetected  = false; // Variables used in ISR must be volatile!
bool alarmOn = false;      //Var to track alarm. Global because in several routines
bool pumpOn  = false;      // Var to show if pump on/off.
TFT_eSPI tft = TFT_eSPI();    // Invoke TFT_eSPI library with default width and height



// ------------------------- ISR section -------------------------
// Interrupt service routines
// Put any ISR necessary for your customization here
void IRAM_ATTR ISR_water(){waterDetected = true;}


// ------------------------- Setup section -------------------------
// Setup
// Put any additional setup requirements for your customization here
void customEarlySetup(){
// customEarlySetup will be called before most of the server resources are
// made available. Use customEarlySetup for things that may need to be done
// more or less immediately upon boot, such as ensuring that a buzzer is not
// on or blanking a display (maybe display a flash screen)
  pinMode(waterAlarm, OUTPUT);
  digitalWrite(waterAlarm, LOW);
  pinMode(waterPump, OUTPUT);
  digitalWrite(waterPump, LOW);
  initGraphics();
}


void customLateSetup(){
// customLateSetup will be called after all of the basic webserver setup is
// complete which means that things like local time, the SD card, Telegram,
// etc. would already be initialized and available within your customization
  // NOTE: Attach an external 10K resistor to pull this pin to 3.3V!
  pinMode(waterDetect, INPUT);
  // ISR_water will be called when waterDetect starts to go low
  attachInterrupt(waterDetect,ISR_water, FALLING);
}



// ------------------------- Webpage section -------------------------
void customPages(){
// Custom web pages
// Put any custom pages required for your customization here
// customPages() will be called during webserver setup
// Some pages may require no setup at all as they may be picked up by the standard server.serveStatic routine if they are in the location defined for stdFiles
// (though the full name would be required. Like "/sensors.htm").
// Only add them here if you are getting page not found errors when accessing your pages
  // Register the webpage "/water"
  server.on("/water", HTTP_GET, [](AsyncWebServerRequest *request){
    char alarmTxt[100];
    if(alarmOn || waterDetected){
      // Text will be an "X" symbol + "Water Detected!"
      strcpy(alarmTxt, "&#x274C; Water Detected!");}
    else {
      // Text will be a checkmark + "No Water Detected"
      strcpy(alarmTxt, "&#x2713; No Water Detected");}
    // Add in pump status
    strcat(alarmTxt, "<br>Pump is: ");
    if(pumpOn)
      strcat(alarmTxt, "On");
    else
      strcat(alarmTxt, "Off");
    AsyncWebServerResponse *response =
      request->beginResponse(200, "text/html", alarmTxt);
    request->send(response);
  });
}



// ------------------------- Status section -------------------------
void customServerStatus(char stat[]){
// Use customServerStatus to add any messages to the server status that
// may be required. Remember to end messages with a newline ("\n").
// Note, the standard status set by the server is usually less than
// 300 characters long. The status may NEVER be more than 999 characters
// long so keep that in mind when creating your own messages!

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

  static unsigned long lastDisplay = -15000;  // last display update
  bool customDidOne = false;      // Set to true if any custom processing is done.

  // REALLY high priority custom routines should always be processed
  // Put such processing prior to if(didOne || customDidOne)

  if(alarmOn                         // Alarm is on
  && (!digitalRead(waterDetect))){   // but water is no longer there
    digitalWrite(waterAlarm, LOW);   // Turn piezo buzzer OFF
    waterDetected = false;           // Reset waterDetected so we see it next time
    alarmOn = false;                 // Remember that the alarm is OFF
    sendWaterMsg();                  // Notify user that problem was resolved
    customDidOne = true;             // We did some processing
  } else if((!alarmOn)               // Alarm is not on
  && waterDetect){                   // but water was detected by the interrupt
    digitalWrite(waterAlarm, HIGH);  // Turn piezo buzzer ON
    alarmOn = true;                  // Remember that the alarm is ON
    sendWaterMsg();                  // Notify via Telegram that there is a problem
    customDidOne = true;             // We did some processing
  }
    
  if(didOne || customDidOne)         // The rest of this is no priority
    return customDidOne;             // So get out of here
    
  if((millis() - lastDisplay) > 15000){  // Update display every 15 seconds
    displayStatus();        // Update display
    lastDisplay = millis();
    customDidOne = true;    // Remember that we did processing
  }
  return customDidOne;      // Let WebServer know if we did a process
}




// ------------------------- Websocket section -------------------------
void customWS_EVT(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
// Put responses to custom web socket requests here
// customWS_EVT will be called when a websocket data request is received that is NOT
// handled by the standard webserver processes themselves.
// For further details on the parameters, see the documentation for the ESP Asynchronous Web Server library
// For samples of handling messages, see the documentation
}

#if ((UseTelegram == TelegramSend) || (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive))
// ------------------------- Telegram section -------------------------
bool customTelegramRcvd(int botNum, UniversalTelegramBot bot, struct telegramMessage tMsg){
// Put handling of received Telegram Messages here. Be sure to set UseTelegram in customize.h to enable receiving Telegram messages!
// botNum is the index of the receiving bot in Bots
// bot is the bot itself
  bool msgHandled = false;
  switch(botNum){                         // Depending on which bot received the message
    case 0:                               // If it was bot 0
      if(tMsg.text == "/water"){          // User wants a water status
        sendWaterMsg();                   // Send the water status message
      }
      if(tMsg.text == "/pumpOn"){         // User wants to turn pump on
        digitalWrite(waterPump, HIGH);    // Turn pump ON
        pumpOn = true;                    // Remember that pump is on
      }
      if(tMsg.text == "/pumpOff"){        // User wants to turn pump off
        digitalWrite(waterPump, LOW);     // Turn pump OFF
        pumpOn = false;                   // Remember that pump is off
      }
      break;
  }
  return msgHandled;
}

#endif

void sendWaterMsg(){
  char msgTxt[100];              // Variable for text of message
  int numBtns = 2;               // Msg will have two buttons
  TelegramBtn btns[numBtns] = {  // Define the two buttons
    {"Pump On", "/PumpOn"},      // Says "Pump On", sends "/PumpOn"
    {"Pump Off", "/PumpOff"}     // when pressed. 2nd button is similar
  };
  // Build a text message based on water detection
  if(true || alarmOn || waterDetected){
    strcpy(msgTxt, "Water Detected!");}
  else {
    strcpy(msgTxt, "No Water Detected");}
  // Add pump status to message
  strcat(msgTxt, "\nPump is: ");
  if(!pumpOn)
    strcat(msgTxt, "On");
  else
    strcat(msgTxt, "Off");
  // Send Telegram Msg with keyboard (the 2 buttons)
  sendTelegramMsgKbd(0, tgChatID, msgTxt, btns, numBtns);
}

void initGraphics(){
  tft.begin();                        // Initialize TFT
  // By default the TFT_eSPI Library will turn the Backlight on.
  // Change the pin to analog and dim the screen
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 128);
  tft.setRotation(3); // 0&2 are portrait, 1&3 are landscape. If 0/1 is upside down use 2/3
}

void displayStatus(){
  char buff[100];
  int h = 29;                               // Height of a single line of text
  tft.fillScreen(TFT_BLACK);                // Clear the screen to black
  tft.setTextDatum(BL_DATUM);               // Bottom/Left oriented text
  tft.setFreeFont(&FreeSansBold12pt7b);     // Set 12 point, bold, Sans font
  tft.setTextColor(TFT_WHITE, TFT_BLACK);   // Text is white on black

  tft.drawString("Water:", 0, h, GFXFF);
  if(alarmOn || waterDetected)
    tft.drawString("Detected!", 100, h, GFXFF);
  else
    tft.drawString("Not detected", 100, h, GFXFF);
  tft.drawString("Pump:", 0, h*2, GFXFF);
  if(pumpOn)
    tft.drawString("On", 100, h*2, GFXFF);
  else
    tft.drawString("Off", 100, h*2, GFXFF);
}
