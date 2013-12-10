// For initial testing of RFM12B modules.
// Based on RF12 library's pingPong example, but with LCD support.
// Created on 2011-03-30 by JD

#include <LiquidCrystal.h>
#include <RF12.h>
#include <Ports.h>

MilliTimer sendTimer;
char payload[] = "Hello!";
byte needToSend = 1;
LiquidCrystal lcd(8, 7, 6, 5, 4, 3); // Would be good to incorporate something like this into RF12 library

void setup()
{
  Serial.begin(57600);
  Serial.println("Ping Pong Demo");

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Ping Pong Demo");

  rf12_initialize(1, RF12_433MHZ);
}

void loop()
{
  if (rf12_recvDone() && rf12_crc == 0) // Would be interesting to count damaged packets
  {
    Serial.print("R@");
    Serial.print(millis() / 1000);
    Serial.print("s: ");
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print("R@");
    lcd.print(millis() / 1000);
    lcd.print("s: ");
    for (byte i = 0; i < rf12_len; ++i)
    {
      Serial.print(rf12_data[i]);
      lcd.print(rf12_data[i]);
    }
    Serial.println();
  }
    
  if (sendTimer.poll(3000)) needToSend = 1;

  if (needToSend && rf12_canSend())
  {
    needToSend = 0;
    rf12_sendStart(0, payload, sizeof payload);
    Serial.print("S@");
    Serial.print(millis() / 1000);
    Serial.print("s: ");
    Serial.println(payload);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("S@");
    lcd.print(millis() / 1000);
    lcd.print("s: ");
    lcd.print(payload);
  }
}
