#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <StringArray.h>
#include "Arduino.h"
#include "driver/rtc_io.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
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


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
    <head>
        <meta http-equiv="refresh" content="12" />
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body { text-align:center; }
            button { width: 96px;}
            img { min-width: 300px;}

            .hori{ margin-bottom: 0%; }
            .vert { margin-bottom: 10%; }
        </style>
    </head>
    <body>
        <div id="container">
            <h2>MIRO: Hall-CAM</h2>
            <p>Takes a photo of the entrance as the front opens.</p>
            <p>Shoot photos manually by pressing the 'SHOOT' button below.<br/>Shoot photos remotely by publishing "FIRE" to trigger/CAM.</p>
            <p>
                <button class="button" onclick="capturePhoto()">SHOOT</button>
                <button class="button" onclick="rotatePhoto();">ROTATE</button>
            <!-- <button onclick="location.reload();">REFRESH PAGE</button> -->
            </p>
        </div>
        <div>
            <img src="saved-photo" id="photo" width="25%">
        </div>
    </body>
    <script>
        var deg = 0;
        function capturePhoto() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', "/capture", true);
            xhr.send();
        }
        function rotatePhoto() {
            var img = document.getElementById("photo");
            deg += 90;
            if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
            else{ document.getElementById("container").className = "hori"; }
            img.style.transform = "rotate(" + deg + "deg)";
        }
        function isOdd(n) { return Math.abs(n % 2) == 1; }
    </script>
</html>
)rawliteral";


boolean shoot_next = false;

  // was shoot successful?
bool check_photo( fs::FS &fs ) {
  File photo = fs.open( FILE_PHOTO );
  unsigned int size = photo.size();
  return ( size > 100 );
}

  // shoot and save to spiffs
void shoot_and_save( void ) {
  camera_fb_t* fb = NULL;
  bool success = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    success = check_photo(SPIFFS);
  } while ( !success );
}

void setup_cam_module(AsyncWebServer& server)
{
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS: something went wrong. Restarting mcu...");
    ESP.restart();
  }
  else {
    delay(400);
    Serial.println("SPIFFS initialized.");
  }

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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    shoot_next = true;
    request->send_P(200, "text/plain", "Taking Photo");
  });
  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });

  server.begin();
}

void shoot()
{
  if (shoot_next) {
    digitalWrite(LED, HIGH);
    shoot_and_save();
    shoot_next = false;
    digitalWrite(LED, LOW);
  }
  delay(1);
}