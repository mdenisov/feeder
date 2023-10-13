![Pet Feeder](docs/intro.png)

# Pet Feeder

Smart feeder for pets. Works according to schedule. Supports Wifi and MQTT. WiFi remote based on ESP8266.

## Features
- Scheduled feeding
- Supports WiFi
- Supports MQTT
- Possibility of integration with a smart home

## Assembled device
The **STL** folder contains all the files necessary for printing.

### Reductor
Reductor designed for stepper motor **Nema 17**.
![reductor](docs/reductor.png)

### Case
Step 1
![step_1](docs/step_1.png)
Step 2
![step_2](docs/step_2.png)
Step 3
![step_3](docs/step_3.png)
Step 4
![step_4](docs/step_4.png)
Step 5
![step_5](docs/step_5.png)
Step 6
![step_6](docs/step_6.png)
Final
![final](docs/final.PNG)

## PCB

In the **PCB** folder you will find everything you need to produce a printed circuit board.

### Main components are:

- ESP8266 (ESP-12F)
- Stepper driver A4988
- MP2307DN
- Inductor 10uH
- Pair diodes SS34
- Switch (TA-03)
- Pair JST connectors
- Screw connector
- Resistors and capacitors (0805)

Final view of the printed circuit board:
![pcb](docs/Feeder_PCB_3D.png)