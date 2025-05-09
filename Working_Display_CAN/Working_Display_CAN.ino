#include <SPI.h>
#include <SD.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <genieArduino.h>

const int chipSelect = 53;
struct can_frame canMsg;
MCP2515 mcp2515(10);
 
// ←– Wiring: adjust if needed

#define DISP_TX_PIN   18    // UNO D11 ← Display TX
#define DISP_RX_PIN   19    // UNO D10 → Display RX
#define DISP_RST_PIN   4    // UNO D4 → Display RESET (optional)
#define DISP_BAUD    38400   // Must match your WS4 Comms Speed
#define DISP_SERIAL  Serial1    

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

Genie genie;
 
void setup() {
  //reset
  pinMode(DISP_RST_PIN, OUTPUT);
  digitalWrite(DISP_RST_PIN, LOW);
  delay(50);
  digitalWrite(DISP_RST_PIN, HIGH);
  delay(2000);
 
  // Start the serial link to the display
  DISP_SERIAL.begin(DISP_BAUD);

  genie.Begin(DISP_SERIAL);



  pinMode(10, OUTPUT);
  pinMode(53, OUTPUT);
  digitalWrite(10, HIGH);
  digitalWrite(53, HIGH);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

}

unsigned long currentTimeMillis = millis();
float currentTimeSeconds = currentTimeMillis / 1000.0; 
 
void loop() {
  genie.DoEvents();
    // put your main code here, to run repeatedly:
  digitalWrite(10, LOW);
  int total_data = 0;
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print("ID: ");
    Serial.println(canMsg.can_id);
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      total_data += int(canMsg.data[i]);
      
    }   
  }
  total_data /= canMsg.can_dlc;
  /*for (uint16_t v = 0; v <= 15000; v += 1000) {
    genie.WriteObject(GENIE_OBJ_IGAUGE, IDX_RPM_METER, v);
    delay(20);
  }
  delay(500);
 
  // Sweep back down 15 000 → 0
  for (int16_t v = 15000; v >= 0; v -= 1000) {
    genie.WriteObject(GENIE_OBJ_IGAUGE, IDX_RPM_METER, v);
    delay(20);
  }
  */
  static uint16_t v = 0;
 
  // Update the Coolant-IN 4-digit display
  switch(canMsg.can_id){
  case(0x000):
  Serial.println("1");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_GEAR, 9);
  break;
  case(0x001):
  Serial.println("2");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_IN, total_data);
  break;
  case(0x002):
  Serial.println("3");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_OUT, total_data);
  break;
  case(0x003):
  Serial.println("4");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_FUEL_PRESSURE, total_data);
  break;
  case(0x004):
  Serial.println("5");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_TEMP, total_data);
  break;
  case(0x005):
  Serial.println("6");
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_PRESSURE, total_data);
  break;
  default:
  Serial.println("ERR");
  break;
  }
  genie.WriteObject(GENIE_OBJ_ILED, 0, 1);
  genie.WriteObject(GENIE_OBJ_ILED, 1, 1);
  genie.WriteObject(GENIE_OBJ_ILED, 0, 0);
  genie.WriteObject(GENIE_OBJ_ILED, 1, 0);
  digitalWrite(10, HIGH);


}

 