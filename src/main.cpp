#define Test false

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include "S_definitions.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include "Secrets.h"
#include <EEPROM.h>
#include <RTClib.h>
#include <time.h>
#include <Wire.h>
#include "urls.h"
#include <SPI.h>
#include "FS.h"

BearSSL::CertStore certStore;

// GIT Update
WiFiClient espClient;
PubSubClient clients(espClient);

WiFiServer telnetServer(23); // Telnet server runs on port 23
WiFiClient telnetClient;

// AWS Init ..
WiFiClientSecure net;
PubSubClient client(net);

// AWS Code Initialization
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
BearSSL::X509List cert(cacert);

RTC_DS3231 rtc;

// int const MainPin = D7;
// int const RelayPin1 = D1;
// int const RelayPin2 = D2;
// int const RelayPin3 = D3;

int const MainPin = D0;
int const RelayPin1 = D6;
int const RelayPin2 = D5;

const String FirmwareVer = {"0.3"};

String Device_Uid = "";
String Domain = "iothome.loca.lt";
String User_Id = "";

// T1

String StartTiming = "";
String EndTiming = "";
String SwitchAssigned = "";

// T2 Timer Check
String StartTiming2 = "";
String EndTiming2 = "";
String SwitchAssigned2 = "";

// T3
String StartTiming3 = "";
String EndTiming3 = "";
String SwitchAssigned3 = "";

// T4
String StartTiming4 = "";
String EndTiming4 = "";
String SwitchAssigned4 = "";

void checkTimer(String StartTiming, String EndTiming, String SwitchAssigned, String current_hours, String current_minutes, int timerNumber);
String postMethods(const char *serverUrl, String SendData);
int eepromWritter(int startLp, int EndLp, String Data);
String TimerStringChecker(int startLp, int EndLp);
void subscribetopics(String topics);
void subscribeTopicInitialization();
int turnOffRelays(int RelayPinsfn);
int turnOnRelays(int RelayPinsfn);
void messageEvent(String datas);
void clientCheckAwsConnected();
void PinModeDeclaration();
void Initialization();
void InstanceBegin();
void TimerChecker();
void eepromClear();
void TimerBegins();
void connectAWS();
void setClock();

void initEepromData()
{

  // T1 checker   (162 - 174)
  StartTiming = TimerStringChecker(162, 166);
  EndTiming = TimerStringChecker(168, 172);
  SwitchAssigned = TimerStringChecker(174, 175);

  // T2 Timer Check
  StartTiming2 = TimerStringChecker(177, 181);
  EndTiming2 = TimerStringChecker(183, 187);
  SwitchAssigned2 = TimerStringChecker(189, 190);

  // T3
  StartTiming3 = TimerStringChecker(192, 196);
  EndTiming3 = TimerStringChecker(198, 202);
  SwitchAssigned3 = TimerStringChecker(204, 205);

  // T4
  StartTiming4 = TimerStringChecker(207, 211);
  EndTiming4 = TimerStringChecker(213, 217);
  SwitchAssigned4 = TimerStringChecker(219, 220);
}

String assignRelay(const char *switchAssigned)
{
  if (strcmp(switchAssigned, "Switch1") == 0)
    return "R1";
  if (strcmp(switchAssigned, "Switch2") == 0)
    return "R2";
  if (strcmp(switchAssigned, "Switch3") == 0)
    return "R3";
  if (strcmp(switchAssigned, "Switch4") == 0)
    return "R4";

  return "Unknown";
}

int assignRelayint(String switchAssigned)
{
  if (switchAssigned == "R1")
  {
    Serial.println("Assigning MainPin to R1");
    return MainPin;
  }
  if (switchAssigned == "R2")
  {
    Serial.println("Assigning RelayPin1 to R2");
    return RelayPin1;
  }
  if (switchAssigned == "R3")
  {
    Serial.println("Assigning RelayPin2 to R3");
    return RelayPin2;
  }

  Serial.println("Invalid switch assigned");
  return -1;
}

