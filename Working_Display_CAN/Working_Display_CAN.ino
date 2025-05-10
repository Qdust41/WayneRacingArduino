#include <SPI.h>
#include <SD.h>
#include <mcp2515.h>
#include <genieArduino.h>

const int chipSelect = 53;
struct can_frame canMsg;
MCP2515 mcp2515(10);
 
//Set Up for display and Arduino
#define DISP_TX_PIN   18    //TX1 ON MEGA
#define DISP_RX_PIN   19    //RX1 ON MEGA
#define DISP_RST_PIN   4    //D4 ON MEGA (RESET PIN)
#define DISP_BAUD    115200   //BAUD RATE OF DISPLAY
#define DISP_SERIAL  Serial1

//Index for Display Elements
#define IDX_COOL_IN       0 
#define IDX_COOL_OUT      1
#define IDX_FUEL_PRESSURE 2
#define IDX_OIL_TEMP      3  
#define IDX_OIL_PRESSURE  4 
#define IDX_GEAR          5
#define IDX_RPM           6

//Can IDs For Data
#define CAN_IDX_RPM         0x123
#define CAN_IDX_GEAR        0x000
#define CAN_IDX_COOL_IN     0x001
#define CAN_IDX_COOL_OUT    0x002
#define CAN_IDX_FUEL_PRESS  0x003
#define CAN_IDX_OIL_TEMP    0x004
#define CAN_IDX_OIL_PRESS   0x005



//For Flashing Alerts
int stateShiftAlert = 0;
int stateSensorAlert = 0;

bool shiftAlertState = false;
bool sensorFailureState = false;

unsigned long lastShiftBlinkTimeOff = 0;
const int blinkIntervalOff = 200;
unsigned long lastShiftBlinkTimeOn = 0;
const int blinkIntervalOn = 100;

unsigned long lastSensorBlinkTimeOff = 0;
unsigned long lastSensorBlinkTimeOn = 0;


//last value for sensors
int rpm = 0;
int lastCoolIn = 0;
int lastCoolOut = 0;
int lastFuelPress = 0;
int lastOilTemp = 0;
int lastOilPress = 0;

Genie genie;
 
void setup() {
  //reset
  pinMode(DISP_RST_PIN, OUTPUT);
  digitalWrite(DISP_RST_PIN, LOW);
  delay(50);
  digitalWrite(DISP_RST_PIN, HIGH);
  delay(5000);
 
  //Baud Rate Setup For Everything
  DISP_SERIAL.begin(DISP_BAUD);
  Serial.begin(115200);
  genie.Begin(DISP_SERIAL);

  //Pin Set Up for SD card and CAN Bus
  pinMode(10, OUTPUT);
  pinMode(53, OUTPUT);
  digitalWrite(10, HIGH);
  digitalWrite(53, HIGH);

  //CAN Set Up
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

}
 
void loop() {
    // put your main code here, to run repeatedly:
  digitalWrite(10, LOW);
  int total_data = 0;
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.println(canMsg.can_id);
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      total_data += int(canMsg.data[i]);
    }   
  }
  total_data /= canMsg.can_dlc;
  
  if(canMsg.can_id == CAN_IDX_RPM){
    rpm = total_data;
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_RPM, rpm);
  }

  if(rpm >= 11000){
    shiftAlertState = true;
  }else{
    shiftAlertState = false;
  }

  //Gear Update
  if(canMsg.can_id == CAN_IDX_GEAR){
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_GEAR, total_data);
  }
  
  //Coolant In Update
  if(canMsg.can_id == CAN_IDX_COOL_IN && lastCoolIn != total_data){
    lastCoolIn = total_data;
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_IN, lastCoolIn);
  }

  //Coolant Out Update
  if(canMsg.can_id == CAN_IDX_COOL_OUT && lastCoolOut != total_data){
    lastCoolOut = total_data;
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_COOL_OUT, lastCoolOut);
  }

  //Fuel Pressure Update
  if(canMsg.can_id == CAN_IDX_FUEL_PRESS && lastFuelPress != total_data){
    lastFuelPress = total_data;
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_FUEL_PRESSURE, lastFuelPress);
  }

  //Oil Temp Update
  if(canMsg.can_id == CAN_IDX_OIL_TEMP && lastOilTemp != total_data){
    lastOilTemp = total_data;
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_TEMP, lastOilTemp);
  }

  //Oil Press Update
  if(canMsg.can_id == CAN_IDX_OIL_PRESS && lastOilPress != total_data){
    lastOilPress = total_data;
    genie.WriteObject(GENIE_OBJ_LED_DIGITS, IDX_OIL_PRESSURE, lastOilPress);
  }
  
  //Gear Shift Alert
  if(shiftAlertState == true){
    if(millis() - lastShiftBlinkTimeOn >= blinkIntervalOn){
      lastShiftBlinkTimeOn = millis();
      stateShiftAlert = 1;
      genie.WriteObject(GENIE_OBJ_ILED, 0, stateShiftAlert);
    }
    if(millis() - lastShiftBlinkTimeOff >= blinkIntervalOff){
      lastShiftBlinkTimeOff = millis();
      stateShiftAlert = 0;
      genie.WriteObject(GENIE_OBJ_ILED, 0, stateShiftAlert);
    }
  }else if(stateShiftAlert != 0){
    stateShiftAlert = 0;
    genie.WriteObject(GENIE_OBJ_ILED, 0, stateShiftAlert);
  }

  //Sensor Failure
  if(lastFuelPress >= 100 || lastOilPress >= 100 || lastOilPress <= 0){
    sensorFailureState = true;
  }else{
    sensorFailureState = false;
  }

  if(sensorFailureState == true){
    if(millis() - lastSensorBlinkTimeOn >= blinkIntervalOn){
      lastSensorBlinkTimeOn = millis();
      stateSensorAlert = 1;
      genie.WriteObject(GENIE_OBJ_ILED, 1, stateSensorAlert);
    }
    if(millis() - lastSensorBlinkTimeOff >= blinkIntervalOff){
      lastSensorBlinkTimeOff = millis();
      stateSensorAlert = 0;
      genie.WriteObject(GENIE_OBJ_ILED, 1, stateSensorAlert);
    }
  }else if(stateSensorAlert != 0){
    stateSensorAlert = 0;
    genie.WriteObject(GENIE_OBJ_ILED, 1, stateSensorAlert);
  }

  genie.DoEvents();
  digitalWrite(10, HIGH);
}

 