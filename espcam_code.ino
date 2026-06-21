#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>

// ===========================
// CAMERA MODEL SELECTION
// ===========================
#define CAMERA_MODEL_ESP32S3_EYE

// ===========================
// ESP32-S3-EYE CAMERA PINS
// ===========================

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15

#define SIOD_GPIO_NUM     4
#define SIOC_GPIO_NUM     5

#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM       8
#define Y3_GPIO_NUM       9
#define Y2_GPIO_NUM       11

#define VSYNC_GPIO_NUM    6
#define HREF_GPIO_NUM     7
#define PCLK_GPIO_NUM     13

// ===========================
// ENTER YOUR WIFI DETAILS
// ===========================

const char* ssid = "Mohit2";
const char* password = "999999999";

// ===========================
// CAMERA WEB SERVER
// ===========================

#include <WebServer.h>

WebServer server(80);

void handle_jpg_stream(void) {
  WiFiClient client = server.client();

  String response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";

  server.sendContent(response);

  while (true) {

    camera_fb_t * fb = esp_camera_fb_get();

    if (!fb) {
      Serial.println("Camera capture failed");
      continue;
    }

    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += "Content-Length: " + String(fb->len) + "\r\n\r\n";

    server.sendContent(response);

    client.write(fb->buf, fb->len);

    server.sendContent("\r\n");

    esp_camera_fb_return(fb);

    if (!client.connected()) {
      break;
    }
  }
}

void handle_root() {

  String html =
    "<html><body style='text-align:center;'>"
    "<h1>ESP32-S3-EYE Camera</h1>"
    "<img src='/stream' width='100%'>"
    "</body></html>";

  server.send(200, "text/html", html);
}

void startCameraServer() {

  server.on("/", HTTP_GET, handle_root);

  server.on("/stream", HTTP_GET, handle_jpg_stream);

  server.begin();

  Serial.println("Camera server started");
}

// ===========================
// SETUP
// ===========================

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("ESP32-S3-EYE Camera Starting...");

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

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;

  // SAFER SETTINGS
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  // ===========================
  // CAMERA INIT
  // ===========================

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.printf("Camera init failed with error 0x%x\n", err);

    return;
  }

  Serial.println("Camera initialized successfully");

  sensor_t * s = esp_camera_sensor_get();

  s->set_vflip(s, 1);

  // ===========================
  // WIFI CONNECT
  // ===========================

  WiFi.begin(ssid, password);

  WiFi.setSleep(false);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  // ===========================
  // START SERVER
  // ===========================

  startCameraServer();

  Serial.print("Camera Ready! Open: http://");

  Serial.println(WiFi.localIP());
}

// ===========================
// LOOP
// ===========================

void loop() {

  server.handleClient();

  delay(1);
}