/*                                 AWS CODE START                        */

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (unsigned int i = 0; i < length; i++) // Fixed condition: i < length
  {
    Serial.print((char)payload[i]); // Print each character of the payload
    message += (char)payload[i];    // Append character to the message string
  }

  messageEvent(message); // Pass the constructed message to event handler
  Serial.println();
}

void messageEvent(String datas)
{
  delay(10);
  Serial.println(datas);
  int commaIndex = datas.indexOf(',');

  // Use substring to get parts before and after the comma
  String part1 = datas.substring(0, commaIndex); // "72"
  String data = datas.substring(commaIndex + 1); // "R5_On"

  if (data.indexOf("R1_On") != -1)
  {
    Serial.println("R1 On detected switch");
    turnOnRelays(MainPin);
  }
  else if (data.indexOf("R1_Off") != -1)
  {
    Serial.println("R1 Off detected switch");
    turnOffRelays(MainPin);
  }
  else if (data.indexOf("R2_On") != -1)
  {
    Serial.println("R2 On detected switch");
    turnOnRelays(RelayPin1);
  }
  else if (data.indexOf("R2_Off") != -1)
  {
    Serial.println("R2 Off detected switch");
    turnOffRelays(RelayPin1);
  }
  else if (data.indexOf("R3_On") != -1)
  {
    Serial.println("R3 On detected switch");
    turnOnRelays(RelayPin2);
  }
  else if (data.indexOf("R3_Off") != -1)
  {
    Serial.println("R3 Off detected switch");
    turnOffRelays(RelayPin2);
  }
  else if (data.indexOf("Insts") != -1)
  {
    InstanceBegin();
  }
  else if (data.indexOf("Timer") != -1)
  {
    TimerBegins();
  }
  else if (data.indexOf("Initialization") != -1)
  {
    AwsCommand = true;
    Initialization();
  }
  else if (data.indexOf("Restart") != -1)
  {
    ESP.restart();
  }
}

bool isInternetConnected()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return false;
  }

  WiFiClient clientsq;
  bool connected = clientsq.connect("www.google.com", 80);
  clientsq.stop();  // Close the connection regardless of success or failure
  return connected; // Return true if connected, false otherwise
}

void connectAWS()
{

  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Connecting to AWS IoT");

  while (!client.connect(THINGNAME))
  {
    Serial.print("AWS IoT Connection failed: ");
    Serial.println(client.state());

    if (client.state() == -2)
    {
      Serial.println("Client error, retrying...");
    }

    delay(5000); // Wait before retrying
  }

  // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
  subscribeTopicInitialization();
}

void subscribetopics(String topics)
{
  if (!client.connected())
  {
    Serial.println("MQTT client not connected! Cannot subscribe to topic: " + topics);
    return;
  }

  bool result = client.subscribe(topics.c_str());
  if (!result)
  {
    Serial.println("Failed to subscribe to topic: " + topics);
  }
  else
  {
    Serial.println("Successfully subscribed to topic: " + topics);
  }
}

void subscribeTopicInitialization()
{
  if (User_Id.length() > 0 && Device_Uid.length() > 0)
  {

    // Device Control
    String baseDeviceUid = "";
    baseDeviceUid = User_Id + "/" + Device_Uid;
    subscribetopics(baseDeviceUid);

    // Instance Initialized
    String baseInstanceInitialized = "";
    baseInstanceInitialized = User_Id + "/" + "Instance";
    subscribetopics(baseInstanceInitialized);

    // Timer
    String baseDeviceTimer = "";
    baseDeviceTimer = User_Id + "/" + "Timer";
    subscribetopics(baseDeviceTimer);
  }
}

/*                                 AWS CODE END                       */

