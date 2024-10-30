/*
 * Example: Single-Channel ECG and PPG Acquisition with AFE4950 Module and ESP32
 * 
 * Description:
 * This example demonstrates how to use the AFE4950 Module with an ESP32 to acquire 
 * single-channel ECG and PPG signals simultaneously. The AFE4950 is configured 
 * with a sampling frequency of 500 Hz, and the FIFO is set with a watermark level 
 * of 3 samples.
 * 
 * Library: AFE4950
 * Author: Juan José Londoño Carrillo - Biomedical Engineer
 * License: MIT
 */

#include <AFE4950.h> // Include library

/*
 * Define MCU pins for SPI protocol
 * Define digital pin for AFE reset signal
 * Define digital pin for receive AFE interrupts
*/
#define SCLK 18
#define MISO 19
#define MOSI 23
#define CS 5
#define AFE_RST 4
#define FIFO_RDY 16

AFE4950 afe; // Create AFE object for control an AFE
bool captureEnabled = false; // Flag for control the data acquisition

void setup() {
  Serial.begin(115200); // Begin Serial communication

  while(!afe.configure(SCLK, MISO, MOSI, CS, AFE_RST, FIFO_RDY)) { // Wait for AFE configuration
    Serial.println("Error with AFE4950 configuration");
  }
}

void loop() {
  if (Serial.available() > 0) { // Search for data available in Serial port
    String command = Serial.readStringUntil('\n'); // Get command after a \n
    command.trim(); // Trim special characters in command like \r

    if (command.equalsIgnoreCase("start")) {
    captureEnabled = true;
    afe.enableCapture(); // Enable data capture from the AFE into the MCU
    }

    else if (command.equalsIgnoreCase("stop")) {
    captureEnabled = false; 
    afe.disableCapture(); // Disable data capture
    } 
  }

  if (captureEnabled) {
    if(afe.ready()) { // Ensure that MCU has readed the data from the AFE's FIFO
      /*
       * Get the data in a pointer to a constant buffer 
       * this buffer is read-only, its content must not be modified.
      */
      const uint8_t* data = afe.getData();
      size_t dataLength = afe.getBytesToSend(); // Get the number of bytes ready for send

      Serial.write(data, dataLength); // Send the entire buffer through Serial port
    }
  }
}