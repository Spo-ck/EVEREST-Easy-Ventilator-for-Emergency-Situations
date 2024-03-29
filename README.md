# EVEREST - Easy Ventilator for Emergency Situations

Welcome to the EVEREST - Easy Ventilator for Emergency Use. EVEREST was developed by Levente Türk and Jan-H. Zünkler as a pragmatic DIY solution during the first COVID-19 Wave in Europe. On the software and hardware Level it was designed to be as generic as possible, so that other projects can use EVEREST as a starting point for their development. Later, the Project participated at the [EUvsVirus Hackathon](https://devpost.com/software/everest-easy-ventilator-for-emergency-situations). After the Hackathon, our Team joined [Polyvent](https://covid-eic.easme-web.eu/solution/polyvent-global-ventilator-design-formula/about), one of the Hackathon winners, and assisted the Team with the implementation EVEREST subsystems in the Polyvent Ventilator Prototype in the [Innovation Week in Linz](https://ooe.orf.at/stories/3071365/). 
This documentation wil be enhanced in the upcomming months into a tutorial on how to built a ventilator. 

On the IT Side, Everest consists out of a Microcontroller (ESP32) that controlls the Ventilation, and a Raspberry Pi that is hosting the GUI. The Gui is hosted in the Open Source Application Node-Red that has to be installed on the Rasperry Pi. The Raspberry Pis is then connected to the Microcontroller via USB. The Node-Red GUI can be used with a Touch display, that is communicating with the Microcontroller. The COntroller itself is then controlling the ventilation based on the settings from the GUI and the Sensor Readings. The Controller itself has to be flashed using the Arduino IDE.

![image](https://github.com/Spo-ck/EVEREST-Easy-Ventilator-for-Emergency-SItuations/blob/master/Raspberry%20Pi%20User%20Interface/EVREST%20GUI%20V15_2.png?raw=true)
***Raspberry Pi Touch Interface***

![image](https://github.com/Spo-ck/EVEREST-Easy-Ventilator-for-Emergency-SItuations/blob/master/Schematics/Schematics.png?raw=true)
***Electronic Schematic***
