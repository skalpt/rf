#include <OneWire.h>
#include <LiquidCrystal.h>
#include <RF12.h>
#include <Ports.h>

class PacketBuffer : public Print
{
  public:
    PacketBuffer () : fill(0) {}
    const byte* buffer() { return buf; }
    byte length() { return fill; }
    void reset() { fill = 0; }
    virtual void write(uint8_t ch) { if (fill < sizeof buf) buf[fill++] = ch; }
  private:
    byte fill, buf[RF12_MAXDATA];
};

OneWire ds(9);
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

MilliTimer readTimer;
MilliTimer readDoneTimer;
MilliTimer scrollTimer;
byte readStatus = 1;
byte needToSend = 0;
byte scrollStatus = 0;
byte addr[8];
String localTemp;
String remoteTemp;
long localReadTime = 0;
long remoteReadTime = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Initialising...");

  rf12_initialize(1, RF12_433MHZ);

  scrollTimer.set(1);
}

void loop()
{
  byte i;

  if (rf12_recvDone() && rf12_crc == 0)
  {
    remoteReadTime = millis();

    remoteTemp = "";
    for (i = 0; i < rf12_len; ++i) remoteTemp += rf12_data[i];
  }

  if (needToSend && rf12_canSend())
  {
    PacketBuffer payload;
    payload.print(localTemp);
    rf12_sendStart(0, payload.buffer(), payload.length());
    needToSend = 0;
  }

  if (readStatus == 0 && readTimer.poll(10000)) readStatus = 1;

  if (readStatus == 1)
  {
    if (!ds.search(addr))
    {
      ds.reset_search();
      return;
    }
    if (ds.crc8(addr, 7) != addr[7]) return;
    if (addr[0] != 0x28) return;
    
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);
    
    readDoneTimer.set(1000);
    readStatus = 2;
  }

  if (readStatus == 2 && readDoneTimer.poll())
  {
    ds.reset();
    ds.select(addr);
    ds.write(0xBE);
  
    byte data[9];
    for (i = 0; i < 9; i++) data[i] = ds.read();
    if (ds.crc8(data, 8) != data[8]) return;
  
    localReadTime = millis();
  
    int TReading = (data[1] << 8) + data[0];

    byte SignBit = 0;
    if (TReading < 0) SignBit = 1;
    TReading = abs(TReading);

    int Tc_100 = (6 * TReading) + TReading / 4;
    int Whole = Tc_100 / 100;
    int Fract = Tc_100 % 100;
    
    localTemp = "";
    if (Whole < 10) localTemp += " ";
    if (SignBit) localTemp += "-";
    else if (Whole < 100) localTemp += " ";
    localTemp += Whole;
    localTemp += ".";
    if (Fract < 10) localTemp += "0";
    localTemp += Fract;
  
    ds.reset_search();

    needToSend = 1;
    readStatus = 0;
  }
  
  if (scrollTimer.poll())
  {
    switch (scrollStatus)
    {
      case 0:
        scrollStatus = 1;
        if (localReadTime == 0)
        {
          scrollTimer.set(1);
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Local:");
          lcd.setCursor(0, 1);
          lcd.print(localTemp.trim());
          lcd.print((char)223);
          lcd.print("C ");
          if (millis() - localReadTime < 60000)
          {
            lcd.print((millis() - localReadTime) / 1000);
            lcd.print("s ago");
          }
          else if (millis() - localReadTime < 3600000)
          {
            lcd.print((millis() - localReadTime) / 60000);
            lcd.print("m ago");
          }
          else
          {
            lcd.print((millis() - localReadTime) / 3600000);
            lcd.print("h ago");
          }
          scrollTimer.set(3000);
        }
        break;
      case 1:
        scrollStatus = 0;
        if (remoteReadTime == 0)
        {
          scrollTimer.set(1);
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Remote:");
          lcd.setCursor(0, 1);
          lcd.print(remoteTemp.trim());
          lcd.print((char)223);
          lcd.print("C ");
          if (millis() - remoteReadTime < 60000)
          {
            lcd.print((millis() - remoteReadTime) / 1000);
            lcd.print("s ago");
          }
          else if (millis() - remoteReadTime < 3600000)
          {
            lcd.print((millis() - remoteReadTime) / 60000);
            lcd.print("m ago");
          }
          else
          {
            lcd.print((millis() - remoteReadTime) / 3600000);
            lcd.print("h ago");
          }
          scrollTimer.set(3000);
        }
        break;
    }
  }
}

