

const char *Device_Type = "CFOW_4";

const char *Device_Firmware_Version = "0.0.1";




unsigned long previousMillis = 0;
unsigned long lastSync = 0;
unsigned long lastAWSReconnect = 0;
unsigned long firmwareUpdateInterval = 86400000; // 1 hour (adjustable)
unsigned long syncInterval = 600000;  // 10 minutes
unsigned long TimeInterval = 10000 ; //300000;  // 10 minutes

unsigned long awsReconnectInterval = 10000; // 1 minute for AWS reconnect

bool isInitialized = false;
bool AwsCommand = false;

struct Timer {
  long start;
  long end;
};


Timer relayTimers[3][5]; // For example, 3 relays, each with up to 5 time intervals 
