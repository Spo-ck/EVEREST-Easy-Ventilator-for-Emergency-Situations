/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  EVEREST - Easy Ventilator for Emergency Situations 
//  Main Controller Code 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  for Heltec Wireless Stick
//  Version Main-26
//  Developer: Jan-Henrik Zünkler
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Archievements:
//
//  BME280 Sensors implemented
//  Relay Module implemented
//  Flow Calculation
//  Breathing control modes
//  Patient Monitoring
//  Advanced selfbreathing
//  publishing json strings
//  receive json commands
//  change parameters from node-red
//  print_settings
//  calculate_parameters updated to calculate pressure parameters
//  Alarms
//  Function for ending ventilation smovely
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  In Progress:
//
//  TIn and Rate still have to be calculated in calculate_parameters()
//  analog interface
//  Settings stored in EEPROM
//  ALARM
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Format of the Input String for commands
// {"Rate":-1,"IE":-1,"PEEP":-1,"PPl":-1,"Vol":-1,"Mode":"","OP":"Start","TrgP":-1,"Telm":-1,"InOp":-1,"OutOp":-1,"FiO2":-1}
// {"Rate":-1,"IE":-1,"PEEP":-1,"PPl":-1,"Vol":-1,"Mode":"","OP":"Stop","TrgP":-1,"Telm":-1,"InOp":-1,"OutOp":-1,"FiO2":-1}
      
#include "Arduino.h"
#include "heltec.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

//////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////

// BME280 Pressure Sensors
// Objects
Adafruit_BME280 bme1;
Adafruit_BME280 bmeRef;
// Variables
float Pressure_1;
float Pressure_Ref;
float Humidity_1;
float Temperature_1;
// Calculation Parameters
#define Enviromental_Pressure (996.47)
#define bmeRef_offset (-10.00)
#define bme1_offset (+0.00)

// Relay-Module
// Pins
#define Relay1 (32)                // Relay one connected to Pin 12
#define Relay2 (33)                // Relay one connected to Pin 13
#define Relay3 (13)                // Relay one connected to Pin 33
#define Relay4 (14)                // Relay one connected to Pin 32
// State
int realy1_state = 0;
int realy2_state = 0;
int realy3_state = 0;
int realy4_state = 0;

// Flow-Meter
// Connected Pins
#define Pin_Flow1 (2)              // Flow-Meter 1 at Analog Pin A1
#define Pin_Flow2 (27)             // Flow-Meter 2 at Analog Pin A2
// Hardware Parameters - ADC
const float U_In_Max = 3.3;        // Maximum Input Volatge ADC in V
const float Res_ADC = 4095;        // ADC Resolution (Arduino: 1024, ESP32: 4095)
// Hardware Parameters - Flow Sensor
const float Flow_Max = 30000;      // Maximum Flow through the Flommeter in ml/min
// Variables
// last measurement
float Flow1_last = 0;              // Flow Sensor 1
float Flow2_last = 0;              // Flow Sensor 2
unsigned long timestep1_last = 0;
unsigned long timestep2_last = 0;
// current measurement
float Flow1_curr = 0;              // Flow Sensor 1
float Flow2_curr = 0;              // Flow Sensor 2
unsigned long timestep1_curr = 0;
unsigned long timestep2_curr = 0;

//////////////////////////////////////////////
// Ventilator Operation
//////////////////////////////////////////////

float settings_rate     = 15;      // Brathing Rate in 1/min
float settings_IE       = 2;       // Breathing In time in % of one breath
float settings_vol      = 500;     // Volume in ml
float settings_PEEP     = 10;      // PEEP Preassure in mbar
float settings_PPl      = 30;      // Peak Pressur in mbar
String settings_mode    = "time"; // Volume Controlled (V) or Pressure Controlled (P)
String settings_op      = "Stop";  // Start / Stop Operation
float settings_triggerP = 10;      // Trigger Pressure - mBar
float settings_InOpen   = 25;      // %, % The In Valve Opens
float settings_OutOpen  = 25;      // %, % The Out Valve Opens
float settings_FiO2     = 21;      // %, Setting FiO2
String breathing_state  = "Stop";  // Stop / In / HoldIn / Out / HoldOut 
String last_state       = "Stop";  // Stop / In / HoldIn / Out / HoldOut 

