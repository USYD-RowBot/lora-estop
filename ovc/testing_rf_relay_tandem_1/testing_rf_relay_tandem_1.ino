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

#define TIMEOUT 250

char state = SAFE; // The state of the system


// Test Configuration

char myState = 0;

char messageLen = 3;
char onSig = 'H';
char offSig = 'L';

long lastTime, curTime;
int stepTime = 1000;



void setup() {
  state = SAFE;

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
  Serial.println("Sending message");

  // Build the message to send
  char toSend[messageLen];
  sprintf(toSend, "Hi");

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

      Serial.print("Got reply: ");
      Serial.print((char*)buf);
      Serial.print(" RSSI: ");
      Serial.print(rf95.lastRssi(), DEC);
      Serial.println();

      if(buf[0] == 'H') {
        Serial.println("Good Response, switching to RUN");
        state = RUN;
        relayOn();
      }
      else {
        Serial.println("Bad Response, switching to SAFE");
        state = SAFE;
        relayOff();
      }

      Serial.println(" ");
    }
    else
    {
      Serial.println("Receive failed");
      Serial.println("No Response, switching to SAFE");
      state = SAFE;
      relayOff();
    }
  }
  else
  {
    Serial.println("No reply, is the receiver running?");
    digitalWrite(LED, LOW); //Turn off status LED
    Serial.println("No Response, switching to SAFE");
    Serial.println(" ");
    state = SAFE;
    relayOff();
  }
  
  delay(250);
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
