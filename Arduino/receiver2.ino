#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ezButton.h>
#include <EEPROM.h>

// Wi-Fi credentials
const char* ssid = "ALFA";
const char* password = "167349abc";
ESP8266WebServer server(80);
// Server IP address and endpoint
const char* serverIP = "192.168.0.106";  // Replace with the server IP address
const int serverPort = 80;
const char* endpoint = "/data2";
typedef struct curtain {
  // time
  int storedHourOpen;
  int storedMinuteOpen;
  int storedHourClose;
  int storedMinuteClose;
  bool repeatEveryDay = false;
  bool timeSet = false;

  // toogle values we get from user (webpage)
  bool tempToggleValueCurtain = false;
  bool sunToggleValueCurtain = false;

  int storedDay;
  int storedMonth;
  int storedYear;
} curtain;

curtain curtainData;
bool curtainStatus;
// Callback function that will be executed when data is received
void handleData(String jsonData) {
  // Parse the JSON data
  DynamicJsonDocument jsonDoc(512);
  deserializeJson(jsonDoc, jsonData);
  // Extract values from the JSON object

  curtainData.storedHourOpen = jsonDoc["storedHourOpen"];
  curtainData.storedMinuteOpen = jsonDoc["storedMinuteOpen"];
  curtainData.storedHourClose = jsonDoc["storedHourClose"];
  curtainData.storedMinuteClose = jsonDoc["storedMinuteClose"];
  curtainData.repeatEveryDay = jsonDoc["repeatEveryDay"];
  curtainData.tempToggleValueCurtain = jsonDoc["tempToggleValueCurtain"];
  curtainData.sunToggleValueCurtain = jsonDoc["sunToggleValueCurtain"];
  curtainData.timeSet = jsonDoc["timeSet"];
  curtainData.storedDay = jsonDoc["storedDay"];
  curtainData.storedMonth = jsonDoc["storedMonth"];
  curtainData.storedYear = jsonDoc["storedYear"];

}

const int analogInputPin = A0;
// 4021 control pins
const int controlPinA = D3;
const int controlPinB = D4;
const int controlPinC = D7;

// Motor control pins
int in1 = D5;
int in2 = D6;
int enA = D8;
const char* ntpServer = "pool.ntp.org";
const int timeZone = 3;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeZone);
bool isSummer;
float curtainOutTemp;
float curtainInTemp;
float curtainLdr;
String motorDirection = "None";

unsigned long previousMillis = 0;  // Store the previous time for getting datas from sender
const unsigned long interval = 10000;  // Interval in milliseconds (10 seconds)
unsigned long previousExecution = 0; // Variable to store the previous execution time for getting epochtime
const unsigned long intervalTime = 24 * 60 * 60 * 1000; // 24 hours in milliseconds
unsigned long previousMillisSend = 0;  // Store the previous time for getting datas from sender
const unsigned long intervalSend = 13000;  // Interval in milliseconds (13 seconds)
unsigned long previousMillisSensor = 0;  // Store the previous time for getting datas from sender
const unsigned long intervalSensor = 5000;  // Interval in milliseconds (5 seconds)
ezButton limitSwitchUp(4), limitSwitchDown(5);
void setup() {
  // Start Serial monitor
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  // Make an HTTP GET request to the server
  limitSwitchUp.setDebounceTime(50); // set debounce time to 50 milliseconds
  limitSwitchDown.setDebounceTime(50);

  timeClient.begin();

  server.on("/open-curtain2", openCurtain);
  server.on("/close-curtain2", closeCurtain);
  server.begin();
  // Set control pins as outputs
  pinMode(controlPinA, OUTPUT);
  pinMode(controlPinB, OUTPUT);
  pinMode(controlPinC, OUTPUT);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enA, OUTPUT);


  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  epochTime += timeZone * 3600;
  setTime(epochTime);
  // check suumer or winter.
  if (month() >= 4 && month() < 10) {
    isSummer = true;
  } else {
    isSummer = false;
  }
  EEPROM.begin(50);
}

