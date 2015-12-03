#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

//TEMP
#include <Ticker.h>
Ticker ticker;

#define ENABLE_SSD1306

#ifdef ENABLE_SSD1306
#include <ssd1306_i2c.h> //https://github.com/squix78/esp8266-oled-ssd1306
//#include "fonts.h"

SSD1306 display(0x3c, 0, 2);

#define heating_width 32
#define heating_height 32
const char heating_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00, 0xE0, 0x03, 0x00, 
  0x00, 0xF0, 0x01, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 
  0x00, 0xFC, 0x01, 0x00, 0x00, 0xFC, 0x01, 0x00, 0x00, 0xFC, 0x03, 0x00, 
  0x00, 0xF8, 0x07, 0x00, 0x80, 0xF8, 0x0F, 0x00, 0x80, 0xFB, 0x1F, 0x00, 
  0x00, 0xFF, 0x3F, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0x7F, 0x00, 
  0x00, 0x7E, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0x00, 0x80, 0x7F, 0xFE, 0x00, 
  0x80, 0x3F, 0xFE, 0x01, 0xC0, 0x3F, 0xFC, 0x01, 0xC0, 0x1F, 0xFC, 0x01, 
  0xE0, 0x0F, 0xEE, 0x01, 0xE0, 0x07, 0xE6, 0x01, 0xC0, 0x03, 0xC7, 0x01, 
  0xC0, 0x03, 0xC1, 0x00, 0xC0, 0x03, 0xE0, 0x00, 0x80, 0x03, 0x70, 0x00, 
  0x00, 0x07, 0x70, 0x00, 0x00, 0x07, 0x38, 0x00, 0x00, 0x0C, 0x0C, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};   

#define water_width 32
#define water_height 32
static char water_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 
  0x00, 0xC0, 0x03, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0xF0, 0x0F, 0x00, 
  0x00, 0xF8, 0x1F, 0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0xFC, 0x3F, 0x00, 
  0x00, 0xFE, 0x7F, 0x00, 0x00, 0xFE, 0x7F, 0x00, 0x00, 0xFF, 0xFF, 0x00, 
  0x00, 0xFF, 0xFF, 0x00, 0x80, 0xFF, 0xFF, 0x01, 0x80, 0xFF, 0xFF, 0x01, 
  0x80, 0xFF, 0xFF, 0x01, 0xC0, 0xFF, 0xFF, 0x03, 0xC0, 0xFF, 0xFF, 0x03, 
  0xC0, 0xFF, 0xDF, 0x03, 0xC0, 0xFF, 0x9F, 0x03, 0xC0, 0xFF, 0x8F, 0x03, 
  0xC0, 0xFF, 0xCF, 0x03, 0xC0, 0xFF, 0xC7, 0x03, 0x80, 0xFF, 0xC3, 0x01, 
  0x80, 0xFF, 0xE0, 0x01, 0x00, 0x3F, 0xF0, 0x00, 0x00, 0x7F, 0xFE, 0x00, 
  0x00, 0xFE, 0x7F, 0x00, 0x00, 0xFC, 0x3F, 0x00, 0x00, 0xF0, 0x1F, 0x00, 
  0x00, 0xC0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
};


#endif

#include "config.h"
//contains
//const char* emoncmsKey = "your emoncms write key";


//emoncoms
const char* host = "emoncms.org";


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

String c1 = "0";
String c2 = "0";
String c3 = "0";
String v = "0";

void serialEvent();
bool isInt(String str);
void sendToEmonCMS(String nodeId, String data);

void drawFrame(int x, int y);

String getValue(String data, char separator, int index)
{

    int maxIndex = data.length()-1;
    int j=0;
    String chunkVal = "";

    for(int i=0; i<=maxIndex && j<=index; i++)
    {

      if(data[i]==separator)
      {
        j++;

        if(j>index)
        {
          chunkVal.trim();
          return chunkVal;
        }    

        chunkVal = "";    
      }  
      else {
        chunkVal.concat(data[i]);
      }
    }  
}


void injectData() {
  inputString = "10 1163.90 243.00 12376.00 0.00 220.22";
  stringComplete = true;
  Serial.println("injected");
}

void setup() {
  //ticker.attach(10, injectData);
  
  // make sure the serial baudrate is the same as on your serial sending device
  Serial.begin(9600);

#ifdef ENABLE_SSD1306
  // initialize dispaly
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32 - 8, "connecting");
  display.display();
#endif

  WiFiManager wifi;
  wifi.setTimeout(120); //so if it restarts and router is not yet online, it keeps rebooting and retrying to connect
  if(!wifi.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
#ifdef ENABLE_SSD1306
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32 - 8, "FAILED");
  display.display();
#endif

    delay(1000);
    //reset and try again
    ESP.reset();
    delay(5000);
  } 

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
        delay(0);

#ifdef ENABLE_SSD1306
        c1 = String(getValue(data, ',', 0).toInt());
        c2 = String(getValue(data, ',', 1).toInt());
        c3 = String(getValue(data, ',', 2).toInt());
        v = String(getValue(data, ',', 4).toInt());
        Serial.println(c1);
        Serial.println(c2);
        Serial.println(c3);
        Serial.println(v);
#endif
      }
    }

  }

#ifdef ENABLE_SSD1306
  display.clear();
  drawFrame(0, 0);
  display.display();
#endif
}


#ifdef ENABLE_SSD1306

/*
void drawFrame1(int x, int y) {
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  String date = "november";
  int textWidth = display.getStringWidth(date);
  display.drawString(64 + x, 10 + y, date);
  display.setFont(ArialMT_Plain_24);
  String time = "12:12PM";
  textWidth = display.getStringWidth(time);
  display.drawString(64 + x, 20 + y, time);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}
*/


void drawItem(int x, int y, String value, String unit) {
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(x, y, value);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(x + 2, y + 5, unit);
}

void drawItemSmall(int x, int y, String value, String unit) {
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(x, y, value);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(x + 1, y, unit);
}

void drawFrame(int x, int y) {
//  double column
  drawItem(x + 118, y + 2 , c1, "w");
  drawItem(x + 118, y + 2 + 22, c2, "w");
  drawItem(x + 118, y + 2 + 44, c3, "w");
  drawItem(x + 45, y + 48, v, "v");
  display.drawXbm(x + 3, y + 6, heating_width, heating_height, heating_bits);
  display.drawXbm(x + 35, y + 6, water_width, water_height, water_bits);
/*
//single column
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(x +45, y, v);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(x + +45 + 2, y, "v");
  
  drawItem(x + 45, y + 10 , c1, "w");
  drawItem(x + 45, y + 27, c2, "w");
  drawItem(x + 45, y + 44, c3, "w");
*/
/*  //single row
  drawItemSmall(x + 35, y + 20, c1, "w");
  drawItemSmall(x + 80, y + 20, c2, "w");
  drawItemSmall(x + 120, y + 20, c3, "w");
  drawItem(76, y, v, "v");
*/
}
#endif

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

