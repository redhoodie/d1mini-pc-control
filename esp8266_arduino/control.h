// Hardware Control / State

Ticker powered_on_ticker;
Ticker keep_alive_action_ticker;

void control_setup() {  
  pinMode(PWR_LED_PIN, INPUT);
//  pinMode(HDD_LED_PIN, INPUT);
//  pinMode(PWR_PIN, OUTPUT);
//  pinMode(RST_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
//  pinMode(LED_ENABLE, OUTPUT);
//  digitalWrite(LED_ENABLE, LOW);
 

  keep_alive_action_ticker.attach(1, keep_alive_check);
  powered_on_ticker.attach(0.3, update_powered_on);
}


// TIMERS

void keep_alive_check() {
  if (keep_alive && isDead() && !waiting_for_state_change) {
    doKeepAlive();
  }
}

void update_powered_on() {
  bool value = (digitalRead(PWR_LED_PIN) == HIGH);
  if (value != is_powered_on) {
    // State changed
    waiting_for_state_change = false;
    currentAction = NO_ACTION;
  }
  is_powered_on = value;

  digitalWrite(LED_BUILTIN, !is_powered_on);
}


// ACTIONS

void doPowerOn() {
  log_write("DO POWER ON");
  Serial.println("DO POWER ON");
  if (is_powered_on) {
    Serial.println("System is already ON. Skipping task.");
    return;
  }

  waiting_for_state_change = true;

  togglePin(PWR_PIN, PUSH_TIME);
  Serial.println("Power ON signal was sent");
}

void doPowerOff() {
  log_write("DO POWER OFF");
  Serial.println("DO POWER OFF");
  if (!is_powered_on) {
    Serial.println("System is already off. Skipping task.");
    return;
  }

  waiting_for_state_change = true;

  togglePin(PWR_PIN, PWR_OFF_TIME);
  Serial.println("Power OFF signal was sent");
}

void doShutdown() {
  log_write("DO SHUTDOWN");
  Serial.println("DO SHUTDOWN");
  if (!is_powered_on) {
    Serial.println("System is already off. Skipping task.");
    return;
  }

  waiting_for_state_change = true;

  togglePin(PWR_PIN, PUSH_TIME);
  Serial.println("Shutdown signal was sent");
}

void doReset() {
  Serial.println("DO RESET");
  log_write("DO RESET");
  if (!is_powered_on) {
    Serial.println("System is turned off. Skipping task.");
    return;
  }

  waiting_for_state_change = false;

  togglePin(RST_PIN, PUSH_TIME);
  Serial.println("Reset signal was sent");
}

void doHardReset() {
   Serial.println("DO HARD RESET");
  log_write("DO HARD RESET");
  if (!is_powered_on) {
    Serial.println("System is turned off. Skipping task.");
    return;
  }

  waiting_for_state_change = false;

  togglePin(PWR_PIN, PWR_OFF_TIME);
  delay(2000);
  togglePin(PWR_PIN, PUSH_TIME);
  Serial.println("Reset signal was sent");
}

void doKeepAlive() {
  log_write("DO KEEP ALIVE");
  if (is_powered_on) {
    doHardReset();
  } else {
    needAction = POWER_ON_ACTION;
    applyAction(millis());
    doPowerOn();
  }
  keep_alive = false;
}

void togglePin(int pin, int ms)
{
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
  delay(ms);
  pinMode(pin, INPUT);
}
