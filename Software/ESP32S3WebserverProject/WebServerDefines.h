// Define here what type, if any, SD card is to be used.
#define TypeNoSD 0                      // Value indicating that no SD will be used.
#define TypeSD   1                      // Value indicating that a SD card will be used. Card can be up to 32GB. Standard SPI pins will be used
#define TypeMMC  2                      // Value indicating that a MMC card will be used. A MMC card is faster than SD, can be up to 2TB. Pins defined below

// Define what file system is being used for standard files. Don't change the values for onSD or onLFS!
#define onSD     1                      // Defines that standard files (/admin, /favicon.ico, DefaultHTM, etc.) are on SD/MMC
#define onLFS    2                      // Defines that standard files (/admin, /favicon.ico, DefaultHTM, etc.) are in LittleFS (Flash)

// Define if and how Telegram messages may be implemented on the server.
#define noTelegram          0           // Value indicating Telegram will not be implemented
#define TelegramSend        1           // Value indicating Telegram will be implemented to send messages only
#define TelegramReceive     2           // Value indicating Telegram will be implemented to receive messages only
#define TelegramSendReceive 3           // Value indicating Telegram will be implemented to send and receive messages

// Structure defining Telegram Bots used:
typedef struct{
  String token;                         // The bot token for your Telegram bot was provided to you by Telegrams bot father. It identifies your bot.
  bool receives;                        // True if this bot RECEIVES messages. False if it is only for sending messages.
  long lastReceived;                    // Used internally to track new messages. Need not be assigned when creating bots.
}Bot;

// Structure defining simple buttons for use with Telegram
typedef struct{
  char label[21];
  char response[21];
}TelegramBtn;
