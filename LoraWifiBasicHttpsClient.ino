/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

ESP8266WiFiMulti WiFiMulti;
String output = "0";

void setup() {

  Serial.begin(9600);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("TriM Guest", "");
}

void loop() {
    // while there is data coming in, read it
    // and send to the hardware serial port:
    while (Serial.available() > 0) {
      output = "";
      String string = Serial.readString();
      int n_size = string.length();
      char number[n_size + 1];
      strcpy(number, string.c_str());
      
      int binaryNum[8];
      int i = 0;
      int n = atoi(number);
      while (n > 0) {
   
          // storing remainder in binary array
          binaryNum[i] = n % 2;
          output = String(binaryNum[i])+String(output);
          n = n / 2;
          i++;
      }
      while(output.length() < 8){
        output = String(0)+String(output);
      }
      // print our string
      Serial.println(string);
      Serial.println(number);
      Serial.print("Result");
      Serial.println(output);
    }
  
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setInsecure();
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    // client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    // wait for WiFi connection
    if (https.begin(*client, "https://www.tri-m.app/aminess/api/mqtt.php")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      https.addHeader("Content-Type", "application/json");
      // start connection and send HTTP header
      Serial.println(output);
      //char json[50];
      //sprintf(json, "{\"data\":{\"stanje\":%d}}", String(output));
      String json = String("{\"data\":{\"stanje\":\"")+String(output)+String("\"}}");
      Serial.println(json);
      int httpCode = https.POST(json);

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}
