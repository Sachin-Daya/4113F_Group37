#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>

// Replace these with your eduroam credentials:
#define EAP_ANONYMOUS_IDENTITY "anonymous@uct.ac.za"
#define EAP_IDENTITY           "dyxsac001@wf.uct.ac.za"
#define EAP_PASSWORD           "SachinDevil13@"
#define EAP_USERNAME           "dyxsac001@wf.uct.ac.za"

const char* ssid = "eduroam";

// Google Apps Script Web App URL
String Web_App_URL = "https://script.google.com/macros/s/AKfycbw_0ynD7BNs7uizqB9FjDF_JdGhFnei3nZMQani1Uut_jslIdH3zMOYIomOCFtqK4ap-A/exec";

// RFID
#define SS_PIN  5  
#define RST_PIN 22
#define TRIGGER_PIN 15  // GPIO to signal ESP32-CAM *********TRIGGER
MFRC522 mfrc522(SS_PIN, RST_PIN);

String UID_Result = "";

void http_Req(String uid) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error! WiFi disconnected.");
    return;
  }

  String url = Web_App_URL + "?uid=" + uid;
  Serial.println("\n-------------");
  Serial.println("Sending request to Google Sheets...");
  Serial.println("URL : " + url);

  HTTPClient http;
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET(); 
  Serial.print("HTTP Status Code : ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Payload : " + payload);
  }
  Serial.println("-------------");
  http.end();
}

bool getUID() {  
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  byteArray_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, UID_Result);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return true;
}

void byteArray_to_string(byte *array, unsigned int len, String &out) {
  char buffer[32];
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  buffer[len * 2] = '\0';
  out = String(buffer);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin();      
  mfrc522.PCD_Init(); 

  //*********TRIGGER CODE
  //pinMode(TRIGGER_PIN, OUTPUT);
  //digitalWrite(TRIGGER_PIN, LOW);

  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  // WPA2 Enterprise connection (eduroam)
  WiFi.disconnect(true);
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  int timeout = 80; // 20 seconds
  while (WiFi.status() != WL_CONNECTED && timeout-- > 0) {
    Serial.print(".");
    delay(250);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed. Restarting...");
    delay(1000);
    ESP.restart();
  }
  Serial.println("\nWiFi connected");
  Serial.println("Ready to log attendance.");
  Serial.println("------------");
}

void loop() {
  if (getUID()) {
    Serial.println("Card Detected: " + UID_Result);
    http_Req(UID_Result);
    delay(1000); // Delay to prevent repeat reads


  }
}