float pressure_PEEP         = 0;   // PEEP Pressure
float pressure_mean         = 0;   // Mean Pressure
float pressure_plateau      = 0;   // Plateau Pressure
float pressure_peak         = 0;   // Peak Pressure
// Pressure Monitoring time parameters
// Peep, Mean and Peak Preassure will be calculated over 2 periods
// Thus this time parameters are used in the calculation     
int num_measurements_mean   = 0;
int num_measurements_pl     = 0;
int num_measurements_peep   = 0;

float vol_act               = 0;  // Current Volumesettings_PPeak
float vol_in                = 0;  // Breathing In Volume
float vol_out               = 0;  // Breathing Out Volume

float curr_Rate             = 0;  // Breathing Rate in 1/min
float curr_IE               = 0;  // Breathing in Time Ratio
float curr_Flow             = 0;  // Current Flow in l/min
float curr_FiO2               = -1; // Current % O2 in the Breahing in Circuit

unsigned long T_start       = 0;  // Starting time currentbreathing cycle
unsigned long T_In          = 0;  // Time Inhaling
unsigned long T_end         = 0;  // End time current breathing cycle
unsigned long T_Valve1      = 0;  // Valve 1 Open Time
unsigned long T_Valve2      = 0;  // Valve 2 Open Time

float ppeak_offset          = 20; // mbar

int num_telemetry         = 4;    // in times/s -Number of times that telemetry is printed
unsigned long last_print  = 0;    // Last millis values of when telemetry was printed last time
char command_mode         = 'C';  // Use cammands by a connected computer ("C") or analog interface ("I")

const unsigned long max_long = 4294967295; // Maximum numer of the millis function

String ALARM_MESSAGE = "";

//////////////////////////////////////////////
// Setup
//////////////////////////////////////////////

void setup() {
  //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, false /*Heltec.LoRa Disable*/, true /*Serial Enable*/, false /*PABOOST Enable*/);
  // Display Settings
  Heltec.display->setContrast(255);

  //Initialise Serial Cummunication with a computer
  Serial.begin(115200);
  Serial.println();

  // Initialize I2C Buses
  Wire.begin();
//  Wire1.begin();

  //Set Pinmodes
  initialize_pins();
  
  //Initalize Sensors
  initialize_sensors();

  // Initialize Relay
  initialize_relay();
}

//////////////////////////////////////////////
// Resetting
//////////////////////////////////////////////

// Resets time parameters after 50days
// everything else yould result in crashes without a daubt
void reset_time_parameters() {

  if (last_print > (60000 + millis())) {
    last_print = last_print - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }
  
  if (timestep1_last > (60000 + millis())) {
    timestep1_last = timestep1_last - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }
  
  if (timestep2_last > (60000 + millis())) {
    timestep2_last = timestep2_last - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }
  
  if (timestep1_curr > (60000 + millis())) {
    timestep1_curr = timestep1_curr - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }
  
  if (timestep2_curr > (60000 + millis())) {
    timestep2_curr = timestep2_curr - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }
  
  if (T_In > (60000 + millis())) {
    T_In = T_In - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }

  if (T_end > (60000 + millis())) {
    T_end = T_end - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }

  if (T_Valve1 > (60000 + millis())) {
    T_Valve1 = T_Valve1 - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }

  if (T_Valve2 > (60000 + millis())) {
    T_Valve2 = T_Valve2 - max_long; // Thus the millis funtion is restarting, also the parameter has to be resetted
  }
}

//////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////

// Set Pinmodes
void initialize_pins() {
  // Relay Pins
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);

  // Flow Meter Pins
  pinMode(Pin_Flow1, INPUT);
  pinMode(Pin_Flow2, INPUT);
}

