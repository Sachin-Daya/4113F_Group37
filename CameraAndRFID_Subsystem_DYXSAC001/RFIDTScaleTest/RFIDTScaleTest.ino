#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>
#include <HX711.h>

// ----- Eduroam credentials -----
#define EAP_ANONYMOUS_IDENTITY "anonymous@uct.ac.za"
#define EAP_IDENTITY           "dyxsac001@wf.uct.ac.za"
#define EAP_PASSWORD           "SachinDevil13@"
#define EAP_USERNAME           "dyxsac001@wf.uct.ac.za"
const char* ssid = "eduroam";

// ----- Google Apps Script URL -----
String Web_App_URL = "https://script.google.com/macros/s/AKfycbzmTck2CD9jZx_da39DUtA-0vWwDUEC9DUAfPukkBu7MWC-8W2BTwncaRTKGSAqD23zaQ/exec";

// ----- RFID -----
#define SS_PIN      5
#define RST_PIN     22
#define TRIGGER_PIN 15  // GPIO to signal ESP32-CAM

MFRC522 mfrc522(SS_PIN, RST_PIN);

// ----- HX711 load cells -----
const int LOADCELL_DOUT_PIN_A = 13;
const int LOADCELL_SCK_PIN_A  = 12;
const int LOADCELL_DOUT_PIN_B = 14;
const int LOADCELL_SCK_PIN_B  = 27;
const int LOADCELL_DOUT_PIN_C = 26;
const int LOADCELL_SCK_PIN_C  = 25;
const int LOADCELL_DOUT_PIN_D = 33;
const int LOADCELL_SCK_PIN_D  = 32;
const float CAL_FACTOR_A = -32.177;
const float CAL_FACTOR_B = -105.496;
const float CAL_FACTOR_C = -147.5453;
const float CAL_FACTOR_D = -83.841;

HX711 scaleA, scaleB, scaleC, scaleD;

// ----- Globals -----
String UID_Result = "";

void http_Req(String uid, long avgWeight) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error! WiFi disconnected.");
    return;
  }

  // Add weight as parameter in URL
  String url = Web_App_URL + "?uid=" + uid + "&weight=" + avgWeight;
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

long readAverageWeight() {
  if (scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()) {
    Serial.println("READING...");
    long readingA = scaleA.get_units(20);
    long readingB = scaleB.get_units(20);
    long readingC = scaleC.get_units(20);
    long readingD = scaleD.get_units(20);
    Serial.println("RESULTS...");
    Serial.print("A: "); Serial.println(readingA);
    Serial.print("B: "); Serial.println(readingB);
    Serial.print("C: "); Serial.println(readingC);
    Serial.print("D: "); Serial.println(readingD);
    long avgWeight = (readingA + readingB + readingC + readingD) / 4;
    Serial.print("Average Weight: "); Serial.println(avgWeight);
    return avgWeight;
  } else {
    Serial.println("ERROR: HX711 NOT FOUND");
    return 0;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // ----- Setup RFID -----
  SPI.begin();      
  mfrc522.PCD_Init(); 

  // ----- Setup camera trigger pin -----
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW);

  // ----- Setup WiFi -----
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");

  Serial.print("Connecting to ");
  Serial.println(ssid);

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
  Serial.println("Ready to log attendance and trigger camera.");
  Serial.println("------------");

  // ----- Setup scales -----
  scaleA.begin(LOADCELL_DOUT_PIN_A, LOADCELL_SCK_PIN_A);
  scaleB.begin(LOADCELL_DOUT_PIN_B, LOADCELL_SCK_PIN_B);
  scaleC.begin(LOADCELL_DOUT_PIN_C, LOADCELL_SCK_PIN_C);
  scaleD.begin(LOADCELL_DOUT_PIN_D, LOADCELL_SCK_PIN_D);

  if (scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()) {
    scaleA.set_scale(CAL_FACTOR_A);   
    scaleB.set_scale(CAL_FACTOR_B);
    scaleC.set_scale(CAL_FACTOR_C); 
    scaleD.set_scale(CAL_FACTOR_D);

    //Tare Calibration
    Serial.println("TARE...");
    scaleA.tare();
    scaleB.tare();
    scaleC.tare();
    scaleD.tare();
    Serial.println("PLACE ITEM ON SCALE NOW");
    delay(5000);
  }
}

void loop() {
  if (getUID()) {
    Serial.println("Card Detected: " + UID_Result);

    // Read weight now
    long avgWeight = readAverageWeight();

    // Upload to Google Sheets
    http_Req(UID_Result, avgWeight);

    // --- Trigger camera ---
    digitalWrite(TRIGGER_PIN, HIGH);
    delay(200); // Hold high long enough for detection
    digitalWrite(TRIGGER_PIN, LOW);

    delay(1000); // Delay to prevent repeat reads
  }
}

