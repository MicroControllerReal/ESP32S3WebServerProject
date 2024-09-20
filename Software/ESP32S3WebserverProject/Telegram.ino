// This is the structure of a Telegram Message as defined by the UniversalTelegramBot library:
/*struct telegramMessage{
  String text;
  String chat_id;
  String chat_title;
  String from_id;
  String from_name;
  String date;
  String type;
  String file_caption;
  String file_path;
  String file_name;
  bool hasDocument;
  long file_size;
  float longitude;
  float latitude;
  int update_id;
  int message_id;  
  int reply_to_message_id;
  String reply_to_text;
  String query_id;
};*/

#if (UseTelegram == TelegramReceive) || (UseTelegram == TelegramSendReceive)
bool getTelegramMsgs(){
  WiFiClientSecure secured_client;
  UniversalTelegramBot bot("", secured_client);
  bool msgReceived = false;
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);      // Add root certificate for api.telegram.org
  for(int b = 0; b < numBots; b++){
    if(Bots[b].receives){
      bot.updateToken(Bots[b].token);
      int numNewMessages = bot.getUpdates(Bots[b].lastReceived + 1);
      Bots[b].lastReceived = bot.last_message_received;
      while(numNewMessages){
        msgReceived = true;
        for(int m = 0; m < numNewMessages; m++){
          if((bot.messages[m].from_id == tgChatID)          // Primary Telegram user.
          || (bot.messages[m].from_id == tgAltChatID))      // Alternate Telegram user.
            handleTelegramMessage(b, bot, m);
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
    }
  }
  return msgReceived;
}

bool handleTelegramMessage(int botNum, UniversalTelegramBot bot, int msgNum){
  bool msgHandled = false;
  if((botNum == 0) && (bot.messages[msgNum].text == "/status")){
      char stat[1000];
      serverStatus(stat);
      bot.sendMessage(bot.messages[msgNum].from_id, stat, "");
      msgHandled = true;
  }
  if(!msgHandled){
    msgHandled = customTelegramRcvd(botNum, bot, bot.messages[msgNum]);
  }
  return msgHandled;
}
#endif

#if (UseTelegram == TelegramSend) || (UseTelegram == TelegramSendReceive)

void sendTelegramMsg(int botNum, char chatId[], char msg[]){
  WiFiClientSecure secured_client;
  UniversalTelegramBot bot("", secured_client);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);      // Add root certificate for api.telegram.org
  bot.updateToken(Bots[botNum].token);
  bot.sendMessage(chatId, msg, "");                         // Send the message to the designated chatId
}

void sendTelegramMsgKbd(int botNum, char chatId[], char msg[], TelegramBtn btns[], int numBtns){
  WiFiClientSecure secured_client;
  UniversalTelegramBot bot("", secured_client);
  String kbd = "[";
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);      // Add root certificate for api.telegram.org
  bot.updateToken(Bots[botNum].token);
  for(int i = 0; i < numBtns; i++){
    kbd += "[{\"text\":\"";
    kbd += btns[i].label;
    kbd += "\",\"callback_data\":\"";
    kbd += btns[i].response;
    kbd += "\"}]";
    if(i < numBtns -1) kbd += ",";
  }
  kbd += "]";
  bot.sendMessageWithInlineKeyboard((String)chatId, msg, "", kbd);  // Send the message with the keyboard in a JSON String
}
#endif