void initialize_sensors() {
  
  // Initializing the BME280 Sensors
  
//  // BME280-0 (Wire Bus, SDO Connected to VCC)
//  Serial.print("Initailizing BME280-0...");
//  while(!bme0.begin(0x77, &Wire)) {
//  //    Serial.print(".");
//  }
//  Serial.println("Intialization Complete!");
  
  // BME280-1 (Wire Bus, SDO Connected to GND)
  Serial.print("Initailizing BME280-1...");
  while(!bme1.begin(0x76, &Wire)) {
  //    Serial.print(".");
  }
  Serial.println("Intialization Complete!");

  // BME280-Ref (Wire Bus)
  Serial.print("Initailizing BME280-Ref...");
  while(!bmeRef.begin(0x77, &Wire)) {
//    Serial.print(".");
  }
  Serial.println("Intialization Complete!");
  Serial.println();
}

// Initialize Relay Module
void initialize_relay() {
  // Set Relay Pins to LOW
  digitalWrite(Relay1, HIGH); // High means Relay x off
  digitalWrite(Relay2, HIGH); // High means Relay x off
  digitalWrite(Relay3, HIGH); // High means Relay x off
  digitalWrite(Relay4, HIGH); // High means Relay x off
}

//////////////////////////////////////////////
// DISPLAY
//////////////////////////////////////////////

// Function that prings a message consisting of two strings for two lindes to the display of the sensor package
void display_message(String string1, String string2) {
  Heltec.display->clear();
  Heltec.display->setLogBuffer(5, 30);

  // Print to the screen
  Heltec.display->println(string1);
  Heltec.display->println(string2);

  // Draw it to the internal screen buffer
  Heltec.display->drawLogBuffer(0, 0);

  // Display it on the screen
  Heltec.display->display();
}

//////////////////////////////////////////////
// Measurements
//////////////////////////////////////////////

// Function that manages all sensors
void manage_sensors() {
  
  // Function for Aquiring Sensor Readings
  read_pressuresensors();
  read_flowsensors();

  // Function to Calculate Ventilator Parameters
  // Function to Calculate Volume and Petssure Parameters
  // As well as secondary parameters
  calculate_parameters();
  
}

// Funtion that take the actual Measurements from the connected Sensors
void read_pressuresensors() {
//  // Reference Value / Preassure outside the System
//  Pressure_Ref = Enviromental_Pressure;
//    Pressure_Ref = (bmeRef.readPressure() / 100) + bmeRef_offset;

  // Pressure before the first Valve
  Pressure_Ref = (bmeRef.readPressure() / 100) + bmeRef_offset;
  
  // Pressure , Humindity and Temperature_1 before the Patient
  Pressure_1 = (bme1.readPressure() / 100) + bme1_offset - Pressure_Ref;
  Humidity_1 = bme1.readHumidity();
  Temperature_1 = bme1.readTemperature();
}

// Reads current Flow Value in l/min
void read_flowsensors() {
  // Store last readings
  Flow1_last = Flow1_curr;
  Flow2_last = Flow2_curr;
  timestep1_last = timestep1_curr;
  timestep2_last = timestep2_curr;

  // Read Analog Voltages
  Flow1_curr = analogRead(Pin_Flow1);
  timestep1_curr = millis();
  Flow2_curr = analogRead(Pin_Flow2);
  timestep2_curr = millis();
  
  // Map Readings to the actual Voltage Level 
  Flow1_curr = (U_In_Max/Res_ADC) * Flow1_curr;
  Flow2_curr = (U_In_Max/Res_ADC) * Flow2_curr;
  
  // Map Voltage to Flow
  // 3.3V equals 30l/min with an linear analog Flow Meter
  Flow1_curr = (Flow_Max/U_In_Max) * Flow1_curr;
  Flow2_curr = (Flow_Max/U_In_Max) * Flow2_curr;

  curr_Flow = Flow1_curr - Flow2_curr;

}

