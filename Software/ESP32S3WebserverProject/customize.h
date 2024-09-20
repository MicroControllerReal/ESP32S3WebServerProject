// ------------------------- Admin section -------------------------
// These values are used as defaults for the access point and WiFi.
// Note that these defaults will be burned into the sketch on the board.
// It is convenient to leave these defaults as is and after
// loading the sketch onto the board using the default access point
// and changing these values using the admin.htm web page.
// Then, sharing the sketch doesn't share your security credentials.
// Normally, these defaults are only good until the first boot,
// because once they are burned onto the board their presence will
// be detected and the values from the board will be used.
// Changing them after the first boot normally has no effect.
// In order to force the parameters to be used during the next boot
// set def_FORCE true. Change it back to false after the next boot!
#define def_HostName      "WebServer"                                   // Default name of this server and access point
#define def_APpassword    "TopSecret"                                   // Default password for this access point
#define def_Beacon        true                                          // Default beacon for this access point (false hides AP)
#define def_SSID          ""                                            // Local WiFi. Blank if none
#define def_Password      ""                                            // Password for local WiFi (if any)
#define def_AdminName     "Admin"                                       // Administrative admin logon
#define def_AdminPassword "Secret"                                      // Administrative admin password
#define def_Timezone      "EDT+5:00EST+4:00,M3.2.0/2:00,M11.1.1/2:00"   // Local time zone (here, Germany)
#define def_NTPServer     "us.pool.ntp.org"                             // Name of default NTP server to synchronize to
#define def_FORCE         true                                          // Force these parameters to be used after the next boot
#define onlyLocalAdmin    true                                          // Admin.htm, fileman.htm, SerialWS and certain websocket requests (security
                                                                        // related) may only be accessed from an IP on the local network
//----------------------------------------------------------------------------------------------------

// ------------------------- Safety net section -------------------------
#define useSafetyNet true             // Safety net forces a reboot based on...
#if useSafetyNet
  #define safetyMinFreeHeapKB    50   // Minimum heap that must be available. Reboot if lower! 0 turns this feature off
  #define safetyMinAllocHeapKB   5    // Minimum block of heap that must be available. Reboot if lower! 0 turns this feature off
  #define safetyMaxRebootDays    0    // Force a reboot every n days between 3:00:00-3:59:59. 0 turns this feature off.
#endif
//----------------------------------------------------------------------------------------------------

// ------------------------- Mass storage section -------------------------
#define SDType TypeNone               // Possible values are TypeNoSD, TypeSD, or TypeMMC only
#if (SDType == TypeSD)                // If using standard SD following defines are necessary
  #define SDformatIfFail false        // True if SD card should be formatted if mounting fails
  #define SDcs 10                     // Chip select pin for standard SD card (as of Version 3.0, changed to 10 on ESP32Webserver hardware!)   
  #define SDMHz 40                    // The SD library has a speed of 4MHz but tests have shown 40MHz to be stable
  #define SDmaxOpenFiles 5            // The SD library has 5 as the default so good starting point. However...
                                      // if a webpage calls for several files back to back (i.e. during initial load
                                      // requesting .css files, .js files, maybe images) then the 5 is quickly spent
                                      // and may need to be raised.
#endif
#if (SDType == TypeMMC)               // If using MMC, following defines are necessary
  #define MMCformatIfFail false       // True if SD card should be formatted if mounting fails
  #define MMCmode1bit false           // True for 1 bit, false for 4 bit (faster) SPI communication with SD card
  #define MMCChipDetect 18            // Pin to detect if a SD card is inserted. Set to -1 if chip detect is not needed --
  #define MMCclk 14                   // CLK pin for MMC card                                                             |
  #define MMCcmd 15                   // CMD pin for MMC card                                                             |
  #define MMCd0   2                   // Data0 pin for MMC card                                                           |-- Pins as wired on The ESP32S3 Webserver
  #if (!MMCmode1bit)                  // If not using the slower 1 bit mode                                               |   hardware TFT Backpack
    #define MMCd1   4                 // Data1 pin for MMC card                                                           |
    #define MMCd2  47                 // Data2 pin for MMC card                                                           |
    #define MMCd3  21                 // Data3 pin for MMC card                                                         --
  #else
    #define MMCd1 GPIO_NUM_NC         // Data1 pin for MMC card is not used in 1 bit mode
    #define MMCd2 GPIO_NUM_NC         // Data2 pin for MMC card is not used in 1 bit mode
    #define MMCd3 GPIO_NUM_NC         // Data3 pin for MMC card is not used in 1 bit mode
  #endif
  #define MMCMHz BOARD_MAX_SDMMC_FREQ // SPI speed, in MHz, for MMC access. BOARD_MAX_SDMMC_FREQ is defined for each board and is recommended.
  #define MMCmaxOpenFiles 5           // The SD_MMC library has 5 as the default so good starting point. However...
                                      // if a webpage calls for several files back to back (i.e. during initial load
                                      // requesting .css files, .js files, maybe images) then the 5 is quickly spent
                                      // and may need to be raised.
#endif
#define UseLittleFS true              // Will LittleFS (files in flash) be used. Would be in addition to a SD card, if any are being used.
#if UseLittleFS
  #define fmtLittleFS true            // If LittleFS is used and doesn't mount, should it be formatted?
#endif
//
//----------------------------------------------------------------------------------------------------


// ------------------------- Serial section -------------------------
// Define Serial use and setup verbosity.
// NOTE: Serial (via UART/USB) and SerialWS are separate. You can use one, the other, or both.
#define serialEnabled true            // If true, Serial will be enabled during setup. If false, Serial will not be enabled
                                      // Some errors may occassionally be shown on the Serial port, so it is recommended.