/*                         Timer Fetching and Storing Start             */
// Timer
void TimerBegins()
{
  String serverUrl = String(Domain_URL) + "/" + Device_timer;
  Serial.print("Server URL: ");
  Serial.println(serverUrl);

  // Create JSON document
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["Device_UID"] = Device_Uid;
  jsonDoc["User_id"] = User_Id;

  // Serialize JSON to string
  String requestBody;
  serializeJson(jsonDoc, requestBody);

  // Send POST request and get response
  String getTime = postMethods(serverUrl.c_str(), requestBody);
  Serial.print("Get User Id: ");
  Serial.println(getTime);

  // Deserialize response
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, getTime);

  if (error)
  {
    Serial.print(F("Deserialization failed: "));
    Serial.println(error.c_str());
    return;
  }

  Serial.println(F("Deserialization succeeded"));

  // Check the status field
  const char *statusResponse = doc["status"];
  if (statusResponse)
  {
    Serial.print("Response: ");
    Serial.println(statusResponse);
  }
  else
  {
    Serial.println("No status found in the response.");
    return;
  }

  // Check TimerInfo field
  if (!doc.containsKey("TimerInfo"))
  {
    Serial.println("TimerInfo key is missing!");
    return;
  }

  if (!LittleFS.begin())
  {
    Serial.println("Little Fs Mount Failed");
    return;
  }

  JsonArray timerArray = doc["TimerInfo"].as<JsonArray>();
  int count = 0;
  int basestorageeeprom = 162;
  if (!timerArray.isNull())
  {
    eepromClear();
    for (JsonObject timer : timerArray)
    {
      // Check for the required fields in the outer object
      if (timer.containsKey("Timer_id") && timer.containsKey("TimerInfo"))
      {

        JsonObject timerInfo = timer["TimerInfo"].as<JsonObject>();

        // Check for the required fields in the nested TimerInfo object
        if (timerInfo.containsKey("Switch_assigned") && timerInfo.containsKey("Start_time") && timerInfo.containsKey("End_time"))
        {
          count++;
          const char *switchAssigned = timerInfo["Switch_assigned"];
          const char *startTimes = timerInfo["Start_time"];
          const char *endTimes = timerInfo["End_time"];
          String startTime = startTimes;
          String endTime = endTimes;

          String relay_assign = assignRelay(switchAssigned);

          // Print the received data
          Serial.println(startTime);
          Serial.println(endTime);
          Serial.println(relay_assign);

          // Start time
          Serial.println(basestorageeeprom);
          int starttime_start = basestorageeeprom;
          int starttime_end = basestorageeeprom + startTime.length() + 1; // Add +1 for null termination
          basestorageeeprom = starttime_end;                              // Update address after writing start time
          int status_starttime = eepromWritter(starttime_start, starttime_end, startTime);
          Serial.print("Start Status: ");
          Serial.println(status_starttime);

          // End time
          Serial.println(basestorageeeprom);
          int status_end_start = basestorageeeprom;
          int status_end_end = basestorageeeprom + endTime.length() + 1; // Add +1 for null termination
          basestorageeeprom = status_end_end;                            // Update address after writing end time
          int status_endtime = eepromWritter(status_end_start, status_end_end, endTime);
          Serial.print("End Status: ");
          Serial.println(status_endtime);

          // Switch Assigned
          Serial.println(basestorageeeprom);
          int status_switch_assigned_start = basestorageeeprom;
          int status_switch_assigned_end = basestorageeeprom + relay_assign.length() + 1; // Add +1 for null termination
          basestorageeeprom = status_switch_assigned_end;                                 // Update address after writing switch assigned
          int status_switch_assigned = eepromWritter(status_switch_assigned_start, status_switch_assigned_end, relay_assign);
          Serial.print("Switch Status: ");
          Serial.println(status_switch_assigned);

          Serial.println(basestorageeeprom);
        }

        else
        {
          Serial.println("One or more fields are missing in the nested TimerInfo object!");
        }
      }
      else
      {
        Serial.println("One or more fields are missing in the Timer object!");
      }
    }
  }
  else
  {
    Serial.println("No timers found in the response.");
  }
  initEepromData();
}

/*                         Timer Fetching and Storing End                    */