// calculates the volume of air in the patients lunghs in l
void calculate_parameters() {
  /////////////////////
  // Volume Calculation
  /////////////////////
  
  // Calculate the mean flow in the last Period
  float Flow1_temp = (Flow1_curr + Flow1_last) / 2;
  float Flow2_temp = (Flow2_curr + Flow2_last) / 2;
  
  //Calculte the Volume flow through each sensor in this period

  Flow1_temp = Flow1_temp * (float(timestep1_curr - timestep1_last) / 60000); // Inflowing Volume
  Flow2_temp = Flow2_temp * (float(timestep2_curr - timestep2_last) / 60000); // Outflowing Volume
  
  vol_in = vol_in + Flow1_temp;
  vol_out = vol_out + Flow2_temp;

  // Calculate the Volume of air in the Patients lungh
  vol_act = vol_act + Flow1_temp - Flow2_temp; // Vol out Air in the Patients lungh (= Inflown Vol - Outflown Vol)


  ///////////////////////
  // Pressure Calculation
  ///////////////////////

  // Calculating Mean Values for Pressure

  // Mean Pressure
  num_measurements_mean = num_measurements_mean + 1;

  if (num_measurements_mean > 10000) {
    num_measurements_mean = 1;
  }
  
  if (num_measurements_mean == 1 ) {
    pressure_mean = Pressure_1;
  }
  else {
    pressure_mean = (pressure_mean * (num_measurements_mean - 1) + Pressure_1) / num_measurements_mean;
  }

  // Peak Pressure
  if (pressure_peak < Pressure_1) {
      pressure_peak = Pressure_1;
  }

  // PEEP Pressure
  if (breathing_state == "HoldOut") {
    num_measurements_peep = num_measurements_peep + 1;
    
    pressure_PEEP = (pressure_PEEP  * (num_measurements_peep - 1) + Pressure_1) / num_measurements_peep;
  }
  else if (breathing_state != "HoldOut" && num_measurements_peep != 0) {
    num_measurements_peep = 0;
  }

  // Plateau Pressure
  if (breathing_state == "HoldIn") {
    num_measurements_pl = num_measurements_pl + 1;
    
    pressure_plateau = (pressure_plateau * (num_measurements_pl - 1) + Pressure_1) / num_measurements_pl;
  }
  else if (breathing_state != "HoldIn" && num_measurements_pl != 0) {
    num_measurements_pl = 0;
  }
}


//////////////////////////////////////////////
// Communication
//////////////////////////////////////////////

// Telemetry to Computer
void print_telemetry() {

  // Generationg the JSON Doument with the Data to be send
  StaticJsonDocument<512> telemetry_object;

  // Fill the Object with Data

  telemetry_object["P"]["Act"]          = Pressure_1;
  telemetry_object["P"]["PEEP"]         = pressure_PEEP;
  telemetry_object["P"]["Mean"]         = pressure_mean;
  telemetry_object["P"]["Pl"]           = pressure_plateau;
  telemetry_object["P"]["Peak"]         = pressure_peak;

  telemetry_object["Flow"]              = curr_Flow;
  telemetry_object["Rate"]              = curr_Rate;
  telemetry_object["IE"]                = curr_IE;
  telemetry_object["FiO2"]              = curr_FiO2;
  
  telemetry_object["Vol"]["Act"]        = vol_act;
  telemetry_object["Vol"]["In"]         = vol_in;
  telemetry_object["Vol"]["Out"]        = vol_out;
  
  // Generate Variable for telemetry transmission string
  String telemetry = "";

  // Serialize the JSON object and store the JSON Sting in the string variable
  serializeJson(telemetry_object, telemetry);
  
  // Print the Telemetry
  Serial.println(telemetry);
}

