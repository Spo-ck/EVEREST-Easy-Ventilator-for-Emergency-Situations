# Main Controller Code

The Everest Main Controller Code was developed using the Arduino IDE. Because it was available from previous Projects, a Wireless Stick Dev Board from Heltec was used, but also all ESP32 Dev Boards can be used for an EVEREST implementation. In order to use or develop the code further, it is sufficient to download the Microcontroller-COde file in this folder and open it in the [Arduino IDE](https://www.arduino.cc/en/main/software). In order programm the microcontroller, also the driver for you Dev Board need to be installed.

In the following, all elements of the microcontroller code will be explained:

## How to Installation Guide: Microcontroller

All microcontrollers that shall be utilizes in this project have in common, that they can be programmed using the Arduino IDE software. In order install the software, the install packages for MacOS, Windows and Linux can be downloaded from the following website:

• [Arduino IDE](https://www.arduino.cc/en/Main/Software)

Since drivers for Arduino Boards are included, these microcontrollers can be programmed directly after installation of the IDE. For the ESP32 based microcontrollers. For the ESP32 and the Heltec Wireless Stick, additional drivers have to be installed:

• Driver Installation:

    1. Open: Arduino IDE
    2. Go to: Tools → Board → Boards Manager
    3. Download and Install drivers:
    
        1. Heltec ESP32 Serial Dev-Boards by Heltec Automation(TM)
        2. esp32 by Espressif Systems
      
In the next step, also additional libraries need to be installed. Since the Sensor will use the I2C bus for communication with the microcontroller, a library needs to be installed. For the Heltec Wireless Stick, also the Heltec library need to be installed, and for the ESP32, the ESP32 library needs to be installed.
After the download, the libraries need to be installed with the following steps in the Arduino IDE:
 
• Library Installation:

    1. Download Libraries:
    
        1. [Heltec Wireless Stick](https://github.com/HelTecAutomation/Heltec_ESP32)
        2. [ESP32](https://github.com/espressif/arduino-esp32)
        3. [Arduino JSON](https://arduinojson.org)
        4. [Pressure Sensor: BMP280](https://github.com/adafruit/Adafruit_BMP280_Library)
        
    2. Open: Arduino IDE
    3. Go to: Sketch → Include Library → Add .ZIP Library
    4. Select the specific library and install it
    
The control code for the Main controller can be opened in the Arduino IDE after Installation, and then be flashed on the microcontroller. After that, it has to be connected to the raspberry pi. Using the software Node-Red, commands, including start/stop operation can be send to the controller via USB connection.
