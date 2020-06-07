#include <NTPClient.h>
#include <WiFiUdp.h>

#define MAX_LOG_SIZE 65536
#define TRIM_LOG_SIZE (MAX_LOG_SIZE / 2)
#define LOG_CHECK_INTERVAL 10
#define LOG_PATH "/log.txt"
#define LOG_TMP_PATH "/log.txt.tmp"
#define LOG_FS fileSystem

long log_line_writes = 0;
File logFile;
void do_log_cleanup();

#define LOG_NTP_UPDATE_INTERVAL 300000


WiFiUDP ntpUDP;
int utcOffsetInSeconds = 43200; //NZST
unsigned long log_previousNTPMillis = 0;

NTPClient timeClient(ntpUDP, "nz.pool.ntp.org", utcOffsetInSeconds, LOG_NTP_UPDATE_INTERVAL);
bool ntp_initalized = false;

void log_setup() {}

void log_loop() {
  // Setup once WL_CONNECTED
  if ((WiFi.status() == WL_CONNECTED) && !ntp_initalized) {
    Serial.println("Initalizing NTP");
    timeClient.begin();
    ntp_initalized = true;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - log_previousNTPMillis >= 60000) {
    // save the last time you blinked the LED
    log_previousNTPMillis = currentMillis;
    Serial.println("NTP Update");
    timeClient.update();
  }

  if (log_line_writes > LOG_CHECK_INTERVAL) {
    do_log_cleanup();
  }
}


String log_ntp_time() {
  return String(timeClient.getEpochTime());
}

void log_write(String line) {
  logFile = LOG_FS->open(LOG_PATH, "a+");
  if (logFile) {
    line = log_ntp_time() + " - " + line;
    logFile.println(line);
    logFile.close();
    log_line_writes++;
  }
  else {
    // Error
  }
}


void do_log_cleanup() {
  File oldLogFile = LOG_FS->open(LOG_PATH, "r");

  if (oldLogFile.size() > MAX_LOG_SIZE) {
    // Trim log file
    logFile = LOG_FS->open(LOG_TMP_PATH, "a+");
    if (oldLogFile && logFile) {
      oldLogFile.seek(TRIM_LOG_SIZE);

      while(oldLogFile.available()) {
        logFile.write(oldLogFile.read());
      }
      logFile.close();
      oldLogFile.close();

      LOG_FS->remove(LOG_PATH);
      LOG_FS->rename(LOG_TMP_PATH, LOG_PATH);
    }
  }
  log_line_writes = 0;
}
