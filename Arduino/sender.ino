#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>
#include "FS.h"
#include <ArduinoJson.h>

typedef struct curtain {
  int storedHourOpen;
  int storedMinuteOpen;
  int storedHourClose;
  int storedMinuteClose;
  bool repeatEveryDay = false;

  // toogle values we get from user (webpage)
  bool tempToggleValueCurtain = false;
  bool sunToggleValueCurtain = false;
  bool timeSet = false;
  int storedDay;
  int storedMonth;
  int storedYear;
} curtain;

curtain curtain1;
curtain curtain2;

int eepromAddress;
float curtain1Temp, curtain2Temp;
bool curtain1Status, curtain2Status;
// Wifi connections
const char* ssid = "ALFA";
const char* password = "167349abc";

ESP8266WebServer server(80);

// NTP server retrieve the time.
const char* ntpServer = "pool.ntp.org";
const int timeZone = 3;
WiFiUDP ntpUDP; //  connect to an NTP server and retrieve the current time.
NTPClient timeClient(ntpUDP, ntpServer, timeZone);

// open and close time informations that we are going to store in eeprom
unsigned long timerOpenHourMinute = 0;
unsigned long timerCloseHourMinute = 0;
unsigned long timerOpenHourMinute2 = 0;
unsigned long timerCloseHourMinute2 = 0;
int temperature;

IPAddress staticIP(192, 168, 0, 201);  // Replace with the desired static IP address
IPAddress gateway(192, 168, 0, 1);     // Replace with your network gateway IP address
IPAddress subnet(255, 255, 255, 0);    // Replace with your network subnet mask
unsigned long previousMillis = 0;  // Store the previous time for getting datas from sender
const unsigned long interval = 10000;  // Interval in milliseconds (10 seconds)
void handleData1() {

  StaticJsonDocument<512> jsonDoc;
  
  jsonDoc["storedHourOpen"] = curtain1.storedHourOpen;
  jsonDoc["storedMinuteOpen"] = curtain1.storedMinuteOpen;
  jsonDoc["storedHourClose"] = curtain1.storedHourClose;
  jsonDoc["storedMinuteClose"] = curtain1.storedMinuteClose;
  jsonDoc["repeatEveryDay"] = curtain1.repeatEveryDay;
  jsonDoc["tempToggleValueCurtain"] = curtain1.tempToggleValueCurtain;
  jsonDoc["sunToggleValueCurtain"] = curtain1.sunToggleValueCurtain;
  jsonDoc["timeSet"] = curtain1.timeSet;
  jsonDoc["storedDay"] = curtain1.storedDay;
  jsonDoc["storedMonth"] = curtain1.storedMonth;
  jsonDoc["storedYear"] = curtain1.storedYear;

  char jsonData[512];
  serializeJson(jsonDoc, jsonData);

  server.send(201, "application/json", jsonData);
}

void handleData2() {

  StaticJsonDocument<512> jsonDoc;
  jsonDoc["storedHourOpen"] = curtain2.storedHourOpen;
  jsonDoc["storedMinuteOpen"] = curtain2.storedMinuteOpen;
  jsonDoc["storedHourClose"] = curtain2.storedHourClose;
  jsonDoc["storedMinuteClose"] = curtain2.storedMinuteClose;
  jsonDoc["repeatEveryDay"] = curtain2.repeatEveryDay;
  jsonDoc["tempToggleValueCurtain"] = curtain2.tempToggleValueCurtain;
  jsonDoc["sunToggleValueCurtain"] = curtain2.sunToggleValueCurtain;
  jsonDoc["timeSet"] = curtain2.timeSet;
  jsonDoc["storedDay"] = curtain2.storedDay;
  jsonDoc["storedMonth"] = curtain2.storedMonth;
  jsonDoc["storedYear"] = curtain2.storedYear;

  char jsonData[512];
  serializeJson(jsonDoc, jsonData);

  server.send(300, "application/json", jsonData);

}

void setup() {

  SPIFFS.begin();
  Serial.begin(9600);

  //WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  server.on("/", handleRoot);
  server.on("/set-timer1", handleTimer1);
  server.on("/set-delete1", handleDelete1);
  server.on("/handleTempToggleValueCurtain1", handleTempToggleValueCurtain1);
  server.on("/handleSunToggleValueCurtain1", handleSunToggleValueCurtain1);
  server.on("/set-timer2", handleTimer2);
  server.on("/set-delete2", handleDelete2);
  server.on("/handleTempToggleValueCurtain2", handleTempToggleValueCurtain2);
  server.on("/handleSunToggleValueCurtain2", handleSunToggleValueCurtain2);
  server.on("/data", handleData1);
  server.on("/data2", handleData2);
  server.on("/retrieveToggleStates", handleRetrieveToggleStates);
  server.on("/temperature_endpoint", handleTempAndStatus1);
  server.on("/temperature_endpoint2", handleTempAndStatus2);
  server.begin();

  EEPROM.begin(50);  // Initialize EEPROM with the required size
  Wire.begin();
  oled.init(); // Initialze SSD1306 OLED display
  oled.clearDisplay(); // Clear SSD1306 OLED display

}

