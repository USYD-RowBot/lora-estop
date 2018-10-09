/*
  ProRF - Basic data transmission over the air
  By: Nathan Seidle
  SparkFun Electronics
  Date: June 12th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware License).

  This example shows how to transmit a simple data message from the TX (transmitter) to the RX (receiver).
  Data can be sent in either direction but these example use TX and RX nomenclature to avoid confusion.

  This code is written for the receiver or 'server' that listens for incoming data and responds with a
  'I heard you' packet.

  This code is heavily based on RadioHead's rfxx_client and rfxx_server examples.

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ProRF with RFM69: https://www.sparkfun.com/products/14757
  ProRF LoRa with RFM95: https://www.sparkfun.com/products/14785

  Software and Hardware Setup:
  You'll need to download and install the SparkFun Atmel Boards. See this repo:
  https://github.com/sparkfun/Arduino_Boards for more information.
  Select 'SparkFun Pro Micro, 3.3V' as your Tools->Board.
  Both the TX and RX ProRF boards will need a wire antenna. We recommend a 3" piece of wire.

*/

#include <SPI.h>

// RF Config

//Install this library by downloading the lastest zip from http://www.airspayce.com/mikem/arduino/RadioHead/
#include <RH_RF95.h>

//On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
RH_RF95 rf95(A5, 7);

// Relay Config
#include <Wire.h>
#define COMMAND_RELAY_OFF     0x00
#define COMMAND_RELAY_ON      0x01

// relayState
#define ON 1
#define OFF 0

char relayState = OFF;


const byte qwiicRelayAddress = 0x18;     //Default Address

const byte LED = 13; //Status LED is on pin 13

// Message Protocol Config
const int MSG_LEN = 1; // the length of the message to send
const int MAX_RAND = 256; // the max value (exclusive) for the challenge code
const int TIMEOUT = 500; // the watchdog timeout in milliseconds
const int PAUSE_TIME = 500; // the downtime between transmissions

int packetCounter = 0; //Counts the number of packets sent

long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Qwiic relay config
  Wire.begin(); // join the I2C Bus
  testForConnectivity(); // test the I2C connection
  relayOff(&relayState); // start safe

  Serial.begin(9600);
  //while (Serial == false) ; //This line requires USB to be hooked up. If you're going to do battery powered range testing, comment it out.
  Serial.println("Booting");
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

void loop()
{
  char challengeCode = random(MAX_RAND); // generates the challnge code for the safety heartbeat
  Serial.println("Sending message");

  //Send a message to the other radio
  char toSend[MSG_LEN];
  sprintf(toSend, challengeCode);
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

      //* checks against the challenge code
      if(buf[0] != challengeCode) { //* if the code doesn't check out -- Fail to Safe
        relayOff(&relayState); //* turn the relay off
      }
      else { //* the code checks out, remain active
        if(relayState != ON) { //* If inactive i.e. Safe
          relayOn(&relayState); //* Set to active
        }
      }

      Serial.print("Got reply: ");
      char outVal[4];
      sprintf(outVal, "%3d",buf[0]);
      Serial.print(outVal);
//      Serial.print("%3i",(char*)buf[0]);
      Serial.print(" RSSI: ");
      Serial.print(rf95.lastRssi(), DEC);
      Serial.println();
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else //* No Reply -- Fail to Safe
  {
    Serial.println("No reply, is the receiver running?");
    digitalWrite(LED, LOW); //Turn off status LED

    relayOff(&relayState); // turn the relay off
  }
  delay(PAUSE_TIME);
}


// RelayOn() turns on the relay at the qwiicRelayAddress
// Checks to see if a slave is connected and prints a
// message to the Serial Monitor if no slave found.
void relayOn(char* relayState) {
  Wire.beginTransmission(qwiicRelayAddress);
  Wire.write(COMMAND_RELAY_ON);
  Wire.endTransmission();
  *relayState = ON;
}


// RelayOff() turns off the relay at the qwiicRelayAddress
// Checks to see if a slave is connected and prints a
// message to the Serial Monitor if no slave found.
void relayOff(char* relayState) {
  Wire.beginTransmission(qwiicRelayAddress);
  Wire.write(COMMAND_RELAY_OFF);
  Wire.endTransmission();
  *relayState = OFF;
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
