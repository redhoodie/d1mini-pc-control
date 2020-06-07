#include <ESP8266HTTPClient.h>

bool http_update_initalized = false;

String http_update_path = "http://raw.githubusercontent.com/redhoodie/d1mini-pc-control/master/esp8266_arduino/data/";

HTTPClient http;

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

  String url = String(http_update_path) + file_name;

  File f = SPIFFS.open(file_name, "w");
  if (f) {
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        http.writeToStream(&f);
      }
    } else {
      log_write("[HTTP] GET... failed, error: " + String(http.errorToString(httpCode).c_str()));
      Serial.println("[HTTP] GET... failed, error: " + String(http.errorToString(httpCode).c_str()));
    }
    f.close();
  }
  http.end();
}