void connect_wifi()
{
  String Wifi_SSIds = TimerStringChecker(15, 49);
  String Wifi_PASSORDIds = TimerStringChecker(50, 85);
  Serial.print("WIFI");
  Serial.println(Wifi_SSIds);
  Serial.print("Password");
  Serial.println(Wifi_PASSORDIds);
  Serial.print("WIFI");
  Serial.println(Wifi_SSIds.length());
  Serial.print("Password");
  Serial.println(Wifi_PASSORDIds.length());

  WiFi.begin(Wifi_SSIds, Wifi_PASSORDIds);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("O");
  }
  Serial.println("Connected to WiFi");
}

void PinModeDeclaration()
{
  pinMode(MainPin, OUTPUT);
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

/*                                 Git Update Start                                   */

void setClock()
{
  Serial.println("Synchronizing time...");
  configTime(5.5 * 3600, 0, "asia.pool.ntp.org");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    if (WiFi.status() != WL_CONNECTED)
    {
      connect_wifi(); // Reconnect if disconnected
    }
  }

  Serial.println("\nTime synchronized!");
}

void FirmwareUpdate()
{
  WiFiClientSecure clients;
  clients.setInsecure();
  client.disconnect();
  String payload = "";
  if (!clients.connect(host, httpsPort))
  {
    Serial.println("Connection failed");
    return;
  }
  clients.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: BuildFailureDetectorESP8266\r\n" +
                "Cache-Control: no-cache\r\n" +
                "Connection: close\r\n\r\n");
  while (true)
  {
    String line = clients.readStringUntil('\n');
    Serial.print("Line: ");
    Serial.println(line);
    if (line == "\r")
    {
      // Serial.println("Headers received");
      break;
    }
  }
  payload = clients.readStringUntil('\n');

  payload.trim();
  Serial.print("Payload: ");
  Serial.println(payload);
  Serial.print("Firmware installed: ");
  Serial.println(FirmwareVer);
  if (payload == FirmwareVer)
  {
    Serial.println("Device already on latest firmware version");
  }
  else
  {
    Serial.println("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    t_httpUpdate_return ret = ESPhttpUpdate.update(clients, URL_fw_Bin);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
  }
  clients.stop();
}

/*                                 Git Update End                                        */

String postMethods(const char *serverUrl, String SendData)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    return "Error: WiFi not connected";
  }
  client.disconnect();
  WiFiClientSecure wifiClient; // Use WiFiClientSecure for HTTPS
  wifiClient.setInsecure();    // Use only for testing, for production, use certificate validation

  HTTPClient http;

  // Debug: Show the server URL
  Serial.println("Connecting to URL: " + String(serverUrl));

  // Start the HTTP connection
  http.begin(wifiClient, serverUrl); // Ensure to pass the WiFiClientSecure instance for HTTPS
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000); // Set timeout to 10 seconds

  // Debug: Show the request body
  Serial.println("Request Body: " + SendData);

  // Send the POST request with the JSON data
  int httpResponseCode = http.POST(SendData);
  Serial.println("HTTP Response Code: " + String(httpResponseCode));

  String response;

  if (httpResponseCode > 0)
  {
    response = http.getString();
    Serial.println("Server response: " + response);
  }
  else
  {
    Serial.println("Error on sending POST: " + String(httpResponseCode));
    response = "Error: " + String(httpResponseCode);
  }

  wifiClient.stop();
  http.end();

  return response;
}

