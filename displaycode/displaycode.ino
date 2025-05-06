#include <SPI.h> 
#include <mcp_can.h>
#include <SoftwareSerial.h>
#include <genieArduino.h>

#define CAN_CS 10
#define CAN_INT 2

// Create CAN and display objects
MCP_CAN CAN(CAN_CS);
Genie genie;
SoftwareSerial DisplaySerial(10, 11);  // RX, TX for gen4-uLCD

// RPM Gauge Settings
#define TICKS_PER_1000_RPM 5  // Adjust ticks per 1000 RPM
#define RED_RPM_THRESHOLD 8000  // RPM where gauge turns red
#define SHIFT_RPM_THRESHOLD 9000  // RPM where shift alert triggers

// Flashing LED Variables
bool shiftAlertState = false;
bool sensorFailureState = false;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 500; // 500ms blink rate

void setup() {
  Serial.begin(115200);  // Serial monitor
  DisplaySerial.begin(9600); // UART to display
  genie.Begin(DisplaySerial);
  genie.WriteContrast(15); // Adjust screen brightness

  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN BUS Initialized Successfully");
  } else {
    Serial.println("CAN BUS Initialization Failed!");
    while (1);
  }
  
  CAN.setMode(MCP_NORMAL);
}

void loop() {
  long unsigned int canId;
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    CAN.readMsgBuf(&canId, &len, buf);
    Serial.print("Received CAN ID: 0x"); Serial.println(canId, HEX);

    // Decode RPM
    if (canId == 0x123) {  // Replace with actual EL129 CAN ID for RPM
      int rpm = (buf[2] << 8) | buf[3];
      Serial.print("RPM: "); Serial.println(rpm);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0, rpm);  // Update RPM display

      // Update RPM Gauge
      int gaugeValue = (rpm / 1000) * TICKS_PER_1000_RPM;
      genie.WriteObject(GENIE_OBJ_GAUGE, 0, gaugeValue);

      // Change gauge color based on RPM
      if (rpm >= RED_RPM_THRESHOLD) {
        genie.WriteObject(GENIE_OBJ_USER_LED, 0, 1); // Turn gauge RED
      } else {
        genie.WriteObject(GENIE_OBJ_USER_LED, 0, 0); // Keep gauge GREEN
      }

      // Shift alert logic
      if (rpm >= SHIFT_RPM_THRESHOLD) {
        shiftAlertState = true;
      } else {
        shiftAlertState = false;
      }
    }

    // Decode Oil Temp
    if (canId == 0x456) {  
      int oilTemp = buf[4];
      Serial.print("Oil Temp: "); Serial.println(oilTemp);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, oilTemp);
    }

    // Decode Oil Pressure
    if (canId == 0x789) {  
      int oilPressure = buf[5];
      Serial.print("Oil Pressure: "); Serial.println(oilPressure);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 2, oilPressure);
    }

    // Decode Fuel Pressure
    if (canId == 0xABC) {  
      int fuelPressure = buf[6];
      Serial.print("Fuel Pressure: "); Serial.println(fuelPressure);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 3, fuelPressure);
    }

    // Decode Coolant Temp In (°C)
    if (canId == 0xDEF) {  
      int coolantTempIn = buf[7];
      Serial.print("Coolant Temp In (°C): "); Serial.println(coolantTempIn);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 4, coolantTempIn);
    }

    // Decode Coolant Temp Out (°C)
    if (canId == 0x101) {  
      int coolantTempOut = buf[0];
      Serial.print("Coolant Temp Out (°C): "); Serial.println(coolantTempOut);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 5, coolantTempOut);
    }

    // Decode Gear Position (1-digit display)
    if (canId == 0x202) {  
      int gearPosition = buf[1] & 0x0F;
      Serial.print("Gear Position: "); Serial.println(gearPosition);
      genie.WriteObject(GENIE_OBJ_LED_DIGITS, 6, gearPosition);
    }


    // Sensor failure detection (example: if oil pressure reads 255, consider it an error)
    if (buf[5] == 255 || buf[6] == 255) {  
      sensorFailureState = true;
    } else {
      sensorFailureState = false;
    }
  }

  // Handle Flashing Alerts
  handleFlashingAlerts();

  genie.DoEvents();
  delay(100);
}

// Function to handle flashing LED indicators
void handleFlashingAlerts() {
  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;

    // Flash Shift Alert LED (Primitive Circle 1)
    if (shiftAlertState) {
      static bool shiftLedState = false;
      shiftLedState = !shiftLedState;
      genie.WriteObject(GENIE_OBJ_USER_LED, 1, shiftLedState);
    } else {
      genie.WriteObject(GENIE_OBJ_USER_LED, 1, 0);
    }

    // Flash Sensor Failure Alert LED (Primitive Circle 2)
    if (sensorFailureState) {
      static bool sensorLedState = false;
      sensorLedState = !sensorLedState;
      genie.WriteObject(GENIE_OBJ_USER_LED, 2, sensorLedState);
    } else {
      genie.WriteObject(GENIE_OBJ_USER_LED, 2, 0);
    }
  }
}
