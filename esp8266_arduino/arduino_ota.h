#include <ArduinoOTA.h>

bool OTAInitalized = false;

void arduino_ota_loop() {
  if ((WiFi.status() == WL_CONNECTED) && !OTAInitalized) {
    Serial.println("Initalizing OTA");
    ArduinoOTA.setPassword((const char *)"M@ster");
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
      log_write("ArduinoOTA Uploaded.");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    OTAInitalized = true;
  }
  else if ((WiFi.status() == WL_CONNECTED) && OTAInitalized) {
    ArduinoOTA.handle();
  }
}
