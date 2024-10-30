# AFE4950 Library

This library enables an Arduino-compatible board (such as the ESP32) to interface with the AFE4950 module, a specialized sensor designed for acquiring physiological signals like ECG (Electrocardiogram) and PPG (Photoplethysmogram). The library offers a robust API for configuring and controlling the AFE4950, allowing for seamless data acquisition and integration with projects focused on vital signs monitoring.

The AFE4950 Module is designed with a single-channel ECG and PPG configuration and includes support for multiple sample rates and FIFO (First In, First Out) management. With this library, developers can set up efficient and synchronized acquisition of physiological data to be processed or displayed in real time.

To use this library:

```cpp
#include <AFE4950.h>
```
## Circuit

The AFE4950 Module requires connections for power, ground, and SPI communication, in addition to specific wiring for ECG and PPG signals. Connect the module’s SPI pins to the appropriate ESP32 pins specified in the example. Note that correct wiring of ECG leads and the PPG sensor is essential for accurate data acquisition. For more details on setting up the hardware, including the pinout and datasheet of the AFE4950 Module, refer to the [AFE4950 Module Documentation](../README.md).

## Examples

[ECG and PPG Acquisition Example](../examples/Single-ECG-PPG-acquisition/Single-ECG-PPG-acquisition.ino): Configures the AFE4950 to simultaneously acquire ECG and PPG signals at a sample rate of 500 Hz. The data sequence in the FIFO is set to ECG, ECG, PPG for efficient reading.

## Requirements

 - AFE4950 Module: This library is specifically tailored for the AFE4950 Module, a custom-designed module for physiological signal acquisition. Please ensure you have this module and the associated setup information. More Information

 - ESP32 (recommended as the host): Utilizes the hardware SPI interface on the ESP32 for efficient data transfer from the AFE4950.

For additional setup instructions and API documentation, refer to the [api.md](api.md) in this docs folder.
