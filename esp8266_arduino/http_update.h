#include <ESP8266HTTPClient.h>

bool http_update_initalized = false;

String http_update_path = "https://raw.githubusercontent.com/redhoodie/d1mini-pc-control/master/esp8266_arduino/data/";

//HTTPClient http;

void do_http_update();
void http_update_get(String file_name);

void http_update_setup(){
  
}

void http_update_loop(){
  if ((WiFi.status() == WL_CONNECTED) && !http_update_initalized) {
    do_http_update();
    http_update_initalized = true;
  }
}

void do_http_update() {
  http_update_get(String("script.js"));
  http_update_get(String("style.css"));
}

void http_update_get(String file_name) {
  log_write("HTTP Update: " + file_name);
  Serial.println("HTTP Update: " + file_name);

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

  bool mfln = client->probeMaxFragmentLength("raw.githubusercontent.com", 443, 1024);
  Serial.printf("\nConnecting to https://raw.githubusercontent.com\n");
  Serial.printf("Maximum fragment Length negotiation supported: %s\n", mfln ? "yes" : "no");
  if (mfln) {
    client->setBufferSizes(1024, 1024);
  }

  const uint8_t fingerprint[20] = {0xEB, 0xD9, 0xDF, 0x37, 0xC2, 0xCC, 0x84, 0x89, 0x00, 0xA0, 0x58, 0x52, 0x24, 0x04, 0xE4, 0x37, 0x3E, 0x2B, 0xF1, 0x41};

  client->setFingerprint(fingerprint);

  HTTPClient https;

  String url = String(http_update_path) + file_name;

  File f = SPIFFS.open(file_name, "w");
  if (f) {
    https.begin(*client, url);
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        https.writeToStream(&f);

        log_write("HTTP Update Success.");
        Serial.println("HTTP Update Success.");
      }
      else {
        log_write("[HTTP] GET... failed, error: " + String(https.errorToString(httpCode).c_str()) + " (" + String(httpCode) + ")");
        Serial.println("[HTTP] GET... failed, error: " + String(https.errorToString(httpCode).c_str()) + " (" + String(httpCode) + ")");
      }
    } else {
      log_write("[HTTP] GET... failed, error: " + String(https.errorToString(httpCode).c_str()) + " (" + String(httpCode) + ")");
      Serial.println("[HTTP] GET... failed, error: " + String(https.errorToString(httpCode).c_str()) + " (" + String(httpCode) + ")");
    }
    f.close();
  }
  https.end();
}
