/* The ESP32 S3 Webserver Project Copyright (c) 2024 MicroControllerReal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// The ESP32 S3 Webserver Project software
// An easily customizable webserver template designed for easy but flexible and performant use.
// Requires an ESP32, S3 preferred
// A WiFi connection is recommended. Can work without but with limited features. (No TimeSynch with internet NTP server, No Telegram messages)
// Optionally supports either a SD (up to 32GB) or MMC card (up to 2TB and faster than SD, thus preferred)
// Optionally supports LittleFS for files in flash memory
// Does not support SPIFFS as it has been deprecated and replaced with LittleFS
// Optionally provides a webpage file manager to list, delete, upload and download to/from SD/MMC and/or LittleFS, including directories.
// Uploading files with the file manager supports a client side progress bar
// Optionally provides Serial output over a webpage (SerialWS). Useful for seeing what errors have occurred without needing a USB cable.
// Does not use an external RTC chip but does synch to an internet time server periodically, ensuring accuracy
// Supports timezones, to include standard and daylight saving time where appropriate
// Uses ESP32 "Preferences" for parameter storage
// Optionally supports OTA (Over The Air, WiFi based loading of sketches to flash memory)
// Optionally supports sending and/or receiving Telegram messages to/from one or two pre-defined recipients.
// NOTE: The ESP32 S3 Webserver Project hardware PVB board can be used. That board also supports a 480 x 320 TFT and optionally a Touchpad.
//       Graphics and touch are not implemented in this webserver template, but can easily be added as a customization.
//       See the example (Appendix D) in the documentation for implementing graphics as a project customization.

#include "WebServerDefines.h"         // Webserver specific definitions
#include "customize.h"                // customize.h contains settings to customize the webserver

// Perform sanity checks on SDType and StandardFiles at compile time
#if ! ((SDType == TypeSD) || (SDType == TypeMMC) || (SDType == TypeNoSD))
  #error Invalid SD Type
#endif
#if ((StandardFiles == onLFS) || (StandardFiles == onSD))
  #if(StandardFiles == onSD)
    #if!((SDType == TypeSD) || (SDType == TypeMMC))
      #error StandardFiles on SD but no SD defined
    #endif
  #else
    #if !(UseLittleFS)
      #error StandardFiles on LittleFS but LittleFS not defined
    #endif
  #endif
#else
  #error Invalid StandardFiles setting
#endif

#if defined(CONFIG_IDF_TARGET_ESP32C3) ||defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) 
  #include "driver/temp_sensor.h"     // Provides temperature sensing for the ESP32S3 chip
#endif
#include <Preferences.h>              // Provides for configuration parameters stored as ESP32 preferences
#include <Time.h>                     // Provides time conversion and NTP synchronization
#include <WiFi.h>                     // Provides WiFi connectivity
#include <ESPAsyncWebServer.h>        // Provides asynchronous web server support for up to 8 clients
#include <ESPmDNS.h>                  // Allows this server to be resolved in local network
#include <SPI.h>                      // Necessary for SPI communications (SD, MMC, Graphics, ...)
#include <ArduinoJson.h>              // Necessary for Telegram and JSON styled WebSocket requests and responses
#if (SDType == TypeSD)
  #include <SD.h>                     // Provides for files on an SD card. 32GB Max!
#endif
#if (SDType == TypeMMC)
  #include <SD_MMC.h>                 // Provides for files on an MMC card. 2TB max and faster than SD!
  #define SD SD_MMC                   // Allows the exact same programming for SD_MMC as SD (without having to use SD_MMC.open, etc.)
#endif
#if UseLittleFS
  #include <LittleFS.h>               // Provides a file system in flash memory
#endif
#if UseOTA
  #include <ArduinoOTA.h>             // Provides for updating sketch over the air (using WiFi instead of USB)
#endif
#if (UseTelegram == TelegramSend) || (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive)
  #include <WiFiClientSecure.h>
  #include <UniversalTelegramBot.h>
#endif
#if useSerialWS
  #include <SerialWS.h>               // Provides a web based Serial Monitor. Requires SerialWS.htm
#endif

const char Firmware[] = "1.3";        // Version of The ESP32S3 Webserver Project software (this program)
// Version history:
// 1.0 Initial version with all features. Developed over time of about Jan-Jun 2024
// 1.1 Bug corrections. Redesign of Telegram messaging to enable multiple bots (mainly to allow for alarms
//     from a second bot with distinct ringtones client side)
// 1.2 Added a blocked area of mass storage that is invisible and protected in file manager. Added safetyNet
//     features (reboot every n days, reboot on low memory, reboot on low heap size)
// 1.3 Made MMC card SPI speed configurable, with default of BOARD_MAX_SDMMC_FREQ (maximum speed defined by board)
//     Added customization for MMC 1 bit mode. Slower, but could provide additional GPIO pins if necessary
//     Removed telegram messaging and time synchronization if running without WiFi (AP mode only).
//     Cleaned up use of setupVerbose when not using Serial (it was wasting program space!)
//     Added Server Running Time to status.
//     Added comments in customize.h about swapping SD CS and TFT CS beginning with version 3.0 of the hardware.
//     Added embedFileman option (helps initially load files to mass storage)
// Future plans
//     Serve resources from a data structure instead of programming each separately. Reduces program size.
//     Improve ability to serve gzip compressed files. This reduces file sizes and thus network traffic and can increase performance.
//     Add serving files directly from PSRAM to make the server even more performant. (? Tests needed!)
//     Rewrite as a super library to reduce programming clutter and maintenance.

// Keep the parameters as global variables. They are used in the websocket events, etc
// and therefore need to be quickly available. Loading multiple parameters from Preferences
// may take too long and cause performance problems or even kick the watchdog...
char ssid[32];                        // Name of WiFi SSID to connect to
char password[64];                    // Password to connect to that WiFi SSID
// hostname will be used by mDNS so that the server can be reached at that name from the local network (i.e. http://myProject)
// hostname will be used as the name for uploading new firmware over the air (OTA)
// hostname + "AP" will be used as the name the access point is established for
char hostname[32] = "";               // Name of this server
char appasswd[64] = "";               // Password users must supply to access this access point
bool beacon = NULL;                   // Will the AP SSID beacon be on or off (AP hidden)?
char adminUsername[32] = "";          // Administrative logon for configuration and file manager
char adminPassword[32] = "";          // Administrative password for configuration and file manager
char timezone[64] = "";               // Local timezone definition, including DST. Limited in admin.htm to 50 characters on input
char ntpserver[64] = "";              // Name of NTP server to synchronize time to. Limited in admin.htm to 35 characters on input

#if (UseTelegram == TelegramSend) || (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive)
  char tgChatID[12] = CHAT_ID;
  char tgAltChatID[12] = ALT_CHAT_ID;
  int numBots = sizeof(Bots) / sizeof(Bots[0]);
#endif

AsyncWebServer server(80);                 // Create Asynchronous Web Server object on port 80
AsyncWebSocket ws("/ws");                  // Create Asynchronous Web Socket handler
AsyncEventSource events("/events");
uint32_t lastNTPSynch;                     // millis() when last NTP synch occurred
tm currentTime;                            // Current time.
int64_t lastReboot=0;                      // micros() when the server was started

#if UseOTA 
  bool activeOTA = false;                  // Will be true during OTA updates
#endif

#if useSerialWS
  SerialWS serialWS;                       // Implements the web based Serial Monitor
#endif

void setup(){

#if serialEnabled
  Serial.begin(serialBaud);                // Start serial output at designated baud (speed)
  Serial.setDebugOutput(true);
#if setupVerbose
  serialPrint("\n\nInitializing\n",true);  // Blank lines first because ESP32 may output some info at boot
#endif
#endif // serialEnabled

  customEarlySetup();                      // Execute custom early setup. Required for customizations, if any.
  
  initParams();                            // Read all parameters from preferences. Do first init if necessary.

#if UseLittleFS
  initLittleFS();                          // Initialize LittleFS (files in flash)
#endif

#if ((SDType == TypeSD)  || (SDType == TypeMMC))
  initSDCard();                            // Start the SD or SD_MMC card and library
#endif

  startWiFi();                             // Start access point if configured, connect to WiFi if configured!

#if UseOTA
  initOTA();                               // Initiate OTA-Over The Air firmware update service
#endif

  initMDNS();                              // Initiate MDNS so the webserver is available in the local network under its name

  if(WiFi.isConnected()){
#if serialEnabled && setupVerbose
    {  char buff[200];
       snprintf(buff,200,"Synchronizing time to: %s", ntpserver);
       serialPrint(buff,true);
    }
#endif
    int count=0;
    while(!synchDateTime()){               // Synchronize date & time to NTP server. Configure timezone and DST){
#if serialEnabled && setupVerbose
      serialPrint(".",false);
#endif
      if(++count>9){
#if serialEnabled && setupVerbose
        serialPrint("\nSynchronization failed!\n",true);
#endif
        break;
      }
    }
#if serialEnabled && setupVerbose
    serialPrint("\n",false);
#endif
  }
#if serialEnabled && setupVerbose
  serialPrint("Initiating Server\n",true);
#endif
  ws.onEvent(onWsEvent);                   // Server will call function onWsEvent when websocket events occur
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client){});
  server.addHandler(&events);              // Configure server to handle connection events

#if UseStatus
  //The /status should never be cached as it is a "moment in time" response
  //status is designed primarily for debugging purposes
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    char stat[1000];
    serverStatus(stat);
    textToHTML(stat, sizeof(stat));
    request->send(200, "text/html", stat);
  });
#endif

  // Favicon will be cached on the client. It is setup here because the browser will always request
  // "favicon.ico" but the file may be called something else or stored who knows where.
  // If not specifically declared, the server routines here would look for favicon.ico,
  // not find it, and tell the browser that none exists.
  server.on("/favicon.ico",HTTP_GET, [](AsyncWebServerRequest *request){
    responseWithCaching(request, FavIcon);
  });

  server.on("/admin", HTTP_GET, [](AsyncWebServerRequest *request){
    if(adminAllowed(request)) responseWithCaching(request, AdminHTM);
  });

#if useSerialWS
  // Start the SerialWS listener on this server
  serialWS.begin(&server, SerialWStxSize, SerialWSrxSize);
  // Register the SerialWS webpage
  server.on("/SerialWS", HTTP_GET, [](AsyncWebServerRequest *request){
    if(adminAllowed(request)) responseWithCaching(request, SerialHTM);
  });
#endif

#if (UseFileManager)
  setupFileManager();                      // Set up everything for the web based file manager
#endif
  
#if defined(CONFIG_IDF_TARGET_ESP32C3) ||defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) 
  // Configure and start the MCU internal temperature sensor
  // Name          range °C  error
  // TSENS_DAC_L0  50 ~ 125  < 3 °C
  // TSENS_DAC_L1  20 ~ 100  < 2 °C
  // TSENS_DAC_L2 -10 ~ 80   < 1 °C  (Default)
  // TSENS_DAC_L3 -30 ~ 50   < 2 °C
  // TSENS_DAC_L4 -40 ~ 20   < 3 °C
  temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
  temp_sensor.dac_offset = TSENS_DAC_L2;
  temp_sensor_set_config(temp_sensor);
  temp_sensor_start();
#endif
  
  customPages();

// If both LittleFS and SD/MMC are being used, set up prefixing so that the webserver knows where files are located
// NOTE: Files other than the standard files (favicon.ico, admin.htm, DefaultHTM) referred to within webpages MUST
// adhere to the prefixing! Use /f/ for files on LittleFS and /s/ for files on SD/MMC!
#if (UseLittleFS) && ((SDType == TypeSD) || (SDType == TypeMMC))
    // Files on the SD will be prefixed with /s/, but the "/s/" will be reduced to "/" for the path on the SD
    // and files will be cached on the client
    server.serveStatic("/s/",SD,"/").setCacheControl(cacheRule);
    // Files in littleFS will be prefixed with /f/, but the "/f/" will be reduced to "/" for the path
    // in littleFS and files will be cached on the client
    server.serveStatic("/f/",LittleFS,"/").setCacheControl(cacheRule);
#endif
  // on "/" MUST be last in the list otherwise it will be used before other options are even considered!
  #if (StandardFiles == onSD)
    server.serveStatic("/", SD, "/").setDefaultFile(DefaultHTM).setCacheControl(cacheRule);
  #endif
  #if (StandardFiles == onLFS)
    server.serveStatic("/", LittleFS, "/").setDefaultFile(DefaultHTM).setCacheControl(cacheRule);
  #endif

  server.onNotFound([](AsyncWebServerRequest *request){
    serialPrint("NOT_FOUND: ",false);
    if(request->method() == HTTP_GET)
      serialPrint("GET",false);
    else if(request->method() == HTTP_POST)
      serialPrint("POST",false);
    else if(request->method() == HTTP_DELETE)
      serialPrint("DELETE",false);
    else if(request->method() == HTTP_PUT)
      serialPrint("PUT",false);
    else if(request->method() == HTTP_PATCH)
      serialPrint("PATCH",false);
    else if(request->method() == HTTP_HEAD)
      serialPrint("HEAD",false);
    else if(request->method() == HTTP_OPTIONS)
      serialPrint("OPTIONS",false);
    else
      serialPrint("UNKNOWN",false);
      
    {  char buff[1000];
       snprintf(buff,1000," http://%s%s\n", request->host().c_str(), request->url().c_str());
       serialPrint(buff,false);
    }

    if(request->contentLength()){
      {  char buff[1000];
         snprintf(buff,1000,"_CONTENT_TYPE: %s\n_CONTENT_LENGTH: %u\n", request->contentType().c_str(), request->contentLength());
         serialPrint(buff,false);
      }
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      {  char buff[1000];
         snprintf(buff, 1000, "_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
         serialPrint(buff,false);
      }
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        {  char buff[1000];
           snprintf(buff, 1000,"_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
           serialPrint(buff,false);
        }
      } else if(p->isPost()){
        {  char buff[1000];
           snprintf(buff, 1000, "_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
           serialPrint(buff,false);
        }
      } else {
        {  char buff[1000];
           snprintf(buff, 1000, "_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
           serialPrint(buff,false);
        }
      }
    }
    serialPrint(" ",true);
    request->send(404);
  });

  validateFiles();
  customLateSetup();                                    // Execute late custom setup, required for customizations, if any

#if serialEnabled && setupVerbose
  serialPrint("Starting server\n",true);
#endif
  server.begin();
  lastReboot=esp_timer_get_time();
  char stat[1000];
#if serialEnabled && setupVerbose
  serialPrint("Server is online\n",false);
  serverStatus(stat);
  serialPrint(stat,false);
  serialPrint("\n",true);
#endif
#if (((UseTelegram == TelegramSend) || (UseTelegram == TelegramSendReceive)) && TelegramOnBoot)
  sprintf(stat, "%s started", hostname);
  serverStatus(stat);
  sendTelegramMsg(0, tgChatID, stat);
  if(TelegramAltOnBoot)sendTelegramMsg(0, tgAltChatID, stat);
#endif
}

bool adminAllowed(AsyncWebServerRequest *request){
#if onlyLocalAdmin
  // Only requests from this access point or same WiFi network are allowed
  if(!isLocal(request->client()->remoteIP())){
    request->send(403);
    return false;
  }
#endif
  // Only requests from users who have logged in with admin name and password are allowed
  if(!request->authenticate(adminUsername, adminPassword)){
    request->requestAuthentication();
    return false;
  }
  return true;
}

void responseWithCaching(AsyncWebServerRequest *request, char fileName[]){
#if (StandardFiles == onLFS)
  responseWithCaching(request, LittleFS, fileName);
#endif 
#if (StandardFiles == onSD)
  responseWithCaching(request, SD, fileName);
#endif
}

void responseWithCaching(AsyncWebServerRequest *request, FS fs, char fileName[]){
  AsyncWebServerResponse *response = request->beginResponse(fs, fileName);
  response->addHeader("Cache-Control", cacheRule);
  request->send(response);
}

void loop(){
#if useSafetyNet
  static unsigned long lastSafety = millis();           // Last millis() when safetyNet features were checked
#endif
  static unsigned long lastClientMaint    = -1000;      // Last millis() when old clients were maintained (inactive deleted)
#if useSerialWS
  static unsigned long lastSWSclientMaint = -500;       // Last millis() when old clients were maintained (inactive deleted) for SerialWS
#endif
  static unsigned long lastTimeSynchCheck = 0;          // Last millis() when a check for a time synch was done (Check! not actually synchronized!)
  static unsigned long lastCurrentTime    = -1000;      // Last time currentTime was updated

#if (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive)
  static unsigned long lastTelegramCheck=-1000*TGRECEIVEFREQ;// Last time Telegram was checked for incoming messages
#endif
  const unsigned long timeSynch = 1000 * 60 * 60 * 6;   // Resynch time to NTP every six hours. (Failed attempts won't be retested for 5 minutes)

  bool didOne = false;                                  // Will be set true if any process actually runs. Prevent more than 1 running per pass through loop!
  bool WiFiConnected = WiFi.isConnected();              // Some processes may be skipped if no WiFi is connected

#if UseOTA
  ArduinoOTA.handle();                                  // Always handle OTA if it is enabled
  didOne = activeOTA;                                   // If OTA is Updating, do nothing else
#endif

  if((!didOne)&&((millis()-lastClientMaint)>=1000)){    // Once a second
    ws.cleanupClients();                                // Delete old client sessions
    lastClientMaint = millis();                         // Remember the last time client maintenance was done
    didOne = true;
  }

#if useSerialWS
  if((!didOne)&&((millis()-lastSWSclientMaint)>=1000)){ // Once a second
    serialWS.cleanupClients();                          // Delete old client sessions
    lastSWSclientMaint = millis();                      // Remember the last time client maintenance was done
    didOne = true;
  }
#endif

  if((!didOne) && (millis() - lastCurrentTime) > 500){  // Every half second
    getLocalTime(&currentTime,NULL);                    // Get current local time. It will be adjusted for timezone and DST!
    lastCurrentTime = millis();                         // Remember last time currentTime was updated
  }

#if (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive)
  if((!didOne) && WiFiConnected && 
    ((millis() - lastTelegramCheck)/1000) >= TGRECEIVEFREQ){
      // BTW & FWIW, in testing, it took 0.7 seconds to check for messages.
      if(!getTelegramMsgs() || TGRECEIVEFREQ <= 10){    // If NO messages were received then we are done
        lastTelegramCheck = millis();
      } else {
        // If messages were received, we will check again soon to handle any additional messages
          lastTelegramCheck += 10000;                   // Force a check again in ten seconds
      }
      didOne = true;
  }
#endif

  // Call customLoop every cycle, regardless of didOne or not. This allows for high priority processes.
  // But, tell the customLoop if didOne or not so lower priority processes will be skipped over
#if UseOTA                                              // Never call customerLoop during an OTA update!
  if(!activeOTA) didOne |= customLoop(didOne);          // Execute processes, if any, required for customizations
#else
  didOne |= customLoop(didOne);                         // Execute processes, if any, required for customizations
#endif

  if(WiFiConnected && (!didOne) && 
      ((millis() - lastTimeSynchCheck) >=(1000*60*5))){ // Once every 5 minutes
    if((millis() - lastNTPSynch) >= timeSynch){         // Check to see if NTP Synch is due
      synchDateTime();                                  // Synchronize time to NTP (if possible). Sets lastNTPSynch if successful!
    }
    lastTimeSynchCheck = millis();                      // We did check. Don't waste resources, even (especially!) if NTP failed!
    didOne = true;
  }

#if useSafetyNet
  if((!didOne) && millis() - lastSafety > 15000){       // Check safety net criteria every fifteen seconds
    if(
    #if safetyMinFreeHeapKB > 0
      ((ESP.getFreeHeap()/1024) < safetyMinFreeHeapKB) ||
    #endif
    #if safetyMinAllocHeapKB > 0
      ((ESP.getMaxAllocHeap()/1024) < safetyMinAllocHeapKB) ||
    #endif
    #if safetyMaxRebootDays > 0
      // From 3:00:00 - 3:59:59 daily (86400000000 = 1000 * 1000mS * 60sec * 60min * 24hrs) 
      ((currentTime.tm_hour == 3) && (((esp_timer_get_time() - lastReboot) / 86400000000) >= safetyMaxRebootDays)) ||
    #endif
    false){                                             // false, just to close the optional #ifs from above...
      serialPrint("Rebooting due to safety net issues\n",true);
      delay(1000);
      ESP.restart();
    }
  }
#endif
}

void initSDCard(){
#if (SDType == TypeSD)
  // Use normal SD card. Up to 32GB supported.
  // By default, the ESP32 SD library defaults to 4MHz
  // The "Internet" says that 40MHz also works.
  //SD::begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint="/sd", uint8_t max_files=5, bool format_if_empty=false);
  // Tested! 40MHz works! Is faster, but not 10x faster!
  while(!SD.begin(SDcs, SPI, SDMHz * 1000000, "/sd", SDmaxOpenFiles, SDformatIfFail)){
    #if serialEnabled && setupVerbose
      serialPrint("SD card mount failed\n",true);
    #endif
    delay(3000);
  }
  #if serialEnabled && setupVerbose
    serialPrint("SD card mounted\n",true);
  #endif
#endif
#if (SDType == TypeMMC)
  //We have mimicked SD_MMC as SD so that the code for file access is identical for SD and SD_MMC!
  #if !(MMCChipDetect == -1)
    pinMode(MMCChipDetect, INPUT_PULLUP);
    while(digitalRead(MMCChipDetect)){
      serialPrint("Insert MMC card\n",true);
      delay(3000);
    }
  #endif
  //bool begin(const char * mountpoint="/sdcard", bool mode1bit=false, bool format_if_mount_failed=false, int sdmmc_frequency=BOARD_MAX_SDMMC_FREQ, uint8_t maxOpenFiles = 5);
  //mode1bit=true is slower, but uses fewer pins
  //Defaults are all best, assuming all data lines are available
  SD_MMC.setPins(MMCclk, MMCcmd, MMCd0, MMCd1, MMCd2, MMCd3);
  while(!SD_MMC.begin("/sdcard", MMCmode1bit, MMCformatIfFail, MMCMHz, MMCmaxOpenFiles)){
    serialPrint("MMC card mount failed\n",true);
    delay(3000);
  }
  #if serialEnabled && setupVerbose
    serialPrint("MMC card mounted\n",true);
  #endif
#endif
}

#if UseLittleFS
void initLittleFS(){
  while(!LittleFS.begin(fmtLittleFS)){
    serialPrint("LittleFS Mount Failed\n",true);
    delay(3000);
  }
#if serialEnabled && setupVerbose
  serialPrint("LittleFS mounted\n",true);
#endif
}
#endif

void serverStatus(char *stat){
  char tmp[100];                                // Temporary variable for local formatting
  stat[0]=0;                                    // Start with a "clean" output buffer
  sprintf(tmp, "%s\n%d cores at %dMHz\n", ESP.getChipModel(),ESP.getChipCores(),ESP.getCpuFreqMHz());
  strcat(stat,tmp);
  sprintf(tmp,"%uKB ",ESP.getFlashChipSize()/1024);
  strcat(stat,tmp);
  switch(ESP.getFlashChipMode()){
  case FM_QIO:
    strcat(stat,"QIO");
    break;
  case FM_QOUT:
    strcat(stat,"QOUT");
    break;
  case FM_DIO:
    strcat(stat,"DIO");
    break;
  case FM_DOUT:
    strcat(stat,"DOUT");
    break;
  case FM_FAST_READ:
    strcat(stat,"Fast Read");
    break;
  case FM_SLOW_READ:
    strcat(stat,"Slow Read");
    break;
  case FM_UNKNOWN:
    break;
  }
  uint32_t x = ESP.getFlashChipSpeed();
  if(x>0){
    sprintf(tmp," %uMhz",x/(1000*1000));
    strcat(stat,tmp);
  }
  strcat(stat," Flash\n");

#if UseLittleFS
  unsigned long total = ((long)LittleFS.totalBytes()/1024L);
  unsigned long used  = ((long)LittleFS.usedBytes()/1024L);
  unsigned long remain = ((long)total - (long)used);
  sprintf(tmp,"%uKB of %uKB free on LittleFS\n",remain,total);
  strcat(stat,tmp);
#endif

  x = ESP.getPsramSize();
  if(x > 0){
    sprintf(tmp,"%uKB of %uKB PSRAM free\n",ESP.getFreePsram()/1024,x/1024);
    strcat(stat,tmp);
  } else {
    strcat(stat,"No PSRAM\n");
  }

#if defined(CONFIG_IDF_TARGET_ESP32C3) ||defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) 
  float MCUtempC = 0;
  temp_sensor_read_celsius(&MCUtempC);
  char pct[6];
  sprintf(tmp,"MCU Temp: %.1f°C\n", MCUtempC);
  strcat(stat,tmp);
#endif
  
  sprintf(tmp,"%uKB of %uKB heap free\n",ESP.getFreeHeap()/1024,ESP.getHeapSize()/1024);
  strcat(stat,tmp);
  sprintf(tmp,"%uKB maximum heap block\n",ESP.getMaxAllocHeap()/1024);
  strcat(stat,tmp);

#if (SDType == TypeSD) || (SDType == TypeMMC)
  uint8_t cardType = SD.cardType();
  strcat(stat,"SD card type: ");
  switch(cardType){
  case CARD_NONE:
    strcat(stat,"None");
    break;
  case CARD_MMC:
    strcat(stat,"MMC");
    break;
  case CARD_SD:
    strcat(stat,"SDSC");
    break;
  case CARD_SDHC:
    strcat(stat,"SDHC");
    break;
  default:
    strcat(stat,"UNKNOWN");
    break;
  }
  if(!(cardType==CARD_NONE)){
    unsigned long total = (SD.totalBytes()/1024L/1024L/1024L);
    unsigned long used  = (SD.usedBytes()/1024L/1024L/1024L);
    unsigned long remain = total - used;
    sprintf(tmp," %uGB of %uGB free\n",remain,total);
    strcat(stat,tmp);
  }
#endif

  sprintf(tmp,"Webserver Version: %s\n",Firmware);
  strcat(stat,tmp);
  sprintf(tmp, "Wifi Strength: %idBm %i%%\n",WiFi.RSSI(),(int)map(WiFi.RSSI(),-30,-90,100,0));
  strcat(stat,tmp);
  
  getLocalTime(&currentTime,NULL);
  sprintf(tmp,"Current Time: %04u/%02u/%02u %02u:%02u:%02u\n",currentTime.tm_year+1900,currentTime.tm_mon+1,currentTime.tm_mday,currentTime.tm_hour,currentTime.tm_min,currentTime.tm_sec);
  strcat(stat,tmp);

  uptime(stat);
  
  customServerStatus(stat);
}

void uptime(char msg[]){
  int ttlSec = ((esp_timer_get_time() - lastReboot) / (1000 * 1000));
  int secs = ttlSec % 60;
  ttlSec -= secs;
  int mins = (ttlSec % 3600) / 60;
  ttlSec -= (mins * 60);
  int hrs = (ttlSec % 86400) / 3600;
  ttlSec -= (hrs * 3600);
  int dys = ttlSec / 86400;
  char buf[60];
  sprintf(buf,"Uptime: %u days, %u hours, %u minutes, %u seconds\n",dys, hrs, mins, secs);
  strcat(msg,buf);
}

// Handles web socket events
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  bool handled = false;
  switch(type){
  case WS_EVT_CONNECT:
    //Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    break;
  case WS_EVT_DISCONNECT:
    //Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    break;
  case WS_EVT_ERROR:
    //Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    break;
  case WS_EVT_PONG:
    //Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    break;
  case WS_EVT_DATA:
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->opcode == WS_TEXT){
      char operand[25];         // Assuming a maximum operand length of 24 plus terminating 0. Increase if necessary. (Or use shorter names!)
      bool JSON = data[0]=='{'; // Rudimentary test to determine if the message is in JSON format

      if(JSON){
        char* reqAddr = strstr((char*)data,"request");                // Does the data contain "request"
        if((reqAddr != NULL) && (reqAddr - (char*)data < 10)){        // within the first ten characters? If so..
          JsonDocument doc;                                           // Allocate the JSON document
          DeserializationError Jerror = deserializeJson(doc, data);    // Test if parsing succeeds and deserialize the JSON document
          
          if(Jerror){                                                  // Test if parsing succeeded
            char buff[1000];
            snprintf(buff, 1000, "deserializeJson() failed:\n%s\n%s\n",Jerror.f_str(),(char*)data);
            serialPrint(buff,true);
          } else {
            // WebSocket request appears to be in a JSON format
            // Possible methods to retrieve values:
            //  const char* sensor = doc["sensor"];                   // Implicit casting to char*
            //  long time = doc["time"];                              // Specific casting is also available
            //  double latitude;                                      // i.e. x = doc["x"].as<int>
            //     latitude = doc["data"][0];
            //  double longitude = doc["data"][1];
            // The protocol in use here requires all JSON messages to have a "request" attribute that uniquely identifies what data is requested
            // Feel free to implement your own protocols.
            const char* request = doc["request"];
            if(strcmp(request,"getDateTime") == 0){
              // Client has requested date and time settings   This event is used in /admin.htm. Could be used in custom pages as well.
              char rsvp[400];
              getDateTimeSettings(rsvp,true);
              client->text(rsvp);
              handled = true;
            }
#if(UseFileManager)
            if((!handled) && strcmp(request,"FMgetBasic") == 0){
              // Client has requested basic data for the file manager. This event is used in /FileMan.htm.
              // Return a JSON string containing hostname, file extensions allowed for edit, and a list of available file systems (SD, LittleFS,...)
              char rsvp[400] = "";
              if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
                FMgetBasic(rsvp);
                client->text(rsvp);
              }
              handled = true;
            }
            if((!handled) && strcmp(request,"FMgetFS") == 0){
              // Client has requested data for a specific file system for the file manager. This event is used in /FileMan.htm.
              // Return a JSON string containing the file system, capacity, used, free. 
              const char* filesys = doc["filesys"];
              char rsvp[400] = "";
              if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
                FMgetFS(rsvp,filesys);
              }
              client->text(rsvp);
              handled = true;
            }
            if((!handled) && strcmp(request,"FMgetDir") == 0){
              // Client has requested a list of directories and files in a specific path on a specific file system
              if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
                const char* filesys = doc["filesys"];
                const char* dir = doc["dir"];
                client->text(FMgetDir(filesys,dir));
              }
              handled = true;
            }
            if((!handled) && strcmp(request,"FMmkDir") == 0){
              // Client has requested to create a new directory.
              if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
                const char* filesys = doc["filesys"];
                const char* dir = doc["dir"];
                const char* dname = doc["dname"];
                int fsNum = getFMfsNum((char*)filesys);
                FMmkDir(fsNum, dir, dname);
                client->text(FMgetDir(filesys,dir));
              }
              handled = true;              
            }
            if((!handled) && strcmp(request,"FMdelete") == 0){
              if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
                const char* filesys = doc["filesys"];
                const char* dir = doc["dir"];
                const char* fname = doc["fname"];
                int fsNum = getFMfsNum((char*)filesys);
                char fullPath[256];

                if((fsNum>=0)&&(dir>"")&&(fname>"")){
                  sprintf(fullPath,"%s%s%s",dir,dir[strlen(dir)-1]=='/' ? "" : "/", fname);
                  FMdelete(fsNum,fullPath);
                  client->text(FMgetDir(filesys,dir));
                }
              }
              handled = true;
            }
            if((!handled) && strcmp(request,"FMuploadBegin") == 0){
              // Client will be uploading a file and wants the server to track the status.
              const char *path = doc["path"];
              const unsigned int siz = doc["size"];
              createUpload((char*)path, (unsigned int)siz);
              handled = true;
            }
            if((!handled) && strcmp(request,"FMuploadEnd") == 0){
              // Client knows/confirms uploading is finished, free up this slot in the upload structure
              const char *path = doc["path"];
              deleteUpload((char*)path);
              handled = true;
            }
            if((!handled) && strcmp(request,"FMuploadStatus") == 0){
              // Client requests the status of this upload
              const char *path = doc["path"];
              char stat[100];
              statusUpload((char*)path, stat);
              client->text(stat);
              handled = true;
            }
#endif // UseFileManager
          }
        }
      } else {
      getOperand(operand, (char*)data, sizeof(operand));
      if(strcmp(operand,"getHostSettings") == 0){
        char rsvp[1000] = "";                   // Buffer to hold host settings
        if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
          // Client has requested host settings.   This event is used within /admin.htm
          getHostSettings(rsvp);
          client->text(rsvp);
        }
        handled = true;
      }
      if((!handled) && strcmp(operand,"setHostSettings") == 0){
        // Client wishes to change host settings   This event is used within /admin.htm
        char rsvp[200] = "";                    // Buffer to hold response for client
        if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
          if(setHostSettings((char*)data,rsvp)){  // If errors, messages will be returned in rsvp.
             sprintf(rsvp,"Message\nConfiguration changed. Rebooting. You will be disconnected.");
            client->text(rsvp);
            serialPrint("Rebooting after admin configuration change\n",true);
            delay(1000);
            ESP.restart();
          }else{
            strcat(rsvp,"\nConfiguration changes ignored");
            client->text(rsvp);
          }
        }
        handled = true;
      }

      if((!handled) && strcmp(operand,"getDateTime") == 0){
        // Client has requested date and time settings   This event is used in /admin.htm. Could be used in custom pages as well.
        char rsvp[500];
        getDateTimeSettings(rsvp,false);
        client->text(rsvp);
        handled = true;
      }

      if((!handled) && strcmp(operand,"setDateTime") == 0){
        // Client wishes to change date and time settings  This event is used in /admin.htm
        char rsvp[200] = "";
        if((!onlyLocalAdmin) || isLocal(client->remoteIP())){
          if(setDateTimeSettings((char*)data,rsvp)){  // If errors, messages will be returned in rsvp
            sprintf(rsvp,"Message2\nDate and Time configuration changed.");
            client->text(rsvp);
          }else{
            strcat(rsvp,"\nDate and Time changes ignored");
            client->text(rsvp);
          }
        }
        handled = true;
      }
    }
    }
  }
  if(!handled){
    customWS_EVT(server, client, type, arg, data, len);
  }
}

// Gets the "name" or "Identifier" of the data event received
// It is terminated either with a zero, or a carriage return (\r)
void getOperand(char operand[], char input[], int maxLen){
  for(int i = 0; i < maxLen; i++){
    operand[i] = input[i];
    if(operand[i] == '\r')
      operand[i] = 0;
    if(operand[i] == 0)
      break;
  }
}

// Assembles a response for the getHostSettings request
void getHostSettings(char rsvp[]){
  sprintf(rsvp,"setHostSettings\n%s\n%s\n%s\n%s\n%s\n%s\n%s",
          hostname, beacon ? "1" : "0",appasswd,ssid,password,adminUsername,adminPassword);
}

// Build up a websocket message response to send date and time settings to client
void getDateTimeSettings(char rsvp[], bool json){
  char pattern[100];
  if(json){
    strcpy(pattern, "{\"answer\":\"setDateTime\",\"timezone\":\"%s\",\"ntpserver\":\"%s\",\"time\":\"%s\",\"comment\":\"%s\"}");
  }else {
    strcpy(pattern, "setDateTime\n%s\n%s\n%s\n%s");
  }
  getLocalTime(&currentTime);
  char dttms[20];
  sprintf(dttms,"%04i%02i%02i%02i%02i%02i",
    currentTime.tm_year + 1900,
    currentTime.tm_mon  + 1,
    currentTime.tm_mday,
    currentTime.tm_hour,
    currentTime.tm_min,
    currentTime.tm_sec);
  sprintf(rsvp,pattern,timezone,ntpserver,dttms, WiFi.status() != WL_CONNECTED ? "\nNo WiFi, time cannot synch" : "");
}

// Handle changing host settings as specified by client in /admin.htm
bool setHostSettings(char *params, char msg[]){
  char* p[7]; // 7 Values will be received. Host, Broadcast, APPasswd, WiFiSSID, WiFiPasswd, AdminName, AdminPasswd
  int cnt = 0;
  bool retVal = false;                         // Assuming that not all tests pass
  msg[0] = 0;
  strcat(msg,"Message\n");
  strtok(params, "\n");                        // "setHostSettings". Return value not necessary
  for(int i = 0; i < 7; i++){                      // Pull in the 7 values
    p[i] = strtok(NULL,"\n");                  // They are delimited with \n
    if(p[i] > "") cnt++;
  };

  if(cnt < 5){                                 // First test, at least 5 values received? (SSID & password are optional!)
    strcat(msg,"Configuration is incomplete. ");
    return retVal;
  }
  if(strlen(p[0]) < 4)                         // Hostname must be at least 4 characters long
    return retVal;

  if(!((p[0][0] >= 'A' && p[0][0] <= 'Z') || (p[0][0] >= 'a' && p[0][0] <= 'z'))){
    strcat(msg,"Hostname must begin with a letter. ");
    return retVal;
  }

  if(strlen(p[2])<8 || strlen(p[2])>16){       // Password for access point must be at least 8 characters (rule in library!)
    strcat(msg,"Invalid AP Password. ");
    return retVal;
  }

  if(strlen(p[4]) > 0 && strlen(p[3]) == 0)    // If no SSID is given but a password is
    p[4][0] = 0;                               // Chop the password out

  if(strlen(p[5])<6 || strlen(p[5])>16){       // Admin name must be 6-16 characters long
    strcat(msg,"Invalid Admin Name. ");
    return retVal;
  }

  if(!((p[5][0] >= 'A' && p[5][0] <= 'Z') || (p[5][0] >= 'a' && p[5][0] <= 'z'))){
    strcat(msg,"Admin Name must begin with a letter. ");
    return retVal;
  }

  if(strlen(p[6])<8 || strlen(p[6])>16){       // Admin password must be 8-16 characters long
    strcat(msg,"Invalid Admin Password. ");
    return retVal;
  }

  Preferences param;
  param.begin(params,false);                   // use "param" preference namespace, readOnly = false means read/write allowed

  updateHostName(p[0], &param);
  updateBeacon(!(p[1][0] == '1'), &param);     // This byte is 0 for no beacon (AP not broadcast), 1 for beacon (AP broadcast)
  updateAPpassword(p[2], &param);
  updateSSID(p[3], &param);
  updatePassword(p[4], &param);
  updateAdminName(p[5], &param);
  updateAdminPassword(p[6], &param);

  retVal = true;
  return retVal;
}

// Handle changing time settings as specified by client in /admin.htm
bool setDateTimeSettings(char *params, char msg[]){
  char* p[2];                                  // Timezone, NTP Server
  int i = 0;
  int cnt = 0;
  bool retVal = false;                         // Assuming that not all tests will pass
  msg[0] = 0;
  strcat(msg,"Message2\n");
  strtok(params, "\n");                        // "setTimeSettings". Return value not necessary
  for(i = 0; i < 2; i++){                      // Pull in both parameters
    p[i] = strtok(NULL,"\n");
    if(p[i]>"")cnt++;
  };

  if(cnt < 2){                                 // First test, 2 values received?
    strcat(msg,"Time/date settings are incomplete. ");
    return retVal;
  }
  if(strlen(p[0]) < 4){                       // Timezone must be at least 4 characters long
    strcat(msg,"Invalid time zone. ");
    return retVal;
  }

  if(strlen(p[1])<8 || strlen(p[1])>16){      // NTP server must be 8-16 characters (pool.ntp.org should be shortest)
    strcat(msg,"Invalid time server. ");
    return retVal;
  }

  Preferences param;
  param.begin(params,false);                   // use "param" preference namespace, readOnly = false means read/write allowed

  updateTimezone(p[0], &param);
  updateNTPServer(p[1], &param);
  // (Falsely) Claim that the last NTP Synch was 30 days ago to force an update in loop.
  lastNTPSynch = millis() - 1000 * 60 * 60 * 24 * 30;

  retVal = true;
  return retVal;
}

// Because Serial.print AND SerialWS.print are both to be supported, all prints will be routed through these functions
// which will send the prints to one and/or the other.
// To keep this simple, only printing of strings and ints will be supported
// NOTE: SerialWS CAN support receiving from the SerialWS Monitor, but this application doesn't
// use receiving at all. It only uses SerialWS and/or Serial to output information to the serial monitor.
void serialPrint(char buff[], bool immediate){
#if serialEnabled
  Serial.print(buff);
#endif  
#if useSerialWS
  serialWS.print(buff);
  if(immediate)serialWS.send();
#endif  
}
void serialPrint(int val, bool immediate){
#if serialEnabled
  Serial.print(val);
#endif  
#if useSerialWS
  serialWS.print(val);
  if(immediate)serialWS.send();
#endif  
}
void serialPrintln(char buff[], bool immediate){
  serialPrint(buff, false);
  serialPrint("\n", immediate);
}
void serialPrintln(int val, bool immediate){
  serialPrint(val, false);
  serialPrint("\n", immediate);
}

void validateFiles(){
  FS *fs;
  #if StandardFiles == onSD
    fs = &SD;
    char sys[] = "SD";
  #elif StandardFiles == onLFS
    fs = &LittleFS;
    char sys[] = "LittleFS";
  #endif
  checkForFile(fs, sys, DefaultHTM);     // Default webpage when webserver is accessed (typically index.htm, main.htm, etc.) Remember the preceding slash!
  checkForFile(fs, sys, FavIcon);        // File for favicon (typically *.ico, *.jpg, *.png, or *.bmp)
  checkForFile(fs, sys, AdminHTM);       // File for admin webpage
  #if UseFileManager
    checkForFile(fs, sys, FilAdminHTM);  // File for file manager webpage
  #endif
  #if useSerialWS
    checkForFile(fs, sys, SerialHTM);    // File for SerialWS webpage
  #endif
}

void checkForFile(FS *fs_, char sys[], char filepath[]){
  if(!fs_->exists(filepath)){
    char buff[500];
    snprintf(buff, 500, "Warning: File %s is missing on %s\n", filepath, sys);
    serialPrint(buff,true);
  }
}
