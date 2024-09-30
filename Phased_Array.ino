int pin_no[6] = {13, 12, 14, 27, 26, 25};
void setup() {

  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(25, OUTPUT);
}

void loop() {
  activate_Pins(pin_no, 2);
}

void activate_Pins(int pin_no[6], int delay) {
  for (int i = 0; i < 6; i++) {
    digitalWrite((pin_no[i]), LOW);
    digitalWrite((pin_no[i]), HIGH);
    delayMicroseconds(5);
    digitalWrite((pin_no[i]), LOW);
    delayMicroseconds(delay);
    Serial.println("Pin activated is: " + String(pin_no[i]));
  }
}