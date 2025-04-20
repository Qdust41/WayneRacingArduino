int in1 = 8; // Motor direction control pin
int in2 = 9;
int ConA = 10; // PWM pin for speed control
int buttonPin = A1; // Pushbutton connected to A1
bool motorOn = false; // Tracks motor state
int lastButtonState = HIGH; // Start as HIGH due to pull-up resistor

void setup() {
pinMode(in1, OUTPUT);
pinMode(in2, OUTPUT);
pinMode(ConA, OUTPUT);
pinMode(buttonPin, INPUT_PULLUP); // Enable internal pull-up resistor
}

void loop() {
int buttonState = digitalRead(buttonPin); // Read the button state
// Detect button press (HIGH to LOW transition due to pull-up)
if (buttonState == LOW && lastButtonState == HIGH) {
motorOn = !motorOn; // Toggle motor state
if (motorOn) {
digitalWrite(in1, LOW);
digitalWrite(in2, HIGH);
analogWrite(ConA, 2147483646); // Full speed this is the int bit limit -1 because it is a percent of the value
} else {
analogWrite(ConA, 0); // Stop motor
}
delay(200); // Simple debounce delay
}
lastButtonState = buttonState; // Store the last button state
}
