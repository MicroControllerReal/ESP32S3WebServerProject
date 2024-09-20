void startWiFi(){
  uint8_t apname[32];                                     // Variable for concatenating various versions of hostname into
  unsigned long start;                                    // Variable for millisecond timing

#if serialEnabled && setupVerbose
  serialPrint("Starting Access Point and WiFi\n",true);
#endif
  strcpy((char*)apname, hostname);                        // Copy the host name to a temporary variable
  strcat((char*)apname,"AP");                             // Append "AP" onto the host name. This will be the SoftAP SSID.
  WiFi.setHostname(hostname);                             // Set the desired hostname. (MUST set this before WiFi.mode!!!)
  WiFi.mode(WIFI_AP_STA);                                 // Provide an access point AND be available over the network IP
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.softAP((char*)apname,(char*)appasswd,1,beacon,8);  // Initiate the access point (WiFi Channel 1), maximum of 8 connections
#if serialEnabled && setupVerbose
  // Show the user the hostname and access point info
  {  char buff[1000];
     snprintf(buff, 1000, "Hostname: %s\nAccess Point %s established on: %s (%s)\n",WiFi.getHostname(),WiFi.softAPSSID(), WiFi.softAPIP().toString(), beacon ? "public" : "hidden");
     serialPrint(buff,true);
  }
#endif
  if(ssid[0] > 0){                                        // If a WiFi SSID has been configured
#if serialEnabled && setupVerbose
    // Show the user the info for the WiFi connecting to
  {  char buff[1000];
     snprintf(buff, 1000, "Connecting to %ssecured WiFi: %s", password[0] ? "" : "un", ssid);
     serialPrint(buff,true);
  }
#endif
    for(int i = 0; i < 3; i++){
      WiFi.disconnect();                                  // Start with a clean WiFi stack
      WiFi.begin((char*)ssid, (char*)password);
      delay(100);
      start = millis();
      while(WiFi.status() != WL_CONNECTED                 // If not yet connected
            && millis() - start < 5000){                  // and 5 seconds have not yet passed
              delay(100);                                 // Wait a bit
#if serialEnabled && setupVerbose
              serialPrint(".",false);                     // Notify user that we are still trying
#endif
      }
    }
#if serialEnabled && setupVerbose
    serialPrint("\n",true);
#endif
  } else {
#if serialEnabled && setupVerbose
    serialPrint("Local WiFi not defined\n",true);         // Notify user that no WiFi has yet been defined
#endif
  }

#if serialEnabled && setupVerbose
  if(WiFi.isConnected()){                                 // If connected to WiFi
    // Show the user the WiFi connected to and the server IP.
    {  char buff[1000];
       snprintf(buff, 1000, "Connected to %s on %s\n", WiFi.SSID(), WiFi.localIP().toString());
       serialPrint(buff,true);
    }
  }
#endif

/*typedef enum {
    WIFI_POWER_19_5dBm = 78,   // 19.5dBm
    WIFI_POWER_19dBm = 76,     // 19dBm
    WIFI_POWER_18_5dBm = 74,   // 18.5dBm
    WIFI_POWER_17dBm = 68,     // 17dBm
    WIFI_POWER_15dBm = 60,     // 15dBm
    WIFI_POWER_13dBm = 52,     // 13dBm
    WIFI_POWER_11dBm = 44,     // 11dBm
    WIFI_POWER_8_5dBm = 34,    // 8.5dBm
    WIFI_POWER_7dBm = 28,      // 7dBm
    WIFI_POWER_5dBm = 20,      // 5dBm
    WIFI_POWER_2dBm = 8,       // 2dBm
    WIFI_POWER_MINUS_1dBm = -4 // -1dBm
} wifi_power_t;*/
  WiFi.setTxPower(WIFI_POWER_19dBm);                      // Set WiFi power to maximum. NOTE: Change to abide by local laws if necessary!
}

