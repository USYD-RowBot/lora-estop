/* Handheld unit code for USYD RowBot's 2018 WAM-V Remote emergency stop
 * 
 * This code assumes that the emergency stop button is connected in a loop from +VCC to Digital Pin 12 using the normally-closed connection.
 * A pull-down resistor and debounce capacitor for Pin 12 are also expected.
 * 
 * This code implements the updated stop protocol from 2018-10-10 and should be paired with compatible OVC code such as updated_protocol_1.ino
 * 
 * 
 */
 
#include <SPI.h>

//Install this library by downloading the lastest zip from http://www.airspayce.com/mikem/arduino/RadioHead/
#include <RH_RF95.h>

//On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
RH_RF95 rf95(A5, 7);

const byte LED = 13; //Status LED is on pin 13

const byte BUTTON = 12; // E-Stop button input is on pin 12

const byte STOPCHAR = '!'; // The character to send to trigger an emergency stop condition

int packetCounter = 0; //Counts the number of packets sent

long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

const char MSG_LEN = 2; // the character length of the message we're sending

void setup()
{
  pinMode(BUTTON, INPUT); // Sets up the button read pin for input
  
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
  digitalWrite(LED,HIGH);
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH); //Turn on status LED
      timeSinceLastPacket = millis(); //Timestamp this packet

//      Serial.print("Got message: ");
//      Serial.print(int(buf[0]));
//      Serial.print("Message Len: ");
//      Serial.print(sizeof(buf));
//      Serial.print(" RSSI: ");
//      Serial.print(rf95.lastRssi(), DEC);
//      Serial.println();
      
      // Perpare to reply

      // Builds the return string, assuming that the button has been pressed so a logic error produces the safe condition
      char toSend[] = {'!', '\0'};

      // Checks to see if the E-Stop button is still in its normal state
      if(digitalRead(BUTTON) == 1) {
        toSend[0] = buf[0]; // sets the return string to echo correctly
      }

      rf95.send(toSend, sizeof(toSend)); // sends the response packet

      rf95.waitPacketSent();
//      Serial.println("Sent a reply");
    }
    else
    {
//      Serial.println("recv failed");
    }
  }

  //Turn off status LED if we haven't received a packet after 1s
  if(millis() - timeSinceLastPacket > 1000)
  {
    digitalWrite(LED, LOW); //Turn off status LED
    timeSinceLastPacket = millis(); //Don't write LED but every 1s
  }
}
