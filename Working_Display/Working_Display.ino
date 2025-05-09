#include <genieArduino.h>
 
// ←– Wiring: adjust if needed

#define DISP_TX_PIN   18    // UNO D11 ← Display RX
#define DISP_RX_PIN   19   // UNO D10 → Display TX
#define DISP_RST_PIN   4    // UNO D4 → Display RESET (optional)
#define DISP_SERIAL  Serial1
#define DISP_BAUD    38400   // Must match your WS4 Comms Speed
 
// ←– Your Coolant-IN widget’s index:

#define TICKS_PER_1000_RPM 5  // Adjust ticks per 1000 RPM
#define RED_RPM_THRESHOLD 12750  // RPM where gauge turns red
#define SHIFT_RPM_THRESHOLD 12750

#define IDX_RPM_METER     0
#define IDX_COOL_IN       0 
#define IDX_COOL_OUT      1
#define IDX_FUEL_PRESSURE 2
#define IDX_OIL_TEMP      3  
#define IDX_OIL_PRESSURE  4 
#define IDX_GEAR          5    // as you reported

bool shiftAlertState = false;
bool sensorFailureState = false;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 500; // 500ms blink rate
long randomNum = 0;
 

Genie genie;
 
void setup() {
  //RESET
  pinMode(DISP_RST_PIN, OUTPUT);
  digitalWrite(DISP_RST_PIN, LOW);
  delay(50);
  digitalWrite(DISP_RST_PIN, HIGH);
  delay(2000);
 
  // Start the serial link to the display
  DISP_SERIAL.begin(DISP_BAUD);

  genie.Begin(DISP_SERIAL);

}


 
void loop() {
  static unsigned long lastUpdate = 0;
  static int rpm = 0;

  if (millis() - lastUpdate >= 200) {
    lastUpdate = millis();
    rpm = (rpm + 500) % 15000;
    genie.WriteObject(GENIE_OBJ_IGAUGE, 0, rpm);
  }
  
  static uint16_t v = 0;
 
  // Update the Coolant-IN 4-digit display
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_GEAR, 9);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_IN, v);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_OUT, v);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_FUEL_PRESSURE, v);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_TEMP, v);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_PRESSURE, v);

  genie.WriteObject(GENIE_OBJ_ILED, 0, 1);
  genie.WriteObject(GENIE_OBJ_ILED, 1, 1);
  genie.WriteObject(GENIE_OBJ_ILED, 0, 0);
  genie.WriteObject(GENIE_OBJ_ILED, 1, 0);
  v = (v + 1) % 10000;
 
}

 