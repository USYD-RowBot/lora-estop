/*
 *  Code to run the On-Vehicle Controller (OVC) of the remote emergency stop for USYD RowBot's 2018 WAM-V
 *  This implements the protocol update for 2018-10-10, which addresses dropped packets without generating spurious stop signals
 *
 *  For safe operation, this OVC code must be paired with handheld code implementing the updated protocol, such as handheld_upgrade_1.ino
 *
 *  This code is similar to the previous version (reliability_improvement_1.ino) but it allows for resending in the case of a garbled or dropeed packet
 *  If 2 such errors occur in a row, the handheld is clearly experiencign issues, so the safe condition is triggered.
 *
 */

#include <Wire.h>

#include <SPI.h>

//Install this library by downloading the lastest zip from http://www.airspayce.com/mikem/arduino/RadioHead/
#include <RH_RF95.h>

// Quiic Relay Configuration
#define COMMAND_RELAY_OFF     0x00
#define COMMAND_RELAY_ON      0x01


const byte qwiicRelayAddress = 0x18;     //Default Address

// RF95 Comms configuration

//On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
RH_RF95 rf95(A5, 7);

const byte LED = 13; //Status LED is on pin 13

int packetCounter = 0; //Counts the number of packets sent

long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

// EStop Function Configuration

#define SAFE 0
#define RUN 1

#define TIMEOUT 350
#define DELAYTIME 250

#define RANDMAX 77

const byte STOPCHAR = '!';

char state = SAFE; // The state of the system


// Test Configuration

int numFailures, timeouts, invalids, badResponses;
int numStops;

char resend = 0;

const char MSG_LEN = 5;
char onSig = 'H';
char offSig = 'L';

long lastTime, curTime;
int stepTime = 1000;



void setup() {
  state = SAFE;

  numFailures = 0;
  timeouts = 0;
  invalids = 0;
  badResponses = 0;
  numStops = 0;

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);

  Serial.println("Relay Begin");
  Wire.begin(); // join the I2C Bus
  testForConnectivity();

  relayOff(); // ensure that we start in safe mode

  Serial.println("RF95 Begin");

  Serial.println("RFM Test");

  if (rf95.init() == false)
  {
    Serial.println("Radio Init Failed - Freezing");
    while (1);
  }

  //The SparkFun ProRF uses the RFM95W
  //This is a highpower, 915MHz device. The 915MHz ISM band is centered
  //around 915MHz but ranges from 902 to 928MHz so if needed you could use
  //908.2 for example if the band is very busy.
  rf95.setFrequency(909.2);

  //Europe is 863-870 with center at 868MHz
  //This works but it is unknown how well the radio configures to this frequency
  //rf95.setFrequency(864.1);

  //Set power up to 23dBm. False = use PA_BOOST instead of RFO pins
  rf95.setTxPower(23, false);

}

void loop() {

  // Produce the challenge string
  char toSend[MSG_LEN];
  for(int i = 0; i < MSG_LEN; i++) {
    if(i < MSG_LEN - 1){
      toSend[i] = char(random(RANDMAX) + '0');
    }
    else {
      toSend[i] = '\0';
    }

  }
  // char challengeVal = char(random(RANDMAX)) + 'A';

//  Serial.println("Sending message");

  // Build the message to send
  // char toSend[messageLen];
  // char msg[] = {challengeVal, '\0'};
  // sprintf(toSend, msg);

  // Send the message
  rf95.send(toSend, sizeof(toSend));
  rf95.waitPacketSent();

  // Now wait for a reply
  byte buf[RH_RF95_MAX_MESSAGE_LEN];
  byte len = sizeof(buf);

  if (rf95.waitAvailableTimeout(TIMEOUT))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
   {

      digitalWrite(LED, HIGH); //Turn on status LED

        // Serial status output

//      Serial.print("Got reply: ");
//      Serial.print((char*)buf);
//      Serial.print(" RSSI: ");
//      Serial.print(rf95.lastRssi(), DEC);
//      Serial.println();
//      Serial.print(", F: ");
//      Serial.print(numFailures, DEC);
      Serial.print("S: ");
      Serial.print(numStops, DEC);
      Serial.print(", T ");
      Serial.print(timeouts, DEC);
      Serial.print(", I ");
      Serial.print(invalids,DEC);
      Serial.print(", B ");
      Serial.print(badResponses, DEC);
      Serial.println();

      bool toStop = true;

      for(int i = 0; i < MSG_LEN - 1; i++) {
        char received = buf[i];

        if(received == toSend[i]) {
          toStop = false;
        }

        else if(received == STOPCHAR){
          toStop = true;
          resend = 0;

          numStops++;

          state = SAFE;
          relayOff();

          break;
        }

        else {
          numFailures++;
          invalids++;

          state = SAFE;
          relayOff();

          break;

        }

      }
      // char received = buf[0];

      // Test the received value for correctness
      // if(received == challengeVal) { // If the response is the challenge character
      if(!toStop) {
//        Serial.println("Valid Response, switching to RUN");
        state = RUN;
        relayOn();

        // Flag that a normal packet has been received
        resend = 0;
      }
//       else if(received == STOPCHAR) { // if the response is the stop character
//         numStops++;
//         state = SAFE;
//         relayOff();
//
//         // Flag that a normal packet has been received
//         resend = 0;
//       }
//       else {
//         numFailures++;
//         invalids++;
// //        Serial.println("Invalid Response, switching to SAFE");
//         state = SAFE;
//         relayOff();
//
//       }
//
//       Serial.println(" ");
    }
    else // Garbled response, resend
    {
      numFailures++;
      badResponses++;


      // Handles the resend logic
      if(resend > 0) { // If we've already tried to reacquire the signal, enter safe
        // enter safe
        state = SAFE;
        relayOff();

//      Serial.println("Receive failed");
//      Serial.println("Bad Response, switching to SAFE");

        // return to standard running
        resend = 0;
      }
      else { // If this is the first error
        resend++; // mark that we need to resend
      }


    }
  }
  else // If the response times out
  {
    numFailures++;
    timeouts++;
//    Serial.println("No reply, is the receiver running?");
    digitalWrite(LED, LOW); //Turn off status LED

    if(resend > 0) { // If we've already tried to reacquire the signal
      // go to safe
      state = SAFE;
      relayOff();

//      Serial.println("No Response, switching to SAFE");
//      Serial.println(" ");

      // return to standard running
      resend = 0;
    }
    else { // If this is the first error
      resend++; // mark that we need to resend
    }

  }

  delay(DELAYTIME); // lwait a little while before initiating the next transmission
}

//*********** Quiic Relay Functions **********************

// RelayOn() turns on the relay at the qwiicRelayAddress
// Checks to see if a slave is connected and prints a
// message to the Serial Monitor if no slave found.
void relayOn() {
  Wire.beginTransmission(qwiicRelayAddress);
  Wire.write(COMMAND_RELAY_ON);
  Wire.endTransmission();
}

// RelayOff() turns off the relay at the qwiicRelayAddress
// Checks to see if a slave is connected and prints a
// message to the Serial Monitor if no slave found.
void relayOff() {
  Wire.beginTransmission(qwiicRelayAddress);
  Wire.write(COMMAND_RELAY_OFF);
  Wire.endTransmission();
}

// testForConnectivity() checks for an ACK from an Relay. If no ACK
// program freezes and notifies user.
void testForConnectivity() {
  Wire.beginTransmission(qwiicRelayAddress);
  //check here for an ACK from the slave, if no ack don't allow change?
  if (Wire.endTransmission() != 0) {
    Serial.println("Check Connections. No slave attached.");
    while (1);
  }
}
