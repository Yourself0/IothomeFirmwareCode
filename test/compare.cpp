#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPI.h>

#include "Secrets.h"

int const MainPin = D7;    //  near board
int const RelayPin1 = D6;  //  middle board
int const RelayPin2 = D5;  //  last board

unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;

#define AWS_IOT_PUBLISH_TOPIC "Data/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "Data/sub"

WiFiClientSecure net;

BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

PubSubClient client(net);

bool timeInitialized = false;  // Flag to track if time has been initialized

void PinModeDeclaration() {
  pinMode(MainPin, OUTPUT);
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

// Function to connect to NTP and get the time
void NTPConnect(void) {
  Serial.print("Setting time using SNTP");

  // Set the timezone for IST (UTC+5:30)
  configTime(19800, 0, "ntp.ubuntu.com", "time.windows.com");  // Use reliable NTP servers

  time_t now = time(nullptr);

  // Wait until time is properly set
  while (now < 1510592825)  // 1510592825 is an arbitrary "past" timestamp
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("done!");

  // Get the current time
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);  // Use localtime_r for local time

  // Print the current time
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));  // Print as a human-readable string
}

void messageEvent(String data) {
  Serial.println(data);

  if (data.indexOf("R1_On") != -1) {
    Serial.println("R1 On detected switch");
    digitalWrite(MainPin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (data.indexOf("R1_Off") != -1) {
    Serial.println("R1 Off detected switch");
    digitalWrite(MainPin, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  } else if (data.indexOf("R2_On") != -1) {
    Serial.println("R2 On detected switch");
    digitalWrite(RelayPin1, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (data.indexOf("R2_Off") != -1) {
    Serial.println("R2 Off detected switch");
    digitalWrite(RelayPin1, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  } else if (data.indexOf("R3_On") != -1) {
    Serial.println("R3 On detected switch");
    digitalWrite(RelayPin2, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (data.indexOf("R3_Off") != -1) {
    Serial.println("R3 Off detected switch");
    digitalWrite(RelayPin2, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  messageEvent(message);
  Serial.println();
}

void connectAWS() {
  if (!timeInitialized) {
    NTPConnect();  // Set time only once
    timeInitialized = true;
  }

  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Connecting to AWS IoT");

  while (!client.connect(THINGNAME)) {
    Serial.print("AWS IoT Connection failed: ");
    Serial.println(client.state());

    if (client.state() == -2) {
      Serial.println("Client error, retrying...");
    }

    delay(5000);  // Wait before retrying
  }

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("WiFi Connected");
  PinModeDeclaration();
  digitalWrite(MainPin, LOW);
  digitalWrite(RelayPin1, LOW);
  digitalWrite(RelayPin2, LOW);
  // int const MainPin = D7;    //  near board
  // int const RelayPin1 = D6;  //  middle board
  // int const RelayPin2 = D5; 

  // Initialize AWS IoT connection
  connectAWS();
}




unsigned long lastSync = 0;
const long syncInterval = 300000;  // Sync with NTP server every 5 minutes (300000 ms)

void loop() {
  // Get the current time from NTP server periodically (every 5 minutes)
  if (millis() - lastSync > syncInterval) {
    lastSync = millis();
    Serial.println("Syncing time with NTP server...");
    NTPConnect();  // Reconnect to NTP server and sync time
  }

  // Fetch the current time
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  Serial.print("Hour: ");
  Serial.println(timeinfo.tm_hour);
  Serial.print("Minute: ");
  Serial.println(timeinfo.tm_min);

  // Example condition to turn on/off the LED based on the time
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  if (currentHour == 18 && currentMinute >= 10) {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn on the LED
    digitalWrite(RelayPin1, HIGH);
    Serial.println("Device ON");
  }

  if (currentHour == 22 && currentMinute >= 30) {
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
    digitalWrite(RelayPin1, LOW);
    Serial.println("Device OFF");
  }

  // Reconnect to AWS IoT if needed
  if (!client.connected()) {
    Serial.println("Reconnecting to AWS IoT...");
    connectAWS();
  } else {
    client.loop();
  }

  // Handle periodic publishing (if applicable)
  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    // publishMessage();  // Add back if you want to send data periodically
  }
}
