#include <ESP8266FtpServer.h>

bool ftp_initalized = false;
FtpServer ftpSrv;

void ftp_setup(){}

void ftp_loop() {
  if ((WiFi.status() == WL_CONNECTED) && !ftp_initalized) {
    ftpSrv.begin("esp8266","esp8266");
    ftp_initalized = true;
  }
  ftpSrv.handleFTP();
}
