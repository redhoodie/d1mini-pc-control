#include <IotWebConf.h>

// VARIABLES

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

unsigned long lastSerialString = 0;
String serialInputString = "";         // a String to hold incoming data

// State Variables
bool waiting_for_state_change = false;
bool keep_alive = false;
unsigned long lastAction = 0;
unsigned int currentAction = NO_ACTION;
unsigned int needAction = NO_ACTION;
bool is_powered_on = false;

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

void web_setup() {
  // -- Initializing the configuration.
  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setupUpdateServer(&httpUpdater);
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  
  //  server.onNotFound([](){ iotWebConf.handleNotFound(); }); // Handled by FS.h

}

void web_loop() {
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }

  if (server.hasArg("pat")) {
    serialInputString = server.arg("pat") + "\n";
    lastSerialString = millis();
    
    server.send(204, "text/html", ".");
    log_write("KeepAlive Pat.");
    return;
  }

  if (server.hasArg("keepalive") && server.arg("keepalive") == "enable") {
    log_write("Enabled KeepAlive.");
    keep_alive = true;
  }
  else if (server.hasArg("keepalive") && server.arg("keepalive") == "disable") {
    log_write("Disabled KeepAlive.");
    keep_alive = false;
  }


  if (server.hasArg("action"))
  {
    String action = server.arg("action");
    if (action.equals("reset"))
    {
      needAction = RESET_ACTION;
    }
    else if (action.equals("poweron"))
    {
      needAction = POWER_ON_ACTION;
    }
    else if (action.equals("poweroff"))
    {
      needAction = POWER_OFF_ACTION;
    }
    else if (action.equals("shutdown"))
    {
      needAction = SHUTDOWN_ACTION;
    }
    else if (action.equals("ping"))
    {
      server.send(204, "text/html", "");
      return;
    }

    else if (action.equals("espreset"))
    {
      log_write("ESP RESET.");
      server.send(200, "text/html", "<!DOCTYPE html><html lang=\"en\"><head><meta http-equiv=\"refresh\" content=\"3;url=/\"/><title></title></head><body>Restarting...</body></html>");
      delay(1000);
      ESP.restart();
    }

    applyAction(millis());
  }

  String html_state = "";
  String html_controls = "";

  String html_spinner = "<span class=\"spinner-border spinner-border-sm\" role=\"status\" aria-hidden=\"true\"></span>";

  if (is_powered_on) {
    html_state = "<div class=\"alert alert-success\" role=\"alert\">Powered ON</div>";

    if (waiting_for_state_change && currentAction == SHUTDOWN_ACTION) {
      html_controls = "<p><a class=\"btn btn-secondary disabled\" href=\"/\">" + html_spinner + " Shutdown</a></p>";
    } else {
      html_controls = "<p><a class=\"btn btn-secondary\" href=\"/?action=shutdown\">Shutdown</a></p>";
    }

    if (waiting_for_state_change && currentAction == POWER_OFF_ACTION) {
      html_controls = "<p><a class=\"btn btn-danger disabled\" href=\"/\">" + html_spinner + " Power Off (Force)</a></p>";
    } else {
      html_controls += "<p><a class=\"btn btn-danger\" href=\"/?action=poweroff\">Power Off (Force)</a></p>";
    }

    if (waiting_for_state_change && currentAction == RESET_ACTION) {
      html_controls = "<p><a class=\"btn btn-danger disabled\" href=\"/\">" + html_spinner + " Reboot (Force)</a></p>";
    } else {
      html_controls += "<p><a class=\"btn btn-danger\" href=\"/?action=reboot\">Reboot (Force)</a></p>";
    }

  } else {
    html_state = "<div class=\"alert alert-danger\" role=\"alert\">Powered OFF</div>";

    if (waiting_for_state_change && currentAction == POWER_ON_ACTION) {
      html_controls = "<p><a class=\"btn btn-primary disabled\" href=\"/\">" + html_spinner + " Power On</a></p>";
    } else {
      html_controls = "<p><a class=\"btn btn-primary\" href=\"/?action=poweron\">Power On</a></p>";
    }
  }

  // Toggle Keep Alive
  if (keep_alive) {
    String html_keep_alive = "<div class=\"btn-group btn-group-toggle mb-3\" data-toggle=\"buttons\"><a class=\"btn btn-primary active\" href=\"/\">Enable keep alive</a><a class=\"btn btn-primary\" href=\"/?keepalive=disable\">Disable keep alive</a></div>";
    int keep_alive_timeout = KEEP_ALIVE_SECONDS - lastSerialStringSecondsAgo();
    if (keep_alive_timeout < (KEEP_ALIVE_SECONDS - 60)) {
      html_keep_alive += "<div class=\"alert alert-danger\">Keep Alive Action in " + String(keep_alive_timeout) + " seconds</div>";
    }
    html_controls = html_keep_alive + html_controls;
  }
  else if (!keep_alive) {
    String html_keep_alive = "<div class=\"btn-group btn-group-toggle mb-3\" data-toggle=\"buttons\"><a class=\"btn btn-primary\" href=\"/?keepalive=enable\">Enable keep alive</a><a class=\"btn btn-primary active\" href=\"/\">Disable keep alive</a></div>";
    html_controls = html_keep_alive + html_controls;    
  }

  // Default Heart Beat state
  String heart_beat_state = "warning";
  String heart_beat_title = "No Heart beat detected yet";
  String heart_beat_message = html_spinner;

  if (lastSerialString == 0 && (((float) millis()) / 1000) > 90) {
    heart_beat_state = "danger";
  }
  else if (lastSerialString != 0) {
    int lastSerialStringSecondsAgo_ = lastSerialStringSecondsAgo();
    if (lastSerialStringSecondsAgo_ < 65) {
      heart_beat_state = "success";
      heart_beat_title = "Last heart beat " + String(lastSerialStringSecondsAgo_) + " seconds ago";
      heart_beat_message = serialInputString;
    }
    else if (lastSerialStringSecondsAgo_ <= 90) {
      heart_beat_state = "warning";
      heart_beat_title = "Heart beat missed. Last heart beat " + String(lastSerialStringSecondsAgo_) + " seconds ago";
      heart_beat_message = html_spinner + " " + serialInputString;
    }
    else if (lastSerialStringSecondsAgo_ > 90) {
      heart_beat_state = "danger";
      heart_beat_title = "Heart beat lost. Last heart beat " + String(lastSerialStringSecondsAgo_) + " seconds ago";
      heart_beat_message = html_spinner + " " + serialInputString;
    }
  }
  
  html_state += "<div class=\"alert alert-" + heart_beat_state + "\" role=\"alert\"><strong>" + heart_beat_title + "</strong><br/> " + heart_beat_message + "</div>";

  String html_debug = "<p>";
  if (keep_alive) {
    html_debug += "keep_alive<br/>";
  } else {
    html_debug += "!keep_alive<br/>";
  }

  if (is_powered_on) {
    html_debug += "is_powered_on<br/>";
  } else {
    html_debug += "!is_powered_on<br/>";
  }
  if (isDead()) {
    html_debug += "isDead<br/>";
  } else {
    html_debug += "!isDead<br/>";
  }
  html_debug += "<a class=\"btn btn-danger\" href=\"/?action=espreset\">Reset ESP</a></p>\n";

  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<link rel=\"stylesheet\" href=\"/bootstrap.min.css\" integrity=\"sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm\" crossorigin=\"anonymous\">";
  s += "<link href=\"/styles.css\" rel=\"stylesheet\" crossorigin=\"anonymous\">";
  s += "<script src=\"/jquery-3.5.1.min.js\" integrity=\"sha256-9/aliU8dGd2tb6OSsuzixeV4y/faTqgFtohetphbbj0= sha384-ZvpUoO/+PpLXR1lu4jmpXWu80pZlYUAfxl5NsBMWOEPSjUn/6Z/hRTt8+pR6L4N2 sha512-bLT0Qm9VnAYZDflyKcBaQ2gg0hSYNQrJ8RilYldYQ1FxQYoCLtUjuuRuZo+fjqhx/qtq/1itJ0C2ejDxltZVFg==\" crossorigin=\"anonymous\"></script>";
  s += "<script src=\"/bootstrap.min.js\" integrity=\"sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl\" crossorigin=\"anonymous\"></script>";  
  s += "<script src=\"/script.js\" crossorigin=\"anonymous\"></script>";
  s += "<title>ESP8266 PC Power Control</title></head>";
  s += "<body><main role=\"main\">";
  s += "<div class=\"jumbotron\"><div class=\"container\"><h1 class=\"display-3\">ESP8266 PC Power Control</h1><p></p><p><a class=\"btn btn-primary btn-lg\" href=\"/config\">Config Page &raquo;</a></p></div></div>";
  s += "<div class=\"container\"><div id=\"state_control\" class=\"row\"><div class=\"col-sm-4\"><h2>Current State</h2><p>" + html_state + "</p></div><div class=\"col-sm-4\"><h2>Controls</h2>" + html_controls + "</div><div class=\"col-sm-4\"><h2>Debug</h2><div id=\"esp-status\" class=\"alert alert-success\" role=\"alert\">ESP Alive</div>" + html_debug + "</div></div></div><div class=\"container\"><div class=\"row\"><div class=\"col\"><h2>Log</h2><pre id=\"log\"></pre></div></div></div></div>";

  s += "</main></body></html>\n";

  server.send(200, "text/html", s);
}

bool isDead() {
  return lastSerialStringSecondsAgo() - KEEP_ALIVE_SECONDS > 0;
}

int lastSerialStringSecondsAgo() {
  signed long ago = millis() - lastSerialString;
  float seconds = ago / 1000;
  return (int) seconds;    
}

void applyAction(unsigned long now)
{
  if ((needAction != NO_ACTION)
    && (ACTION_FEQ_LIMIT < now - lastAction))
  {
    if (needAction == RESET_ACTION) {
      doReset();
    }
    else if (needAction == POWER_ON_ACTION) {
      doPowerOn();
    }
    else if (needAction == POWER_OFF_ACTION) {
      doPowerOff();
    }
    else if (needAction == SHUTDOWN_ACTION) {
      doShutdown();
    }

    currentAction = needAction;
    needAction = NO_ACTION;
    lastAction = now;
  }
}
