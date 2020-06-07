
void serial_setup() {
  serialInputString.reserve(200);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");
}

void serial_setup_final() {
  Serial.println("Ready.");
  log_write("Ready.");
}


void serialEvent() {
  while (Serial.available()) {
    int len = serialInputString.length();
    if (serialInputString.substring(len - 1, len) == "\n") {
      serialInputString = "";
    }

    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    serialInputString += inChar;

    if (inChar == '\n') {
      lastSerialString = millis();
    }
  }
}
