#include <Preferences.h>                   // Included in main ino. Here for reference only.

// Names of the various parameters within preferences
const char *params        = "param";         // Preferences namespace
const char *P_CHIPID      = "CHIPID";        // Chip ID
const char *P_HOSTNAME    = "HOST";          // WiFi Hostname (Access Point, etc). Length of 32 (uint8_t zero padded right)
const char *P_APPASSWD    = "APPASS";        // AP Password. Length of 64 (uint8_t zero padded right)
const char *P_BEACON      = "BEACON";        // This byte is 0 for no beacon (AP not broadcast), 1 for beacon (AP broadcast)
const char *P_SSID        = "SSID";          // WiFi SSID. Length of 32 (uint8_t zero padded right)
const char *P_PASSWORD    = "PASSWD";        // WiFi Password. Length of 64 (uint8_t zero padded right)
const char *P_ADMINNAME   = "ADMIN";         // Name of Admin logon. Length of 32 (uint8_t zero padded right)
const char *P_ADMINPASSWD = "ADPASS";        // Password for Admin logon. Length of 32 (uint8_t zero padded right)
const char *P_TIMEZONE    = "TIMEZN";        // Timezone Data: CET-1CEST,M3.5.0/3,M10.5.0/3  CST6CDT,M3.2.0,M11.1.0 see: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char *P_NTPSERVER   = "NTPSRVR";       // NTP Server to synchronize to. Allocating 64 bytes but limited to less in Admin.htm

bool initParams(){
  bool reInit = false;
  Preferences param;
  uint64_t chipid = ESP.getEfuseMac();       // The chip ID is essentially its MAC address(length: 6 bytes).
  char readid[20];
  char chipIdHex[20];
  sprintf(chipIdHex,"%08X%08X",(uint32_t)(chipid>>32),(uint32_t)(chipid&0xFFFFFFFFLL));
  param.begin(params,false);                 // use "param" preference namespace, readOnly = false means read/write allowed

  param.getBytes(P_CHIPID, readid, sizeof(readid));
  if((def_FORCE) || (!(strcmp(readid,chipIdHex)==0))){
    reInit = true;
#if serialEnabled && setupVerbose
    serialPrint("Creating initial parameters\n",true);
#endif
    param.clear();                           // Delete any/all existing parameters in this namespace
    updateChipID(chipIdHex, &param);
    updateHostName(def_HostName, &param);
    updateAPpassword(def_APpassword, &param);
    updateBeacon(def_Beacon, &param);
    updateSSID(def_SSID, &param);
    updatePassword(def_Password, &param);
    updateAdminName(def_AdminName, &param);
    updateAdminPassword(def_AdminPassword, &param);
    updateTimezone(def_Timezone,&param);
    updateNTPServer(def_NTPServer, &param);
  }

  param.getBytes(P_CHIPID, readid, sizeof(readid));
  param.getBytes(P_HOSTNAME, hostname, sizeof(hostname));
  param.getBytes(P_APPASSWD, appasswd, sizeof(appasswd));
  char beacon_[1];
  param.getBytes(P_BEACON, beacon_, 1);
  beacon = beacon_[0] & B00000001;    // Beacon is stored as false '0' (ASCII 48) or true '1' (ASCII 49).
  param.getBytes(P_SSID, ssid, sizeof(ssid));
  param.getBytes(P_PASSWORD, password, sizeof(password));
  param.getBytes(P_ADMINNAME, adminUsername, sizeof(adminUsername));
  param.getBytes(P_ADMINPASSWD, adminPassword, sizeof(adminPassword));
  param.getBytes(P_TIMEZONE, timezone, sizeof(timezone));
  param.getBytes(P_NTPSERVER, ntpserver, sizeof(ntpserver));

  param.end();
  return reInit;  // Tell caller if we had to reinitialize parameters or not
}

void updateChipID(char *newChipID, Preferences *param){
  param->putBytes(P_CHIPID, newChipID, 20);
}

void updateHostName(char *newhostname, Preferences *param){
  if(!(strcmp(hostname, newhostname) == 0)){
    strcpy(hostname,newhostname);
    param->putBytes(P_HOSTNAME, hostname, sizeof(hostname));
  }
}

void updateAPpassword(char newappassword[], Preferences *param){
  if(!(strcmp(appasswd, newappassword) == 0)){
    strcpy(appasswd, newappassword);
    param->putBytes(P_APPASSWD, appasswd, sizeof(appasswd));
  }
}

void updateBeacon(bool newAPbeacon, Preferences *param){
  if(!(beacon == newAPbeacon)){
    const char *val = newAPbeacon ? "1" : "0";
    param->putBytes(P_BEACON, val,1);
    beacon = newAPbeacon;
  }
}

void updateSSID(char newSSID[], Preferences *param){
  if(!(strcmp(ssid, newSSID) == 0)){
    strcpy(ssid, newSSID);
    param->putBytes(P_SSID, ssid, sizeof(ssid));
  }
}

void updatePassword(char newPassword[], Preferences *param){
  if(!(strcmp(password, newPassword) == 0)){
    strcpy(password, newPassword);
    param->putBytes(P_PASSWORD, password, sizeof(password));
  }
}

void updateAdminName(char newAdminName[], Preferences *param){
  if(!(strcmp(adminUsername, newAdminName) == 0)){
    strcpy(adminUsername, newAdminName);
    param->putBytes(P_ADMINNAME, adminUsername, sizeof(adminUsername));
  }
}

void updateAdminPassword(char newAdminPassword[], Preferences *param){
  if(!(strcmp(adminPassword, newAdminPassword) == 0)){
    strcpy(adminPassword, newAdminPassword);
    param->putBytes(P_ADMINPASSWD, adminPassword, sizeof(adminPassword));
  }
}

void updateTimezone(char newTimezone[], Preferences *param){
  if(!(strcmp(timezone, newTimezone) == 0)){
    strcpy(timezone, newTimezone);
    param->putBytes(P_TIMEZONE, timezone, sizeof(timezone));
  }
}

void updateNTPServer(char newNTPServer[], Preferences *param){
  if(!(strcmp(ntpserver, newNTPServer) == 0)){
    strcpy(ntpserver, newNTPServer);
    param->putBytes(P_NTPSERVER, ntpserver, sizeof(ntpserver));
  }
}