// Prints all Settings to Serial 
// Called on startup and after setting changes
void print_settings() {

  // Generationg the JSON Doument with the Data to be send
  StaticJsonDocument<512> settings_object;

  // Fill the Object with Data
  settings_object["Settings"]["Rate"]  = settings_rate;
  settings_object["Settings"]["IE"]    = settings_IE;
  settings_object["Settings"]["Vol"]   = settings_vol;
  settings_object["Settings"]["PEEP"]  = settings_PEEP;
  settings_object["Settings"]["PPl"]   = settings_PPl;
  settings_object["Settings"]["Mode"]  = settings_mode;
  settings_object["Settings"]["OP"]    = settings_op;
  settings_object["Settings"]["TrgP"]  = settings_triggerP;
  settings_object["Settings"]["FiO2"]  = settings_triggerP;

  // Generate Variable for telemetry transmission string
  String settings_string = "";

  // Serialize the JSON object and store the JSON Sting in the string variable
  serializeJson(settings_object, settings_string);
  
  // Print the Telemetry
  Serial.println(settings_string);
  
}

// Receive Commands from the Interface
void receive_commands() {
  if (command_mode == 'C' ) { // receive cammands from a computer

    // Variable for command from seriel monitor
    String com = "";
  
    // Read every char from the seriel input
    while (Serial.available()) {
      com = com + String(char(Serial.read()));
    }

    // If the Serial message is not empty, extract the command
    if (com != "") {
      
      // Generate a JSON Object
      StaticJsonDocument<512> command_obj;
      
      // Convert the imput Sting into an object
      deserializeJson(command_obj, com);

      // Extract the content from the object
      float  temp_Rate      = command_obj["Rate"].as<float>();
      float  temp_IE        = command_obj["IE"].as<float>();
      float  temp_PEEP      = command_obj["PEEP"].as<float>();
      float  temp_PPl       = command_obj["PPl"].as<float>();
      float  temp_Vol       = command_obj["Vt"].as<float>();
      String temp_Mode      = command_obj["Mode"].as<String>();
      String temp_OP        = command_obj["OP"].as<String>();
      float  temp_TrgP      = command_obj["TrgP"].as<float>();
      int    temp_telemetry = command_obj["Telm"].as<int>();
      float  temp_InOpen    = command_obj["InOp"].as<float>();
      float  temp_OutOpen   = command_obj["OutOp"].as<float>();
      float  temp_FiO2      = command_obj["FiO2"].as<float>();
      
      // Overwrite the command variables
      // Only one parameter will be changed per command
      // Parameters that are included in a command are set to -1 for numbers and "" for strings
      // only one parameter per command is different from these predefined values
      // if to find the one tht is different
      // After a settings change, the new settings are printed as update confirmation
      if (temp_OP != "") {
        settings_op = temp_OP;
        print_settings();
      } 
      if (temp_Rate != -1) {
        settings_rate = temp_Rate;
      }
      else if (temp_IE != -1) {
        settings_IE = temp_IE;
      }
      else if (temp_PEEP != -1) {
       settings_PEEP = temp_PEEP;
      }
      else if (temp_PPl != -1) {
        settings_PPl = temp_PPl;
      }
      else if (temp_Vol != -1) {
        settings_vol = temp_Vol;
      }
      else if (temp_Mode != "") {
        settings_mode = temp_Mode;
      }
      else if (temp_TrgP != -1) {
        settings_triggerP = temp_TrgP;
      }
      else if (temp_telemetry != -1) {
        num_telemetry = temp_telemetry;
      }
      else if (temp_InOpen != -1) {
        settings_InOpen = temp_InOpen;
      }
      else if (temp_OutOpen != -1) {
        settings_OutOpen = temp_OutOpen;
      }
      else if (temp_FiO2 != -1) {
        settings_FiO2 = temp_FiO2;
      }

      // Print Settings
      print_settings();
    } 
  }
  else if (command_mode == 'I' ) { // receive cammands from an analog interface
    
  }
}

//////////////////////////////////////////////
// Ventilator Operation
//////////////////////////////////////////////