void loop() {
  unsigned long currentMillis = millis();  // Get the current time
  // TIME FUNCTIONS //
  if (currentMillis - previousMillis >= interval) {
    // Update the previous execution time
    previousMillis = currentMillis;  // Update the previous time
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    epochTime += 3 * 3600;
    setTime(epochTime);
  }
  timeClient.update();
  // Read temperature from LM335AZ sensor
  int sensorValue = analogRead(A0);
  // Convert analog value to Celsius temperature
  temperature = sensorValue * 330 / 1024 - 273 + 5;
  
  oled.setTextXY(0, 0);
  oled.putString("Date: ");
  putNumberWithLeadingZero(day());
  oled.putString("/");
  putNumberWithLeadingZero(month());
  oled.putString("/");
  oled.putNumber(year());
  oled.setTextXY(2, 0);
  oled.putString("Time: ");
  putNumberWithLeadingZero(hour());
  oled.putString(":");
  putNumberWithLeadingZero(minute());
  oled.putString(":");
  putNumberWithLeadingZero(second());
  oled.setTextXY(4, 0);
  oled.putString("IP:");
  String ip = WiFi.localIP().toString();
  oled.putString(ip.c_str());
  oled.setTextXY(6, 0);
  oled.putString("Temp: ");
  putNumberWithLeadingZero(temperature);

  // Get information of Curtain 1 from eeprom
  EEPROM.get(0, timerOpenHourMinute);
  EEPROM.get(4, timerCloseHourMinute);

  curtain1.storedHourOpen = (timerOpenHourMinute >> 8) & 0xFF;
  curtain1.storedMinuteOpen = timerOpenHourMinute & 0xFF;

  curtain1.storedHourClose = (timerCloseHourMinute >> 8) & 0xFF;
  curtain1.storedMinuteClose = timerCloseHourMinute & 0xFF;

  EEPROM.get(8, curtain1.repeatEveryDay);
  EEPROM.get(9, curtain1.timeSet);
  EEPROM.get(10, curtain1.storedDay);
  EEPROM.get(14, curtain1.storedMonth);
  EEPROM.get(18, curtain1.storedYear);
  EEPROM.get(22, curtain1.tempToggleValueCurtain);
  EEPROM.get(23, curtain1.sunToggleValueCurtain);


  // Get information of Curtain 2 from eeprom

  EEPROM.get(24, timerOpenHourMinute2);
  EEPROM.get(28, timerCloseHourMinute2);

  curtain2.storedHourOpen = (timerOpenHourMinute2 >> 8) & 0xFF;
  curtain2.storedMinuteOpen = timerOpenHourMinute2 & 0xFF;

  curtain2.storedHourClose = (timerCloseHourMinute2 >> 8) & 0xFF;
  curtain2.storedMinuteClose = timerCloseHourMinute2 & 0xFF;

  EEPROM.get(32, curtain2.repeatEveryDay);
  EEPROM.get(33, curtain2.timeSet);
  EEPROM.get(34, curtain2.storedDay);
  EEPROM.get(38, curtain2.storedMonth);
  EEPROM.get(42, curtain2.storedYear);
  EEPROM.get(46, curtain2.tempToggleValueCurtain);
  EEPROM.get(47, curtain2.sunToggleValueCurtain);


  server.handleClient();

}

//read the html file
void handleRoot() {
  String htmlPage = readFile("/index.txt");
  server.send(200, "text/html", htmlPage);
}


String readFile(const char* path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return String();
  }

  String content;
  while (file.available()) {
    content += (char)file.read();
  }

  file.close();
  return content;
}

