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

//Install this library by downloading the lastest zip from http://www.airspayce.com/mikem/arduino/RadioHead/
#include <RH_RF95.h>

//On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
RH_RF95 rf95(A5, 7);

const byte LED = 13; //Status LED is on pin 13

int packetCounter = 0; //Counts the number of packets sent

long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);
  //while (Serial == false) ; //This line requires USB to be hooked up. If you're going to do battery powered range testing, comment it out.

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
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH); //Turn on status LED
      timeSinceLastPacket = millis(); //Timestamp this packet

      Serial.print("Got message: ");
      Serial.print((char*)buf);
      Serial.print(" RSSI: ");
      Serial.print(rf95.lastRssi(), DEC);
      Serial.println();

      // Send a reply
      char toSend[50];
      sprintf(toSend, "And hello back. Counter: %d", packetCounter++);
      rf95.send(toSend, sizeof(toSend));

      rf95.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }

  //Turn off status LED if we haven't received a packet after 1s
  if(millis() - timeSinceLastPacket > 1000)
  {
    digitalWrite(LED, LOW); //Turn off status LED
    timeSinceLastPacket = millis(); //Don't write LED but every 1s
  }
}