#if serialEnabled
  #define serialBaud 115200           // Speed of Serial, if implemented 
  #define setupVerbose true           // Controls Serial output during setup. Typically used during development and then disabled
#endif                                // once the project is completed (and there is no Serial connection anyway). Possible values are true or false

// NOTE: setupVerbose is not available on SerialWS because SerialWS isn't available until the web server is fully
// initialized.
#define useSerialWS true              // If true, serial will also be available over the web using webpage /serialWS
#if useSerialWS                       // It is possible to use SerialWS without SerialEnabled
  #define SerialWSrxSize  16          // If using SerialWS, this defines the size of the receive buffer
  #define SerialWStxSize 256          // and the transmit buffer
#endif
//
//----------------------------------------------------------------------------------------------------



// ------------------------- Options section -------------------------
// Define Server options to be used
#define UseOTA true                     // Using OTA doubles the amount of Flash used for program and reduces space for LittleFS
                                        // but allows for reflashing the server firmware without needing to attach to a USB cable!

#define UseStatus true                  // /status is a dynamic webpage designed for debugging purposes and displays a server status

// Define if and how the File Manager should be used
#define UseFileManager true             // File Manager is a web page and collection of websocket services that allows for listing, uploading
                                        // and downloading of files to either SD/MMC (as defined above) and/or LittleFS (as defined above)
#if (UseFileManager)
  const char FMblockedPath[] ="";       // Path that will be invisible and deemed nonexistent in File Manager web pages
                                        // Use this path for system files (html, css, js, images, etc.) that the File Manager cannot delete, edit or even see.
                                        // This path will be excluded on all file systems used by File Manager (SD and LittleFS)
                                        // "" will disable this feature. Be sure to include leading and trailing "/" (i.e. "/sys/")
  #define embedFileman true             // If true, fileman.htm will be embedded in the program. This is a convenient way to load
                                        // files initially, but increases the program size by several KB. Set to false
                                        // and reload the software once the files are initially loaded to the server!
#endif


// ------------------------- Webpage section -------------------------
// Define what file system is being used for standard files.
#define StandardFiles onLFS             // Defines where admin.htm, favicon.ico and DefaultHTM are located. Possible values are onSD or onLFS

// Caching files on the local web client reduces network traffic and improves the performance of the server
// max_age is always in seconds.
const char* cacheRule = "max-age=31536000, must-revalidate";    //31,536,000 = seconds in 365 days so max-age = 1 year.

// Define some standard files to be used. You must copy these files to the location specified above for StandardFiles! (SD or LittleFS)
#define DefaultHTM "/Main.htm"          // Default file when webserver is accessed (typically index.htm, main.htm, etc.) Remember the preceding slash!
                                        // You typically put the path of your projects webpage as the DefaultHTM.
#define FavIcon "/favicon.ico"          // File for favicon (typically *.ico, *.jpg, *.png, or *.bmp)
#define AdminHTM "/admin.htm"           // File for admin webpage
#if (UseFileManager)
  #define FilAdminHTM "/fileman.htm"    // File for file manager webpage if using the fileman option
                                        // Will be ignored if embedFileman was set to true
#endif
#if useSerialWS
  #define SerialHTM "/SerialWS.htm"     // File for SerialWS webpage if using the SerialWS option
#endif

// NOTE: If BOTH LittleFS AND SD/MMC are being used, files other than the standard files which are used within webpages
// must be prefixed so the webserver knows where to look. Use /f/ for LittleFS files and /s/ for files on SD/MMC!
// Examples: index.htm includes a reference to a file on SD with /s/file.htm but on SD it is as /file.htm (without /s)
//           index.htm includes a reference to a file on LittleFS with /f/file.htm but in LittleFS it is as /file.htm (without /f)
// NOTE: The /status.htm page is dynamically generated by software and is not on SD or in LittleFS!
// 
//----------------------------------------------------------------------------------------------------


// ------------------------- Telegram section -------------------------
#define UseTelegram TelegramSendReceive          // Possible values are noTelegram, TelegramSend, TelegramReceive, or TelegramSendReceive only
#if ((UseTelegram == TelegramSend) || (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive))
  // The bot token for your Telegram bot was provided to you by Telegrams bot father. It identifies your bot.
  // Define all Telegram bots to be used here:
  // First value is the bot token, second = true/false indicating if this bot RECEIVES messages, third should always be zero
  Bot Bots[] = {
    // This is obviously only an EXAMPLE of what the bot token looks like. It is (hopefully) not real!
    {"9999999999:AAA9aABc_dZZaaZ9aZzA9z9A9abC9z-Z9aa",true,0}
  };
  // As a security measure, any messages received that are not from CHAT_ID or ALT_CHAT_ID will be ignored!
  #define CHAT_ID     "888888888"       // Primary Telegram user. System messages will be sent to this Telegram user
  #define ALT_CHAT_ID "999999999"       // Alternate Telegram user.
  #if((UseTelegram == TelegramSend) || (UseTelegram == TelegramSendReceive))
    #define TelegramOnBoot true         // If true, a Message will be sent with Server Status to CHAT_ID each time the server boots
    #define TelegramAltOnBoot false     // If true, the boot message will also be sent to the ALT_CHAT_ID
  #endif
  #if((UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive))
    #define TGRECEIVEFREQ 60           // How many seconds between polling for received messages?
  #endif
#endif

//----------------------------------------------------------------------------------------------------
