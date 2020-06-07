#include "defs.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#include "arduino_ota.h"
#include "web.h"
//#include "fs.h"
//#include "log.h"
#include "control.h"
#include "serial.h"
//#include "ftp.h"

void setup() {
  serial_setup();
  control_setup();
  web_setup();
//  fs_setup();
//  log_setup();
//  ftp_setup();

  serial_setup_final();
}

void loop() {
  // -- doLoop should be called as frequently as possible.
  web_loop();
  arduino_ota_loop();
//  log_loop();
//  ftp_loop();
}