void initMDNS(){
  unsigned long start;                                    // Variable for millisecond timing
  bool started = false;                                   // Assume that mDNS has not started
  for(int i = 0; i < 3; i++){
    start = millis();
    while ((!started) && ((millis() - start) < 5000)){
      started = MDNS.begin(hostname);
      if(!started) delay(100);
    }
  }
  if(started){
    MDNS.addService("http", "tcp", 80);
#if serialEnabled && setupVerbose
    {  char buff[1000];
       snprintf(buff, 1000, "mDNS responder started for: %s\n",hostname);
       serialPrint(buff,true);
    }
#endif
  }
}

void initOTA(){
#if UseOTA
#if serialEnabled && setupVerbose
  serialPrint("Initiating OTA\n",true);
#endif
  byte temp[32];
  strcpy((char*)temp,hostname);                           // Copy the host name to a temporary variable
  strcat((char*)temp, "OTA");                             // Append "OTA" onto the host name. This will be the OTA "Port"
  ArduinoOTA.onStart([]() {activeOTA =true; events.send("Update Start", "ota");});
  ArduinoOTA.onEnd([]() {activeOTA =false;events.send("Update End", "ota");});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char p[32];
    sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
    events.send(p, "ota");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    char temp[32] = "Unknown";
    activeOTA =false;
    if(error == OTA_AUTH_ERROR) strcpy(temp,"Auth Failed");
    else if(error == OTA_BEGIN_ERROR) strcpy(temp,"Begin Failed");
    else if(error == OTA_CONNECT_ERROR) strcpy(temp,"Connect Failed");
    else if(error == OTA_RECEIVE_ERROR) strcpy(temp,"Receive Failed");
    else if(error == OTA_END_ERROR) strcpy(temp,"End Failed");
    events.send(temp,"ota");
    {  char buff[1000];
       snprintf(buff, 1000, "OTA Error: %s\n", temp);
       serialPrint(buff,true);
    }
  });
  ArduinoOTA.setTimeout(5000);            // Default timeout is 1000ms and timeouts often occured. Increase to 5 seconds.
  ArduinoOTA.setHostname((char*)temp);    // Set the name of this device for OTA putposes
#if serialEnabled && setupVerbose
  serialPrint("Starting OTA\n",true);
#endif
  activeOTA =false;
  ArduinoOTA.begin();
#endif
}

bool synchDateTime(){
  tm timeinfo;
  bool retVal = false;                                    // Assume the Synch didn't go through
  if(WiFi.isConnected()){
    configTime(0, 0, ntpserver);                          // Configure which NTP Server to use and 0 offset standard, 0 offset DST
    if(getLocalTime(&timeinfo)){                          // Get time from NTP. It is now in timeinfo. "Local" here means UTC because 0,0 offsets above
      setenv("TZ",timezone,1);                            // Set the TimeZone.
      tzset();                                            // Internal time is now adjusted to show the local time, including DST
      getLocalTime(&timeinfo,NULL);                       // Get local time again. This time it is adjusted for timezone and DST!
      lastNTPSynch = millis();                            // Set global variable remembering when last NTP Synch was successful
      retVal = timeinfo.tm_year > 120;
    }
  }
  return retVal;
}

bool isLocal(IPAddress clientIP){
  bool retVal = false;                                            // Start off assuming the client IP is NOT local

  if(WiFi.isConnected()){                                         // If WiFi is active
    IPAddress serverWiFiIP = WiFi.localIP();                      // Get the IP address of this server
    IPAddress serverWiFiSubnet = WiFi.subnetMask();               // Get the Subnet Mask of this server
    long int intermediate = (serverWiFiIP & serverWiFiSubnet);    // Bitwise and them together. This is the base IP of this server
    retVal = (intermediate == (clientIP & serverWiFiSubnet));     // Bitwise and the client and this subnet. The result must be identical to the base IP.
  }

  if(!retVal){
    IPAddress serverSoftIP = WiFi.softAPIP();                     // Get the IP address of this access point
    IPAddress serverSoftSubnet = WiFi.softAPSubnetMask();         // Get the Subnet Mask of this access point
    long int intermediate = (serverSoftIP & serverSoftSubnet);    // Bitwise and them together. This is the base IP of this access point
    retVal = (intermediate == (clientIP & serverSoftSubnet));     // Bitwise and the client and this subnet. The result must be identical to the base IP.
  }
  return retVal;
}
