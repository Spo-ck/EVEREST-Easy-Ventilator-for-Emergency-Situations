# How to Installation Guide

## Raspberry Pi

The Raspberry Pi is a powerful minicomputer that is well known in all kinds of technical implementations, especially in the Internet of Things and home automation. It offers a complete Computer on a single PCB and can be online 24h a day, what makes perfect for implementation. Here, the Raspberry shall be used to display telemetry and to set parameters on the controller. To set the Pi up, first an HDMI-Display, and a USB-Mouse, as well as an USB-Keyboard are mandatory. Different to other computers, the Raspberry Pi does not offer built in memory; instead, a micro SD-Card is used as memory and and inserted into the micro SD-Port at the bottom side of the Pi. Before starting the Pi, an operating system has to be flashed on the SD- Card. While there is a variety of different operating systems available, some also especially made for the Internet of Things, it was discussed to use Raspbian, a Linux/Debian based operating system, which is also the official Raspberry Pi operating system. Therefore, it should offer the highest flexibility, and because it is open source, the software can be checked for possible security issues. To install Raspbian, it first has to be downloaded from the Raspberry Pi website under the following link:

• Operating System download: https://www.raspberrypi.org/downloads/raspbian/

For the installation of the operating system, another computer is mandatory. Before the operating system installation, the micro SD-Card has to be formatted, and the image file has to be unpacked from the downloaded Raspbian Zip-File. After that, the software that installs/flashes the operating System on the Raspberry needs to be installed. Then, the SD-Card can be inserted into the Rasberry Pis SD-Slot. After connecting the Pi to a power source, it will instantaneously boot, and show the boot window on the connected display.

## Interface / GUI: Node-Red

Node-Red offers a graphical programming environment, that enables developers to create connections between different protocols, and has a dashboard built in to display data. For this implementation, Node-Red was chosen as control and telemetry interface.
To install Node Red, the following command has to the used in a command window: • Install Node-Red: sudo npm install -g --unsafe-perm node-red
Node Red itself is a command line application, and programming and visualization can be done using any browser. To start or stop Node-Red, the following commands have to be used in a command window:

• Start Node-Red: node-red-pi

• Stop Node-Red: press Ctrl + C

Node-Red will then run in the command line window (Figure 3). It is important, that closing this window also results in quitting Node-Red, so that the window has to be always open when Node- Red should be running. Important is also, that there are two different command to start Node- Red: “node-red” and “node-red-pi”. “node-red” can be used for implementations on any hardware, but caused stability issues in the hardware demonstrator on the raspberry pi, because it was not possible to limit the memory usage adequately. For computers with limited resources,

node-red offers the “node-red-pi” command, that offers a memory optimised version of runtime, while there was no limitation in performance in the hardware demonstrator.

After starting Node-Red, it will host 2 webpages to port 1880, that can be accessed using a standard browser using the following URLs:

• User-Interface (Error! Reference source not found.): https://localhost:1880
• Dashboard: https://localhost:1880/ui

In order to use the flow, the following node extensions have to be installed. Installation can be done using the command line or directly in Node-Red

• Installation in Command Line:

    1. Stop Node-Red:                     press Ctrl + C
    2. Install Serial-Port Node:          sudo npm i node-red-node-serialport
    3. Install Dashboard Node:            sudo npm i node-red-dashboard
    
• Installation directly in Node-Red:

    1. Open command Line
    2. Start Node-Red:                    node-red-pi + enter
    3. Open Browser 
    4. Go to:                             localhost:1880
    5. Open Installation menu:            Main Menu (Top Right) → Manage Pallette → Install
    6. Search for:                        node-red-node-serialport
    7. Press Install
    8. Search for:                        node-red-dashboard
    9. Press Install

After all nodes are installed, the Dashboard Flow need to be installed. Flows are saved in a JSON file containing all settings:

• Importing a Flow into Node-Red:

    1. Open command Line
    2. Start Node-Red:                    node-red-pi + enter
    3. Open Browser
    4. Go to:                             localhost:1880
    5. Open Installation menu:
    6. Click on “Select a file to import” and navigate to the flow.json file
    7. Press Import
    
After the flow was imported, it needs to be deployed into Node-Red. To deploy a Flow, the red “Deploy” Button (Top middle-right) needs to be pressed. Afterwards, the dashboard is accessible under localhost:1880/ui.

