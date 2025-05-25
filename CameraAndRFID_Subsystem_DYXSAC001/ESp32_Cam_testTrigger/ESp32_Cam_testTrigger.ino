////////CAMERA CODE BEST VERSION 1
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_wpa2.h" // WPA2 Enterprise library
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "esp_camera.h"

// CAMERA_MODEL_AI_THINKER GPIO.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_LED_PIN 4       // Camera Flash LED (active HIGH)
#define STATUS_LED_PIN 33     // Onboard LED (active LOW, inverted logic)
#define TRIGGER_PIN 2         // Use GPIO 2 as trigger input

#define EAP_ANONYMOUS_IDENTITY "anonymous@uct.ac.za"
#define EAP_IDENTITY           "dyxsac001@wf.uct.ac.za"
#define EAP_PASSWORD           "SachinDevil13@"
#define EAP_USERNAME           "dyxsac001@wf.uct.ac.za"

const char* ssid = "eduroam";

// Replace with your "Deployment ID" and Folder Name.
String myDeploymentID = "AKfycbyTZK96t5At-4vIIQr7UXgKaINBHN8_A1dwlIKBpbf3yw_0ThxuD8h9l_9kOE1EFtfV";
String myMainFolderName = "Group37Design";

bool LED_Flash_ON = false; // No flash on normal photos, only for success indication

WiFiClientSecure client;

// Indicate error or status using onboard LED (inverted logic: LOW=ON, HIGH=OFF)
void setStatusLED(bool on) {
  digitalWrite(STATUS_LED_PIN, on ? LOW : HIGH); // LOW = ON
}

// --- Flash blink function for indicator ---
void blinkFlash(int times = 2, int duration = 200) {
  for (int i = 0; i < times; i++) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(duration);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(duration);
  }
}

//--- Connection test with error/status indication ---
void Test_Con() {
  const char* host = "script.google.com";
  while(1) {
    Serial.println("-----------");
    Serial.println("Connection Test...");
    Serial.println("Connect to " + String(host));

    client.setInsecure();
    setStatusLED(true); // ON: indicate trying to connect

    if (client.connect(host, 443)) {
      Serial.println("Connection successful.");
      setStatusLED(false); // OFF: success
      Serial.println("-----------");
      client.stop();
      break;
    } else {
      Serial.println("Connected to " + String(host) + " failed.");
      Serial.println("Wait a moment for reconnecting.");
      setStatusLED(true); // ON: error
      Serial.println("-----------");
      client.stop();
    }
    delay(1000);
  }
}

//--- Capture and upload photo with flash indication only ---
void SendCapturedPhotos() {
  // --- Blink flash LED to indicate tag detected ---
  blinkFlash(2, 200);  // Blink twice, 200ms each

  const char* host = "script.google.com";
  Serial.println();
  Serial.println("-----------");
  Serial.println("Connect to " + String(host));
  client.setInsecure();

  if (client.connect(host, 443)) {
    Serial.println("Connection successful.");

    Serial.println();
    Serial.println("Taking a photo...");
    // Camera warm-up
    for (int i = 0; i <= 3; i++) {
      camera_fb_t * fb = NULL;
      fb = esp_camera_fb_get();
      if(!fb) {
        Serial.println("Camera capture failed");
        setStatusLED(true); // ON: error
        delay(1000);
        ESP.restart();
        return;
      }
      esp_camera_fb_return(fb);
      delay(200);
    }
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Camera capture failed");
      setStatusLED(true); // ON: error
      delay(1000);
      ESP.restart();
      return;
    }

    Serial.println("Taking a photo was successful.");

    // Sending image to Google Drive.
    Serial.println();
    Serial.println("Sending image to Google Drive.");
    Serial.println("Size: " + String(fb->len) + "byte");
    String url = "/macros/s/" + myDeploymentID + "/exec?folder=" + myMainFolderName;
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: " + String(host));
    client.println("Transfer-Encoding: chunked");
    client.println();

    int fbLen = fb->len;
    char *input = (char *)fb->buf;
    int chunkSize = 3 * 1000;
    int chunkBase64Size = base64_enc_len(chunkSize);
    char output[chunkBase64Size + 1];

    Serial.println();
    int chunk = 0;
    for (int i = 0; i < fbLen; i += chunkSize) {
      int l = base64_encode(output, input, min(fbLen - i, chunkSize));
      client.print(l, HEX);
      client.print("\r\n");
      client.print(output);
      client.print("\r\n");
      delay(100);
      input += chunkSize;
      Serial.print(".");
      chunk++;
      if (chunk % 50 == 0) {
        Serial.println();
      }
    }
    client.print("0\r\n");
    client.print("\r\n");
    esp_camera_fb_return(fb);

    // Wait for response.
    Serial.println("Waiting for response.");
    long int StartTime = millis();
    while (!client.available()) {
      Serial.print(".");
      delay(100);
      if ((StartTime + 10 * 1000) < millis()) {
        Serial.println();
        Serial.println("No response.");
        break;
      }
    }
    Serial.println();
    while (client.available()) {
      Serial.print(char(client.read()));
    }
    // Flash the flash LED after a successful upload as feedback (only here!)
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(500);
    digitalWrite(FLASH_LED_PIN, LOW);
    // Status LED remains OFF (no error)
    setStatusLED(false);

  } else {
    Serial.println("Connected to " + String(host) + " failed.");
    setStatusLED(true); // ON: error
    delay(500);
  }
  Serial.println("-----------");
  client.stop();
}

//--- SETUP ---
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector

  Serial.begin(115200);
  Serial.println();
  delay(1000);

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  pinMode(STATUS_LED_PIN, OUTPUT);
  setStatusLED(true); // ON at startup (until WiFi/connection OK)
  pinMode(TRIGGER_PIN, INPUT_PULLDOWN);

  // WiFi setup
  Serial.println();
  Serial.println("Setting the ESP32 WiFi to station mode.");
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  WiFi.disconnect(true);

  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  int connecting_process_timed_out = 20 * 2; // 20 seconds
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    setStatusLED(true); // ON: connecting/error
    delay(250);
    connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      Serial.println();
      Serial.print("Failed to connect to ");
      Serial.println(ssid);
      setStatusLED(true); // ON: error
      delay(1000);
      ESP.restart();
    }
  }

  setStatusLED(false); // OFF: connected
  Serial.println();
  Serial.println("Successfully connected to eduroam!");
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());

  // Camera setup
  Serial.println();
  Serial.println("Set the camera ESP32 CAM...");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 8;
    config.fb_count = 1;
  }
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    setStatusLED(true); // ON: error
    delay(1000);
    ESP.restart();
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SXGA);

  Serial.println("Setting the camera successfully.");
  Serial.println();

  delay(1000);

  Test_Con();

  Serial.println();
  Serial.println("ESP32-CAM will capture and send a photo only when TRIGGER_PIN (GPIO 2) goes HIGH.");
  setStatusLED(false); // OFF: ready/idle
  Serial.println();
  delay(2000);
}

//--- MAIN LOOP ---
void loop() {
  if (digitalRead(TRIGGER_PIN) == HIGH) {
    delay(50); // debounce
    if (digitalRead(TRIGGER_PIN) == HIGH) {
      SendCapturedPhotos();
      while (digitalRead(TRIGGER_PIN) == HIGH) {
        delay(10);
      }
      delay(200);
    }
  }
}