void loop() {

  limitSwitchDown.loop();
  limitSwitchUp.loop();
  if (limitSwitchUp.isPressed() && motorDirection == "left") {
    stopMotor();
    EEPROM.put(0, 1);
  }
  if (limitSwitchDown.isPressed() && motorDirection == "right") {
    stopMotor();
    EEPROM.put(0, 0);
  }
  EEPROM.get(0, curtainStatus);
  server.handleClient();
  unsigned long currentMillis = millis();  // Get the current time
  // Check if 24 hours have elapsed since the previous execution
  if (currentMillis - previousExecution >= intervalTime) {
    // Update the previous execution time
    previousExecution = currentMillis;

    unsigned long epochTime = timeClient.getEpochTime();
    epochTime += 3 * 3600;
    setTime(epochTime);
  }
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update the previous time
    getCurtainDatas();
  }
  if (currentMillis - previousMillisSend >= intervalSend) {
    previousMillisSend = currentMillis;  // Update the previous time
    sendTempAndStatus();
  }
  if (currentMillis - previousMillisSensor >= intervalSensor) {
    previousMillisSensor = currentMillis;  // Update the previous time
    updateSensorValues();
  }
  if (curtainData.timeSet) {
    if ( curtainData.repeatEveryDay || (curtainData.storedYear == year() && curtainData.storedMonth == month() && curtainData.storedDay == day())) {
      if (hour() == curtainData.storedHourOpen && minute() == curtainData.storedMinuteOpen && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (hour() == curtainData.storedHourClose && minute() == curtainData.storedMinuteClose && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    }
    if (isSummer && curtainData.tempToggleValueCurtain) {
      if (curtainOutTemp + 3.0 < curtainInTemp && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (curtainOutTemp > curtainInTemp + 3.0 && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    } else if (!isSummer && curtainData.tempToggleValueCurtain) {
      if (curtainOutTemp + 3.0 > curtainInTemp && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (curtainOutTemp < curtainInTemp + 3.0 && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    }
  }
  //
  if (curtainData.sunToggleValueCurtain) {
    if (curtainLdr < 0.10 && motorDirection != "right") {
      motorDirection = "right";
      startMotorRight();
    } else if (curtainLdr > 0.40 && motorDirection != "left") {
      motorDirection = "left";
      startMotorLeft();
    }
    if (isSummer && curtainData.tempToggleValueCurtain) {
      if (curtainOutTemp + 3.0 < curtainInTemp && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (curtainOutTemp > curtainInTemp + 3.0 && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    } else if (!isSummer && curtainData.tempToggleValueCurtain) {
      if (curtainOutTemp + 3.0 > curtainInTemp && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (curtainOutTemp < curtainInTemp + 3.0 && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    }

  }
  //
  if (!curtainData.timeSet && !curtainData.sunToggleValueCurtain && curtainData.tempToggleValueCurtain) {
    if (isSummer) {
      if (curtainOutTemp + 3.0 < curtainInTemp && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (curtainOutTemp > curtainInTemp + 3.0 && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    } else if (!isSummer) {
      if (curtainOutTemp + 3.0 > curtainInTemp && motorDirection != "left") {
        motorDirection = "left";
        startMotorLeft();
      } else if (curtainOutTemp < curtainInTemp + 3.0 && motorDirection != "right") {
        motorDirection = "right";
        startMotorRight();
      }
    }
  }


  /*Serial.print("stored hour Open: ");
    Serial.println(curtainData.storedHourOpen);
    Serial.print("stored Minute Open: ");
    Serial.println(curtainData.storedMinuteOpen);
    Serial.println();
    Serial.print("stored hour Close: ");
    Serial.println(curtainData.storedHourClose);
    Serial.print("stored Minute Close: ");
    Serial.println(curtainData.storedMinuteClose);
    Serial.println();
    Serial.println(motorDirection);
    Serial.println(curtainData.storedMinuteOpen);
    curtainData.repeatEveryDay ? Serial.println("repeat every day: yes") : Serial.println("repeat every day: no");
    curtainData.tempToggleValueCurtain ? Serial.println("temp: yes") : Serial.println("temp: no");
    curtainData.sunToggleValueCurtain ? Serial.println("sun: yes") : Serial.println("sun: no");
    delay(2000);*/

}

void startMotorLeft() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(enA, 90);
}
void startMotorRight() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  analogWrite(enA, 50);
}

void stopMotor() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enA, 0);
}

// sensor values connected to 4051
void updateSensorValues() {
  for (int channel = 0; channel < 3; channel++) {
    // Set the control pins to select the appropriate sensor channel
    selectSensorChannel(channel);
    if (channel == 0) {
      // Read the analog input value from the inside temp
      int multiplexerValue = analogRead(analogInputPin);
      curtainInTemp = multiplexerValue * 330 / 1024 - 273;
      delay(50);
    } else if (channel == 1) {
      // Read the analog input value from the outside temp
      int multiplexerValue = analogRead(analogInputPin);
      curtainOutTemp = multiplexerValue * 330 / 1024 - 273;
      delay(50);
    } else if (channel == 2) {
      int sensorValue = analogRead(analogInputPin);   // read the input of ldr
      curtainLdr = sensorValue * (3.3 / 1023.0);
    } else {
      //Serial.println("");
    }
  }
}

void selectSensorChannel(int channel) {
  // Convert channel number to binary representation
  int pinAState = bitRead(channel, 0);
  int pinBState = bitRead(channel, 1);
  int pinCState = bitRead(channel, 2);

  // Set the control pins to select the specified channel
  digitalWrite(controlPinA, pinAState);
  digitalWrite(controlPinB, pinBState);
  digitalWrite(controlPinC, pinCState);
}
void openCurtain() {
  motorDirection = "left";
  startMotorLeft();
}
void closeCurtain() {
  motorDirection = "right";
  startMotorRight();
}
void getCurtainDatas() {
  WiFiClient client;
  HTTPClient http;

  // Concatenate the URL
  String url = "http://";
  url += serverIP;
  url += ":";
  url += serverPort;
  url += endpoint;

  http.begin(client, url);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 300) {
    String response = http.getString();
    handleData(response);
  } else {
    Serial.print("Error - HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // End the request
  http.end();
}
void sendTempAndStatus() {
  // Create an HTTPClient object
  WiFiClient client;
  HTTPClient http;

  // Concatenate the URL
  String url = "http://192.168.0.106/temperature_endpoint";

  // Set the content-type header to JSON
  http.addHeader("Content-Type", "application/json");

  // Create the JSON payload with the temperature and curtain status values
  String payload = "{\"temperature\": " + String(curtainOutTemp) + ", \"curtainStatus\": " + String(curtainStatus) + "}";

  // Begin the request
  http.begin(client, url);

  // Send the POST request with the payload
  int httpResponseCode = http.sendRequest("POST", payload);

  // Check the response code
  if (httpResponseCode == 300) {
    //Serial.println("Temperature and curtain status sent successfully");
  } else {
    Serial.print("Error - HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // End the request
  http.end();
}