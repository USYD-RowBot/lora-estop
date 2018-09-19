/**

*/

// Includes

// Pin Constants

// LoRa Comms Constants

// IIC Comms Constants

// Data Constants

// Setup
void setup() {
  // configure ports

  // configure IIC Comms

  // Configure Relay

  // configure LoRa radio

}

void loop() {

}

/*!
   \brief Handles behaviour when the watchdog timer runs out
   \pre IIC link must be enabled, Relay module enable system must be running
   \return 0 for success, 1 for failure
*/
int timeout() {

}

/*!
   \brief Resets the watchdog timer
   \pre Timer must be enabled
   \return 0 for success, 1 for failure
*/
int resetTimer() {

}

/*!
   \brief Transmits a given value over LoRa to the handheld unit
   \param value - unsigned char - The number to send over the link for validation
   \pre LoRa must be configured
   \return 0 for success, 1 for failure
*/
int sendMessage(unsigned char value) {

}

/*!
   \brief Processes a received packet to compare it to the sent value
   \param packet - void* - the received data
   \param sentVal - unsigned char - the number initially sent, for comparison
   \pre LoRa must be configured
   \return 0 for success (valid comparison), 1 for invalid comparison, -1 for error
*/
int processReceived(void* packet, unsigned char sentVal) {

}

int restoreOperation() {

}
