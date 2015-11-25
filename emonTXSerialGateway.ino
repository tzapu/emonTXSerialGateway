#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


#include "config.h"
//contains
//const char* emoncmsKey = "your emoncms write key";


//emoncoms
const char* host = "emoncms.org";


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void serialEvent();
bool isInt(String str);
void sendToEmonCMS(String nodeId, String data);

void setup() {
  // make sure the serial baudrate is the same as on your serial sending device
  Serial.begin(9600);

  WiFiManager wifi;
  wifi.autoConnect();

  inputString.reserve(200);
}

void loop() {
  serialEvent(); 

  if (stringComplete) {
    Serial.println(inputString);
    String serialData = inputString;
    // clear the string:
    inputString = "";
    stringComplete = false;

    int spacePos = serialData.indexOf(" ");
    if (spacePos != -1) {
      //found a space, try to extract nodeId
      String nodeId = serialData.substring(0, spacePos);
      Serial.println(nodeId);
      if (isInt(nodeId)) {
        Serial.println("number");
        String data = serialData.substring(spacePos + 1);
        data.replace(" ", ",");

        delay(0);
        sendToEmonCMS(nodeId, data);
      }
    }

  }
}

/*
  SerialEvent occurs whenever a new data comes in the
  hardware serial RX.  This routine is run between each
  time loop() runs, so using delay inside loop can delay
  response.  Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
      return;
    } else {
      inputString += inChar;
    }
  }
}

//check if string is all digits
bool isInt(String str) {
  for (byte i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}

void sendToEmonCMS(String nodeId, String data) {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/input/post.json?node=";
  url += nodeId;
  url += "&apikey=";
  url += emoncmsKey;
  url += "&csv=";
  url += data;

  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
  }

}