// CURTAIN 1! //
// Get the time values
void handleTimer1() {

  if (server.hasArg("timer1open") || server.hasArg("timer1close")) {
    eepromAddress = 0;
    timeClient.update();
    server.handleClient();

    unsigned long epochTime = timeClient.getEpochTime();
    epochTime += 3 * 3600;
    setTime(epochTime);
    curtain1.timeSet = true;
    curtain1.sunToggleValueCurtain = false;

    String repeat = server.arg("repeatDaily");
    if (repeat == "true") {
      curtain1.repeatEveryDay = true;
    } else {
      curtain1.repeatEveryDay = false;
    }

    String timerOpenValue = server.arg("timer1open");
    String timerCloseValue = server.arg("timer1close");
    Serial.println("Timer open set to: " + timerOpenValue);
    Serial.println("Timer close set to: " + timerCloseValue);
    
    int timerOpenHour = timerOpenValue.substring(0, 2).toInt();
    int timerOpenMinute = timerOpenValue.substring(3, 5).toInt();
    int timerCloseHour = timerCloseValue.substring(0, 2).toInt();
    int timerCloseMinute = timerCloseValue.substring(3, 5).toInt();

    timerOpenHourMinute = (timerOpenHour << 8) | timerOpenMinute;
    timerCloseHourMinute = (timerCloseHour << 8) | timerCloseMinute;
    int storedDay = day();
    int storedMonth = month();
    int storedYear = year();

    EEPROM.put(eepromAddress, timerOpenHourMinute);
    eepromAddress += sizeof(timerOpenHourMinute);
    EEPROM.put(eepromAddress, timerCloseHourMinute);
    eepromAddress += sizeof(timerCloseHourMinute);
    EEPROM.put(eepromAddress, curtain1.repeatEveryDay);
    eepromAddress += sizeof(curtain1.repeatEveryDay);
    EEPROM.put(eepromAddress, curtain1.timeSet);
    eepromAddress += sizeof(curtain1.timeSet);
    EEPROM.put(eepromAddress, storedDay);
    eepromAddress += sizeof(storedDay);
    EEPROM.put(eepromAddress, storedMonth);
    eepromAddress += sizeof(storedMonth);
    EEPROM.put(eepromAddress, storedYear);
    eepromAddress += sizeof(storedYear);
    EEPROM.commit();

  } else {
    server.send(400, "text/html", "Bad Request");
  }
}
void handleDelete1() {
  curtain1.timeSet = false;
  curtain1.repeatEveryDay = false;
  EEPROM.put(8, curtain1.repeatEveryDay);
  EEPROM.put(9, curtain1.timeSet);
}
// Get the status of the temperature value of the first curtain
void handleTempToggleValueCurtain1() {
  eepromAddress = 22;
  oled.clearDisplay();
  if (server.hasArg("value")) {
    String value = server.arg("value");
    Serial.println("Received value: " + value);
    if (value == "true") {
      curtain1.tempToggleValueCurtain = true;
    } else {
      curtain1.tempToggleValueCurtain = false;
    }
    EEPROM.put(eepromAddress, curtain1.tempToggleValueCurtain);
    EEPROM.commit();
    server.send(200, "text/html", "Temp toggle value set");
  }
}

// Get the status of the sun value of the first curtain
void handleSunToggleValueCurtain1() {
  curtain1.timeSet = false;
  curtain1.sunToggleValueCurtain = true;
  eepromAddress = 23;
  //oled.clearDisplay();
  if (server.hasArg("value")) {
    String value = server.arg("value");
    Serial.println("Received value: " + value);
    if (value == "true") {
      curtain1.sunToggleValueCurtain = true;
    } else {
      curtain1.sunToggleValueCurtain = false;
    }
    EEPROM.put(eepromAddress, curtain1.sunToggleValueCurtain);
    EEPROM.commit();
    server.send(200, "text/html", "Sun toggle value set");
  }
}

