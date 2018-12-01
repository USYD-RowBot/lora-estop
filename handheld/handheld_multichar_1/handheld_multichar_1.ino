/* Handheld unit code for USYD RowBot's 2018 WAM-V Remote emergency stop
 *
 * This code assumes that the emergency stop button is connected in a loop from +VCC to Digital Pin 10 using the normally-closed connection.
 * A pull-down resistor and debounce capacitor for Pin 10 are also expected. 75k pull-down worked well in testing.
 *
 * This code implements a multi-characted key system for improved robustness.
 * As such, this code should be paired with ovc_multichar_1 code for proper functionality
 *
 */

 #include <SPI.h>

 //Install this library by downloading the lastest zip from http://www.airspayce.com/mikem/arduino/RadioHead/
 #include <RH_RF95.h>

 //On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
 RH_RF95 rf95(A5, 7);

 const bool DEBUG = false;

 const byte LED = 9; //Status LED is on pin 9
 const byte BUTTON = 10; // E-Stop button input is on pin 12

 const byte STOPCHAR = '!'; // The character to send to trigger an emergency stop condition

 int packetCounter = 0; //Counts the number of packets sent

 long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

 const char MSG_LEN = 5; // the character length of the message we're sending

 void setup()
 {
   pinMode(BUTTON, INPUT); // Sets up the button read pin for input

   pinMode(LED, OUTPUT);
   digitalWrite(LED, LOW);

   if(DEBUG) {
     Serial.begin(9600);
   }
   //while (Serial == false) ; //This line requires USB to be hooked up. If you're going to do battery powered range testing, comment it out.

   if(DEBUG){
     Serial.println("RFM Test");
   }

   if (rf95.init() == false)
   {
     if(DEBUG){
       Serial.println("Radio Init Failed - Freezing");
     }

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

       // Perpare to reply

       // Builds the return string, assuming that the button has been pressed so a logic error produces the safe condition
       char toSend[MSG_LEN];
       for(int i = 0; i < MSG_LEN; i++){
         if(i < MSG_LEN - 1){
           toSend[i] = '!';
         }
         else {
           toSend[i] = '\0';
         }
       }

       // Checks to see if the E-Stop button is still in its normal state
       if(digitalRead(BUTTON) == 1) {
         // toSend[0] = buf[0]; // sets the return string to echo correctly
         for(int i = 0; i < MSG_LEN; i++){
           if(i < MSG_LEN - 1){
             toSend[i] = buf[i];
           }
           else {
             toSend[i] = '\0';
           }
         }
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
