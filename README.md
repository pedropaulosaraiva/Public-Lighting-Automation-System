# Public Lighting Automation System

## Overview

The project illustrates the operation of an intelligent automation system for public lighting. The main purpose of the project is educational. Therefore some simplifications are made to reduce costs (mainly in the circuit assembly) so that generalization to a real implementation is not compromised.

The system is composed of three controllers for the lighting system that collect voltage, electric current, temperature data from the lamps and make decisions based on those data. In addition, a fourth microcontroller acts as the system master that can send commands to the other controllers and receives the data collected by them via a serial communication protocol.

The project uses the **ATmega328p** microcontroller — a choice intended to minimize assembly costs, given the popularity and low cost of Arduino boards — via the **SPI** protocol.

The system was simulated using **Proteus** software, and all simulations are also in the repository. C++ is used to program the ATmega328p, together with direct manipulation of some processor registers — therefore I recommend reading this project with the ATmega328p datasheet at hand to check the functions of these registers — especially to control reception of data by the slave microcontrollers via SPI, since no dedicated library was used for that.

## Slave behavior

Each slave is responsible for:

- Continuously sampling analog inputs (voltage, current, temperature) and updating local measurements.  
- Immediately reacting to safety-related events (over-range conditions and light sensor changes) using hardware interrupts.  
- Controlling the lamp output (relay driver in a real build; LED in the prototype) and reporting its state to the master.  
- Acting as an SPI slave that responds to master polls, accepts commands (e.g. `all-off`) and send measurements.

## Interrupts (slave)

The prototype uses four interrupt sources to guarantee fast reaction to hazardous conditions and control events:

1. **Comparator / Safety interrupt** (level-change): triggered when an analog comparator (or an external switch in the prototype) signals an over-range on voltage, current, or temperature. In practice three comparators are OR'ed and connected to the interrupt pin.
2. **Light sensor interrupt** (level-change): triggered by the light sensor (or prototype switch) to signal dusk/illumination changes.
3. **Slave command switch interrupt** (level-change): triggered by a local manual command on the slave.
4. **SPI communication interrupt**: used for slave reception when the master polls. The master periodically requests sensor data and slave status and may send an `all-off` command.

> Design note: interrupts ensure that switching-off occurs immediately and is not delayed by the slaves' acquisition/display loop.

## Master behavior

* The master polls **one slave at a time**, timed by a **timer interrupt**.
* On each poll the master requests: temperature, voltage, current, slave command switch state and light sensor state.
* Acquired information is shown on the master's LCD. A local button selects which slave is visible.
* A master-level global switch sends an `all-off` command to every slave.

## How it works (Full explanation)

Each slave microcontroller controls the lamp state (on/off) through a relay (omitted in the circuit assembly and replaced by an LED). Continuously, each microcontroller collects voltage, current and temperature data from the sensors present at the lamp (again omitted in the circuit assembly and replaced by three potentiometers so that their outputs reproduce the analog mapping of the sensors) via its analog input, and converts the read signal to the corresponding measured quantity. Finally, the value is written to an LCD display via the I2C protocol.

Given limit values for voltage, current and temperature, or a command via the master’s switch or the concentrator itself, or the indication from the light sensor, the lamp must turn off as quickly as possible — i.e., this should not occur at some point during the data collection and display loop. To implement this functionality, microcontroller interrupts are used; four interrupts are enabled:

* Level-change interrupt on a pin connected to a comparator circuit. The comparator circuit is composed of three analog comparators that return a logical high if the voltage, current or temperature values read by the sensors exceed the limit values; their outputs are combined by an OR gate — for the circuit assembly, these circuits were omitted in each microcontroller and replaced by a switch that can return 0 or 1 if an overrange occurs;

* Level-change interrupt on a pin connected to the light sensor — also omitted in the circuit assembly and replaced by a switch;

* Level-change interrupt on a pin connected to the slave command switch;

* SPI communication interrupt: the master periodically requests temperature, voltage and current readings, the state of the slave’s command switch and the state of the light sensor. It can also send a command that requests all slaves to turn off their lights.

Finally, the master requests data from one slave at a time, timed by the controller’s timer interrupt, via the SPI protocol. Moreover, the information acquired for each slave is displayed on an LCD, and the user can select which controller is shown using a button. As mentioned earlier, the master has a main switch that sends a command to turn off all the lights regardless of the light sensor state, etc.


## Simulation

* Simulated with **Proteus**. All Proteus project files and recorded simulations are included in the `Simulations/` folder.


## Videos / Demonstrations

The following videos illustrate the physical circuit assembly (English subtitles must be enabled):
* (Overall circuit operation)[https://www.youtube.com/watch?v=ur7O8wWbcbM]
* (Master operation)[https://youtube.com/shorts/-HkSp_af6P0]
* (Master all-off + slave switches)[https://youtube.com/shorts/ae5NhZZ41S8]
* (Slave operation)[https://youtube.com/shorts/8YufgrUaJ8M]


## Notes & Recommendations

* Because the code manipulates hardware registers directly, keep the **ATmega328P datasheet** open when reviewing the firmware.
* The prototype uses potentiometers and switches to simulate sensors and comparators to keep costs low. A real deployment should use proper analog front-ends, isolation and protective circuitry.
* For modern projects that require connectivity or higher performance consider alternatives (e.g. ESP32, STM32, or RISC-V MCUs), but the ATmega328P is a valid educational and low-cost option.

## Contributing

Contributions, bug reports and improvements are welcome. Please open issues or pull requests describing the change.
