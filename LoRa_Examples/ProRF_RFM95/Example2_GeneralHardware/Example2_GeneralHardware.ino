/*
  ProRF - Basic hardware control
  By: Nathan Seidle
  SparkFun Electronics
  Date: June 12th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware License).

  This example shows how to control the basic hardware on the ProRF:
    Print to serial port
    Control GPIO
    Check battery level

  Feel like supporting open source hardware?
  Buy a board from SparkFun! 
  ProRF with RFM69: https://www.sparkfun.com/products/14757
  ProRF LoRa with RFM95: https://www.sparkfun.com/products/14785
*/

const byte batteryVoltage = A11; //The battery level is on pin D12/A11
const byte statLED = 13;

void setup()
{
  Serial.begin(9600); //USB communication
  while (Serial == false) ; //Wait for terminal window to be opened

  Serial1.begin(9600); //TX pin on the board

  pinMode(batteryVoltage, INPUT);

  pinMode(statLED, OUTPUT);
}

void loop()
{
  Serial.println("This prints over USB");
  Serial1.println("This prints over the TX pin");

  //BattLevel is read through a 1/2 resistor divider
  //Assuming the board is running at 3.3V, that means that 1024 = 3.3V
  float battLevel = analogRead(batteryVoltage) * 3.3 / 512;
  Serial.print("battLevel: ");
  Serial.print(battLevel, 2);
  Serial.println("V");

  //Toggle the status LED each second
  byte state = digitalRead(statLED);
  if(state == LOW)
    digitalWrite(statLED, HIGH);
  else
    digitalWrite(statLED, LOW);

  delay(1000);
}