// CURTAIN 2! //
// Get the time values
void handleTimer2() {
  if (server.hasArg("timer2open") || server.hasArg("timer2close")) {
    curtain2.timeSet = true;
    curtain2.sunToggleValueCurtain = false;
    eepromAddress = 24;
    timeClient.update();
    server.handleClient();
    unsigned long epochTime = timeClient.getEpochTime();
    epochTime += 3 * 3600;
    setTime(epochTime);
    String repeat = server.arg("repeatDaily2");
    if (repeat == "on") {
      curtain2.repeatEveryDay = true;
    } else {
      curtain2.repeatEveryDay = false;
    }
    String timerOpenValue = server.arg("timer2open");
    String timerCloseValue = server.arg("timer2close");
    Serial.println("Timer open set to: " + timerOpenValue);
    Serial.println("Timer close set to: " + timerCloseValue);
    //server.send(200, "text/html", "Timer open set to: " + timerOpenValue + "<br>Timer close set to: " + timerCloseValue);

    int timerOpenHour = timerOpenValue.substring(0, 2).toInt();
    int timerOpenMinute = timerOpenValue.substring(3, 5).toInt();
    int timerCloseHour = timerCloseValue.substring(0, 2).toInt();
    int timerCloseMinute = timerCloseValue.substring(3, 5).toInt();

    timerOpenHourMinute2 = (timerOpenHour << 8) | timerOpenMinute;
    timerCloseHourMinute2 = (timerCloseHour << 8) | timerCloseMinute;

    int storedDay = day();
    int storedMonth = month();
    int storedYear = year();

    EEPROM.put(eepromAddress, timerOpenHourMinute2);
    eepromAddress += sizeof(timerOpenHourMinute2);
    EEPROM.put(eepromAddress, timerCloseHourMinute2);
    eepromAddress += sizeof(timerCloseHourMinute2);
    EEPROM.put(eepromAddress, curtain2.repeatEveryDay);
    eepromAddress += sizeof(curtain2.repeatEveryDay);
    EEPROM.put(eepromAddress, curtain2.timeSet);
    eepromAddress += sizeof(curtain1.timeSet);
    EEPROM.put(eepromAddress, storedDay);
    eepromAddress += sizeof(storedDay);
    EEPROM.put(eepromAddress, storedMonth);
    eepromAddress += sizeof(storedMonth);
    EEPROM.put(eepromAddress, storedYear);
    eepromAddress += sizeof(storedYear);
    EEPROM.commit();
  } else {
    server.send(400, "text/html", "Bad Request");
  }
}
void handleDelete2() {
  curtain2.timeSet = false;
  curtain2.repeatEveryDay = false;
  EEPROM.put(32, curtain2.repeatEveryDay);
  EEPROM.put(33, curtain2.timeSet);
}
// Get the status of the temperature value of the first curtain
void handleTempToggleValueCurtain2() {
  eepromAddress = 46;
  oled.clearDisplay();
  if (server.hasArg("value")) {
    String value = server.arg("value");
    Serial.println("Received value: " + value);
    if (value == "true") {
      curtain2.tempToggleValueCurtain = true;
    } else {
      curtain2.tempToggleValueCurtain = false;
    }
    EEPROM.put(eepromAddress, curtain2.tempToggleValueCurtain);
    EEPROM.commit();
    server.send(200, "text/html", "Temp toggle value set");
  }
}
// Get the status of the sun value of the first curtain
void handleSunToggleValueCurtain2() {
  curtain2.timeSet = false;
  curtain2.sunToggleValueCurtain = true;
  eepromAddress = 47;
  //oled.clearDisplay();
  if (server.hasArg("value")) {
    String value = server.arg("value");
    Serial.println("Received value: " + value);
    if (value == "true") {
      curtain2.sunToggleValueCurtain = true;
    } else {
      curtain2.sunToggleValueCurtain = false;
    }
    EEPROM.put(eepromAddress, curtain2.sunToggleValueCurtain);
    EEPROM.commit();
    server.send(200, "text/html", "Sun toggle value set");
  }
}

void putNumberWithLeadingZero(uint8_t number) {
  if (number < 10) {
    oled.putString("0");
  }
  oled.putNumber(number);
}
void handleRetrieveToggleStates() {
  String response = "[";
  response += "{\"id\":\"" + String(1) + "\",\"temp\":" + String(curtain1.tempToggleValueCurtain) + ",\"sun\":" + String(curtain1.sunToggleValueCurtain) + ",\"currentTemp\":" + String(temperature) + "},";
  response += "{\"id\":\"" + String(2) + "\",\"temp\":" + String(curtain2.tempToggleValueCurtain) + ",\"sun\":" + String(curtain2.sunToggleValueCurtain) + ",\"currentTemp\":" + String(temperature) + "}";
  response += "]";
  server.send(200, "application/json", response);
}
void handleTempAndStatus1() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, server.arg("plain"));

  if (error) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  curtain1Temp = json["temperature"];
  curtain1Status = json["curtainStatus"];

  // Do something with the received temperature and curtain status values
  Serial.print("Received temperature: ");
  Serial.println(curtain1Temp);
  Serial.print("Received curtain status: ");
  Serial.println(curtain1Status);


  server.send(200, "text/plain", "OK");
}
void handleTempAndStatus2() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, server.arg("plain"));

  if (error) {
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  curtain2Temp = json["temperature"];
  curtain2Status = json["curtainStatus"];

  // Do something with the received temperature and curtain status values
  Serial.print("Received temperature from 2: ");
  Serial.println(curtain2Temp);
  Serial.print("Received curtain status: ");
  Serial.println(curtain2Status);


  server.send(300, "text/plain", "OK");
}