void ventilator_operation() {
  if (breathing_state == "In") {
    // Open Valve 1 at beginning of state 1
    if (last_state != "In") {

      // Reset v_in after a period
      vol_in = 0;

      // Time Parameter Calculation
      // Calculate curr_Rate
      if ((T_In - T_start) != 0) {
        curr_Rate = 60000 / (millis() - T_start);
      }
      else {
        curr_Rate = 0;
      }
      // Calculate turrent I/E
      if ((T_In - T_start) != 0) {
        curr_IE = (millis() - T_In) / (T_In - T_start);
      }
      else {
        curr_IE = 0;
      }

      // Ventilation      
      // open valve 1
      digitalWrite(Relay1, LOW);

      // Calculate breathing time
      T_start = millis();
      
      T_end = ((60 / settings_rate) * 1000);
      T_In  = T_start + T_end * (1 / (settings_IE  + 1));
      
      T_Valve1 = T_start + T_end * (1 / (settings_IE  + 1)) * (settings_InOpen / 100);
      T_Valve2 = T_In + T_end * (settings_OutOpen / 100);
      
      T_end = T_start + T_end;
      
      // Thus many funtions are time controlled by millis()
      // the controlling parameters have to be resetted after 50 days
      reset_time_parameters();

      // Overwrite Last State
      last_state = "In";
    }

    // Select Mode
    if (settings_mode == "CMV-V" || settings_mode == "IMV-V") {
      // Monitor Volume
      if (vol_act >= settings_vol) {
        // Close Valve 1
        digitalWrite(Relay1, HIGH);
  
        // Overwrite Breathing State
        breathing_state = "HoldIn";
      }
    }
    else if (settings_mode == "CMV-P" || settings_mode == "IMV-P" || settings_mode == "CSV") {
      // Monitor pressure
      if (Pressure_1 >= settings_PPl) {
        // Close Valve 1
        digitalWrite(Relay1, HIGH);
  
        // Overwrite Breathing State
        breathing_state = "HoldIn";
      }
    }
    else if (settings_mode == "time") {
      // Monitor Time
      if (millis() >= T_Valve1) {
        // Close Valve 1
        digitalWrite(Relay1, HIGH);
  
        // Overwrite Breathing State
        breathing_state = "HoldIn";
      }
    }
  }
  else if (breathing_state == "HoldIn") {
    
    if (last_state != "HoldIn") {
      // Overwrite Last State
      last_state = "HoldIn";
    }

    // Select Mode
    if (settings_mode == "CMV-P" || settings_mode == "CMV-V") {
      // Wait for time to exhaust
      if (millis() >= T_In) {
        // Overwrite Breathing State
        breathing_state = "Out";
      }
    }
    else if (settings_mode == "IMV-P" || settings_mode == "IMV-V") {
      // Wait for time to exhaust
      if (millis() >= T_In || Pressure_1 >= (settings_PPl + settings_triggerP)) {
        // Overwrite Breathing State
        breathing_state = "Out";
      }
    }
    else if (settings_mode == "CSV") {
      // Wait for time to exhaust
      if (Pressure_1 >= (settings_PPl + settings_triggerP)) {
        // Overwrite Breathing State
        breathing_state = "Out";
      }
    }
    else if (settings_mode == "time") {
      // Wait for time to exhaust
      if (millis() >= T_In) {
        // Overwrite Breathing State
        breathing_state = "Out";
      }
    }
  }
  else if (breathing_state == "Out") {
    
    if (last_state != "Out") {

      // reset T_In for curr_I/E calculation
      T_In = millis();
      
      // Reset v_out after a period
      vol_out = 0;
      
      // Open Valve 2
      digitalWrite(Relay2, LOW);
      
      // Overwrite Last State
      last_state = "Out";
    }

    // Select Mode
    if (settings_mode == "CMV-V" || settings_mode == "CMV-P" || settings_mode == "IMV-V" || settings_mode == "IMV-P" || settings_mode == "CSV") {
      // Monitor Volume
       if (Pressure_1 <= settings_PEEP) {
        // Close Valve 2
        digitalWrite(Relay2, HIGH);
  
        // Overwrite Breathing State
        breathing_state = "HoldOut";
      }
    }
    else if (settings_mode == "time") {
      
      // Monitor Time
      if (millis() >= T_Valve2) {
        // Close Valve 1
        digitalWrite(Relay2, HIGH);
  
        // Overwrite Breathing State
        breathing_state = "HoldOut";
      }
    }
  }
  else if (breathing_state == "HoldOut") {
    
    if (last_state != "HoldOut") {
      // Overwrite Last State
      last_state = "HoldOut";
    }
    
    // Select Mode
    if (settings_mode == "CMV-P" || settings_mode == "CMV-V") {
      // Wait for time to exhaust
      if (millis() >= T_end) {
        // Overwrite Breathing State
        breathing_state = "In";
      }
    }
    else if (settings_mode == "IMV-P" || settings_mode == "IMV-V") {
      // Wait for time to exhaust
      if (millis() >= T_In || Pressure_1 <= (settings_PEEP - settings_triggerP)) {
        // Overwrite Breathing State
        breathing_state = "Out";
      }
    }
    else if (settings_mode == "CSV") {
      // Wait for time to exhaust
      if (Pressure_1 <= (settings_PEEP - settings_triggerP)) {
        // Overwrite Breathing State
        breathing_state = "Out";
      }
    }
    else if (settings_mode == "time") {
      // Wait for time to exhaust
      if (millis() >= T_end) {
        // Overwrite Breathing State
        breathing_state = "In";
      }
    }
  }
}

