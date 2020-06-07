// HEADER DEFINITIONS
void handleRoot();
bool isDead();

int lastSerialStringSecondsAgo();

// Control
void applyAction(unsigned long now);
void togglePin(int pin, int ms);
void doReset();
void doShutdown();
void doPowerOn();
void doPowerOff();
void doKeepAlive();
void keep_alive_check();
void update_powered_on();

// Log
void log_write(String line);


#define ACTION_FEQ_LIMIT 10000
#define NO_ACTION -1
#define RESET_ACTION 1
#define POWER_ON_ACTION 2
#define POWER_OFF_ACTION 3
#define SHUTDOWN_ACTION 4
#define KEEP_ALIVE_SECONDS 300

#define PWR_PIN D7 // Pin for the PWR signal line
#define RST_PIN D6 // Pin for the RST signal line
#define PWR_LED_PIN D5 // Pin for the status LED signal line
#define HDD_LED_PIN D8 // Pin for the status LED signal line
#define LED_ENABLE D0 // Pin for enabling Case LED Lights

#define STRING_LEN 128

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// How long the PowerOFF button should be pressed to power off PC forcefully
#define PWR_OFF_TIME 4500

// How long the button should be pressed to REBOOT, POWER ON or RESET
#define PUSH_TIME 400
