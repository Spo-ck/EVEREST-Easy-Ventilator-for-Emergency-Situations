# Main Controller Code

The Everest Main Controller Code was developed using the Arduino IDE. Because it was available from previous Projects, a Wireless Stick Dev Board from Heltec was used, but also all ESP32 Dev Boards can be used for an EVEREST implementation. In order to use or develop the code further, it is sufficient to download the Microcontroller-COde file in this folder and open it in the [Arduino IDE](https://www.arduino.cc/en/main/software). In order programm the microcontroller, also the driver for you Dev Board need to be installed. For installation please have a look into [How To](https://github.com/Spo-ck/EVEREST-Easy-Ventilator-for-Emergency-SItuations/tree/master/How%20to).
In the following, all elements of the microcontroller code will be explained:

## Setup Loop

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