void monitor_selfbreathing() {
//  if (breathing_state == "HoldIn") { // Monitors and handels selfbreathing in HoldIn State
//
//    if (Pressure_1 >= (settings_PPl + settings_triggerP)) { // If the Patient wants to breath out himself, the preassure value will increase - jump to next state "Out"
//      if (ALARM_MESSAGE !="" ) {
//        ALARM_MESSAGE = "Selfbreathing";
//      }
//
//      // Open Valve 2
//      digitalWrite(Relay2, LOW);
//      
//      // Wait until Patient finished Breathing
//      while (Pressure_1 >= (pressure_peak + settings_triggerP)) {
//        // Measure wile waiting
//        // Function that takes measurements and calculates all parameters
//        manage_sensors();
//
//        display_message(breathing_state + " - " + String("Breathing"), String(vol_in) + " / " + String(vol_out));
//      }
//
//      // Close Valve 2
//      digitalWrite(Relay2, HIGH);
//
//    }
//    else if (Pressure_1 < (settings_PPl - settings_triggerP)) { // if a Patient want to breath in more, the pressure will drop
//
//      if (ALARM_MESSAGE !="" ) {
//        ALARM_MESSAGE = "Selfbreathing";
//      }
//
//      // Open Valve 1
//      digitalWrite(Relay1, LOW);
//      
//      // Wait until Patient finished Breathing
//      while (Pressure_1 <= (pressure_peak)) {
//        // Measure wile waiting
//        // Function that takes measurements and calculates all parameters
//        manage_sensors();
//
//        display_message(breathing_state + " - " + String("Breathing"), String(vol_in) + " / " + String(vol_out));
//      }
//
//      // Close Valve 1
//      digitalWrite(Relay1, HIGH);
//    }
//  }
//  else  if (breathing_state == "HoldOut") { // Monitors and handels selfbreathing in HoldOut State
//    
//    if (Pressure_1 <= (settings_PEEP - settings_triggerP)) {// If the Patient wants to breath in himself, the preassure value will drop - jump to next step "In"
//
//      if (ALARM_MESSAGE !="" ) {
//        ALARM_MESSAGE = "Selfbreathing";
//      }
//
//      // Open Valve 1
//      digitalWrite(Relay1, LOW);
//      
//      // Wait until Patient finished Breathing
//      while (Pressure_1 <= (settings_PEEP - settings_triggerP)) {
//        // Measure wile waiting
//        // Function that takes measurements and calculates all parameters
//        manage_sensors();
//
//        display_message(breathing_state + " - " + String("Breathing"), String(vol_in) + " / " + String(vol_out));
//      }
//
//      // Close Valve 1
//      digitalWrite(Relay1, HIGH);
//
//    }
//    else if (Pressure_1 > (settings_PEEP + settings_triggerP)) { // if a Patient want to breath out more, the pressure will increase
//
//      if (ALARM_MESSAGE !="" ) {
//        ALARM_MESSAGE = "Selfbreathing";
//      }
//
//      // Open Valve 2
//      digitalWrite(Relay2, LOW);
//      
//      // Wait until Patient finished Breathing
//      while (Pressure_1 >= (pressure_PEEP)) {
//        // Measure wile waiting
//        // Function that takes measurements and calculates all parameters
//        manage_sensors();
//
//        display_message(breathing_state + " - " + String("Breathing"), String(vol_in) + " / " + String(vol_out));
//      }
//
//      // Close Valve 2
//      digitalWrite(Relay2, HIGH);
//    }
//  }
}