void eepromClear()
{
  for (unsigned int i = 161; i <= 251; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

int eepromWritter(int startLp, int EndLp, String Data)
{
  Serial.print("Clearing Data");
  Serial.print("data: ");
  Serial.println(Data);
  if (Data.length() > 0)
  {
    // for (int i = 0; i <= Data.length(); i++)
    for (unsigned int i = 0; i < Data.length(); i++)

    {
      EEPROM.write(i + startLp, Data[i]);
    }
    // Null-terminate the string in EEPROM
    EEPROM.write(startLp + Data.length(), '\0');

    EEPROM.commit();
    return 1; // Success
  }
  else
  {
    return -1; // Error: No data to write
  }
}

// Instance
void InstanceBegin()
{
  String serverUrl = String(Domain_URL) + "/" + Device_instance;
  // Create JSON document
  Serial.print("Server URL: ");
  Serial.println(serverUrl);
  StaticJsonDocument<200> jsonDoc;

  jsonDoc["Device_UID"] = Device_Uid;
  jsonDoc["User_Id"] = User_Id;

  // Serialize JSON to string
  String requestBody;
  serializeJson(jsonDoc, requestBody);

  // Send POST request and get response
  String getInstance = postMethods(serverUrl.c_str(), requestBody);
  Serial.print("Get User Id: ");
  Serial.println(getInstance);
}

// Registration
void Initialization()
{
  String UserUID = "";

  // Correct the loop condition to read from address 150 to 160
  for (int i = 150; i <= 160; i++)
  {
    char value = EEPROM.read(i);
    Serial.print("EEPROM:");
    Serial.println(value);
    if (value != 0 && value != '\0' && value != 0xFF && isPrintable(value))
    {
      UserUID += value; // Convert byte to char and append
    }
  }
  Serial.print("Value");
  Serial.println(UserUID.length());

  // Print the User UID stored in EEPROM
  Serial.print("User UID from EEPROM: ");
  Serial.println(UserUID);
  if (UserUID.length() <= 0 || AwsCommand == true)
  {
    AwsCommand = false;
    String serverUrl = String(Domain_URL) + "/" + Device_initializations; // Ensure https and trailing slash if required
    // Create JSON document
    Serial.print("Server URL: ");
    Serial.println(serverUrl);
    StaticJsonDocument<200> jsonDoc;

    jsonDoc["Device_UID"] = Device_Uid;
    jsonDoc["Status"] = "active";

    // Serialize JSON to string
    String requestBody;
    serializeJson(jsonDoc, requestBody);

    // Send POST request and get response
    String getUserid = postMethods(serverUrl.c_str(), requestBody);

    Serial.println("Server response: " + getUserid);

    // Create document to deserialize the server response
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, getUserid);

    if (error)
    {
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract the UserId from the response
    // "UserId" is a number, so we'll access it directly as an integer

    int user_Uid = doc["UserId"]; // This will extract the UserId as an integer

    if (user_Uid != 0)
    { // Check if UserId is valid (not zero)
      Serial.println("Device UID matched successfully");
      Serial.print("User UID: ");
      Serial.println(user_Uid);

      // Convert UserId to a String for EEPROM storage
      String userId_str = String(user_Uid);

      // Ensure we don't overwrite EEPROM beyond its size
      int len = userId_str.length();
      if (len > 11)
      {
        Serial.println("User ID is too long for EEPROM, truncating.");
        len = 11; // Limit the string to the first 11 characters to fit in EEPROM
      }

      // Write User ID to EEPROM starting from address 150
      for (int i = 0; i < len; i++)
      {
        EEPROM.write(i + 150, userId_str[i]);
      }
      EEPROM.commit();

      // Read back the User ID from EEPROM and print it
      String storedUserId = "";
      for (int i = 150; i < 150 + len; i++)
      {
        Serial.print("EEPROM : ");
        Serial.println((char)EEPROM.read(i));
        char Msg = EEPROM.read(i);
        if (Msg != 0)
        {
          storedUserId += Msg; // Read each byte and convert to char
        }
      }

      // Print the stored User ID
      Serial.println("Stored User ID: " + storedUserId);
      if (Test)
      {
        Serial.println("User uid:");
        User_Id = "7";
        Serial.println(User_Id);
      }
      else
      {
        Serial.println("User uid:");

        User_Id = storedUserId;
        Serial.println(User_Id);
      }
    }
    else
    {
      Serial.println("User ID not found in the response.");
    }
  }
  else
  {
    User_Id = UserUID;
  }
}

void loopContent()
{
  unsigned long currentMillis = millis();

  // Call setClock() and FirmwareUpdate() at specific intervals
  if ((currentMillis - previousMillis) >= firmwareUpdateInterval)
  {
    previousMillis = currentMillis;
    setClock();       // Update the time only once per hour
    FirmwareUpdate(); // Update firmware periodically
  }

  // Sync time every 10 minutes
  if (currentMillis - lastSync >= syncInterval)
  {
    lastSync = currentMillis;
    Serial.println("Syncing time with NTP server...");
    setClock(); // Reconnect to NTP server and sync time
  }

  // Non-blocking AWS IoT reconnection
  if (!client.connected() && (currentMillis - lastAWSReconnect >= awsReconnectInterval))
  {
    lastAWSReconnect = currentMillis;
    Serial.println("Checking weather to connect AWS IoT...");
    if (!client.connected())
    {
      connectAWS();
    }
  }
  if (currentMillis - lastSync >= TimeInterval)
  {
    lastSync = currentMillis;
    Serial.println("Time Interval Sync Started");
    TimerChecker();
  }
  client.loop();
}

String TimerStringChecker(int startLp, int EndLp)
{
  String datas = "";
  for (int i = startLp; i <= EndLp; i++)
  {
    char c = char(EEPROM.read(i));
    if (c == '\0')
      break; // Stop at null-terminator
    datas += c;
  }
  return datas;
}

/* TURN ON and OFF Relay Start */

int turnOnRelays(int RelayPinsfn)
{
  digitalWrite(RelayPinsfn, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);

  return 0;
}

int turnOffRelays(int RelayPinsfn)
{
  digitalWrite(RelayPinsfn, LOW);
  digitalWrite(LED_BUILTIN, LOW);

  return 0;
}

/* TURN ON and OFF Relay End */

void TimerChecker()
{

  //  TODO : Need to make Dynamic Code No need for slow rendering

  // Get Date from RTC
  DateTime now = rtc.now();

  String current_hours = String(now.hour());
  String current_minutes = String(now.minute());
  String current_seconds = String(now.second());

  Serial.print("Hours: ");
  Serial.println(current_hours);
  Serial.print("Minutes: ");
  Serial.println(current_minutes);
  Serial.print("Seconds: ");
  Serial.println(current_seconds);

  checkTimer(StartTiming, EndTiming, SwitchAssigned, current_hours, current_minutes, 1);
  checkTimer(StartTiming2, EndTiming2, SwitchAssigned2, current_hours, current_minutes, 2);
  checkTimer(StartTiming3, EndTiming3, SwitchAssigned3, current_hours, current_minutes, 3);
  checkTimer(StartTiming4, EndTiming4, SwitchAssigned4, current_hours, current_minutes, 4);
}

/*
void checkTimer(String StartTiming, String EndTiming, String SwitchAssigned, String current_hours, String current_minutes, int timerNumber)
{

  char substringcheck = ':';

  Serial.print("Start Timing: ");
  Serial.println(StartTiming);
  Serial.print("End Timing: ");
  Serial.println(EndTiming);
  Serial.print("Switch Assigned: ");
  Serial.println(SwitchAssigned);
  Serial.println("\n");

  if (StartTiming.length() > 0 && EndTiming.length() > 0 && SwitchAssigned.length() > 0)
  {
    int splitindexStart = StartTiming.indexOf(substringcheck);
    int splitindexEnd = EndTiming.indexOf(substringcheck);

    String startHours = StartTiming.substring(0, splitindexStart);
    String startMinutes = StartTiming.substring(splitindexStart + 1);

    String endHours = EndTiming.substring(0, splitindexEnd);
    String endMinutes = EndTiming.substring(splitindexEnd + 1);

    int SwitchName = assignRelayint(SwitchAssigned);

    if ((startHours == current_hours) && (startMinutes == current_minutes) &&
        (endHours == current_hours) && (endMinutes == current_minutes))
    {
      Serial.print("Timer ");
      Serial.print(timerNumber);
      Serial.println(": Start and End times match. Turning off relay.");
      turnOffRelays(SwitchName); // Prioritize turning off the relay
    }
    else if ((startHours == current_hours) && (startMinutes == current_minutes))
    {
      Serial.print("Timer ");
      Serial.print(timerNumber);
      Serial.println(": Time matched, turning on relay.");
      turnOnRelays(SwitchName);
    }
    else if ((endHours == current_hours) && (endMinutes == current_minutes))
    {
      Serial.print("Timer ");
      Serial.print(timerNumber);
      Serial.println(": Time matched, turning off relay.");
      turnOffRelays(SwitchName);
    }
    else
    {
      Serial.println("Else Fors ");
      initEepromData();
    }
  }
}
*/

void checkTimer(String StartTiming, String EndTiming, String SwitchAssigned, String current_hours, String current_minutes, int timerNumber)
{
  char substringcheck = ':';

  if (StartTiming.length() > 0 && EndTiming.length() > 0 && SwitchAssigned.length() > 0)
  {
    int splitindexStart = StartTiming.indexOf(substringcheck);
    int splitindexEnd = EndTiming.indexOf(substringcheck);

    int startHours = StartTiming.substring(0, splitindexStart).toInt();
    int startMinutes = StartTiming.substring(splitindexStart + 1).toInt();
    int endHours = EndTiming.substring(0, splitindexEnd).toInt();
    int endMinutes = EndTiming.substring(splitindexEnd + 1).toInt();

    int SwitchName = assignRelayint(SwitchAssigned);
    if (SwitchName == -1)
    {
      Serial.println("Error: Invalid switch assigned!");
      return;
    }

    Serial.print("Start Timing: ");
    Serial.println(StartTiming);
    Serial.print("End Timing: ");
    Serial.println(EndTiming);
    Serial.print("Switch Assigned: ");
    Serial.println(SwitchAssigned);
    Serial.print("Current Time: ");
    Serial.print(current_hours);
    Serial.print(":");
    Serial.println(current_minutes);

    if (current_hours.toInt() >= startHours && current_hours.toInt() <= endHours)
    {
      if ((current_hours.toInt() == startHours && current_minutes.toInt() >= startMinutes) ||
          (current_hours.toInt() == endHours && current_minutes.toInt() <= endMinutes) ||
          (current_hours.toInt() > startHours && current_hours.toInt() < endHours))
      {
        Serial.print("Timer ");
        Serial.print(timerNumber);
        Serial.println(": Time matched, turning on relay.");
        turnOnRelays(SwitchName);
        return;
      }
    }

    Serial.print("Timer ");
    Serial.print(timerNumber);
    Serial.println(": Outside timing range. Turning off relay.");
    turnOffRelays(SwitchName);
  }
  else
  {
    Serial.println("Error: Invalid timing or switch configuration.");
  }
}

void InitializeRTC()
{
  // Start I2C communication
  Wire.begin();

  // Initialize the RTC
  if (rtc.begin())
  {
    if (rtc.lostPower())
    {
      rtc.adjust(DateTime(F(__DATE__), (F(__TIME__))));
    }
    Serial.println("RTC is RUNNING");

    // Get the current date and time from the RTC
    DateTime now = rtc.now();

    // Print the current date
    Serial.print("Current Date: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.println(now.day(), DEC);

    // Optionally, print the current time as well
    Serial.print("Current Time: ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.println(now.second(), DEC);
  }
  else
  {
    Serial.println("RTC is NOT RUNNING");
  }
}

void clientCheckAwsConnected()
{
  if (!client.connected())
  {
    Serial.println("AWS IoT connection lost. Attempting to reconnect...");
    connectAWS();
  }
  else
  {
    Serial.println("AWS IoT is connected.");
  }
}

void deviceInitialization()
{
  Device_Uid = TimerStringChecker(0, 12);
  Serial.print("Device UID: ");
  Serial.println(Device_Uid);
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);
  Serial.println("");
  PinModeDeclaration();
  deviceInitialization();
  InitializeRTC();
  // rtc.adjust(DateTime(2024, 12, 7, 18, 25, 0));
  Serial.println("Start");
  WiFi.mode(WIFI_STA);
  connect_wifi();
  setClock();
  Initialization();
  FirmwareUpdate();
  TimerBegins();

  String datas = "";
  for (int i = 0; i <= 512; i++)
  {
    char c = char(EEPROM.read(i));
    Serial.println("i:" + String(i) + "+" + String(c));
  }

  initEepromData();
  TimerChecker();
  clientCheckAwsConnected();
}

void loop()
{
  loopContent();
}
