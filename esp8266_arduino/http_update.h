#include <ESP8266HTTPClient.h>

bool http_update_initalized = false;

void http_update_setup(){
  
}

void http_update_loop(){
  if ((WiFi.status() == WL_CONNECTED) && !http_update_initalized) {
    do_http_update();
    http_update_initalized = true;
  }
}

void do_http_update() {
  
}