void safety() {
//  // If the pressure exceeds a safe range, open valve 2
//  if (Pressure_1 >= (settings_PPl + ppeak_offset)) {
//
//      if (ALARM_MESSAGE !="" ) {
//        ALARM_MESSAGE = "OVER";
//      }
//
//    // Open Valve 2
//      digitalWrite(Relay2, LOW);
//      
//      // Wait until Patient finished Breathing
//      while (Pressure_1 >= (pressure_PEEP)) {
//        // Measure wile waiting
//        // Function that takes measurements and calculates all parameters
//        manage_sensors();
//
//        display_message(breathing_state + " - " + String("Safety"), String(vol_in) + " / " + String(vol_out));
//      }
//
//      // Close Valve 2
//      digitalWrite(Relay2, HIGH);
//  }
}
//////////////////////////////////////////////
// Main Loop
//////////////////////////////////////////////

void loop() {

  // Thus many funtions are time controlled by millis()
  // the controlling parameters have to be resetted after 50 days
  reset_time_parameters();
      
  // Receive Commands from if they are available
  receive_commands();
  
  if (settings_op == "Start" || breathing_state != "Stop") {

    // Function that takes measurements and calculates all parameters
    manage_sensors();

    // Monitor if the Pressur exceeds a safe range and open Valve 2 if not!!!
    safety();

    // Display Vol on the display
    display_message(breathing_state + " - " + String(vol_act), String(vol_in) + " / " + String(vol_out));

    
    // Print the Measurements to USB n times per second
    if (millis() >= (last_print + (1000 / num_telemetry))) {
      print_telemetry();      // Generate a JSON String and print it to USB
  
      last_print = millis();  // Update the triggering Variable
    }
  
    // To start ventilation, the breathing state has to be changed from stop to In   
    if (breathing_state == "Stop") {

      // Thus In is the first phase and it will trigger the ventilator functions
      // While Stop as state would stop them
      breathing_state = "In";

      // Time Parameters to calculate peep, mean and peak preassure over two periods
      num_measurements_mean  = 0;
      pressure_mean          = 0;
      pressure_peak          = 0;
      pressure_PEEP          = 0;
      pressure_plateau       = 0;

      // At the beginning of ventilation, the settigs will be printed to give an overview of the ventilator settings
      print_settings();
    }
    
    // Control of the ventilator actuators - actual ventilation!!!!
    // Function that controls operation
    ventilator_operation();

    // Monitor if the Patien is breathing himself and set in and out stat according to settings_triggerP
    monitor_selfbreathing();
  }

  // Settings Mode Changed to stop
  // Wait for the current breathing Period to end
  // Reset breathing state controlling parameterts to get into Stop mode
  if (settings_op == "Stop" && breathing_state == "In" && last_state == "HoldOut") {
    if (breathing_state != "Stop") {
      breathing_state = "Stop";
    }
    
    if (last_state != "Stop") {
      last_state = "Stop";
    }
  }

//// ALARMS
//
//  if (ALARM_MESSAGE !="" ) {
//    if (ALARM_MESSAGE == "Selfbreathing") {
//      Serial.println("{\"Alarm\":\"Selfbreathing\”}");
//      ALARM_MESSAGE = "";
//    }
//    else if (ALARM_MESSAGE == "OVER") {
//      Serial.println(""{\"Alarm\":\"Overpressure\”}"");
//      ALARM_MESSAGE = "";
//    }
//  }
  

}
