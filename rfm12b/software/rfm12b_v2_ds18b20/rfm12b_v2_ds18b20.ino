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
byte readStatus = 1;
byte needToSend = 0;
byte addr[8];
String temp;

void setup()
{
  String here = "Local:   --.--";
  here += char(223);
  here += 'C';
  String other = "Remote:  --.--";
  other += char(223);
  other += 'C';

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(here);
  lcd.setCursor(0, 1);
  lcd.print(other);

  rf12_initialize(1, RF12_433MHZ);
}

void loop()
{
  byte i;

  if (rf12_recvDone() && rf12_crc == 0)
  {
    lcd.setCursor(8, 1);
    for (i = 0; i < rf12_len; ++i) lcd.print(rf12_data[i]);
  }

  if (needToSend && rf12_canSend())
  {
    PacketBuffer payload;
    payload.print(temp);
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
  
    int TReading = (data[1] << 8) + data[0];

    byte SignBit = 0;
    if (TReading < 0) SignBit = 1;
    TReading = abs(TReading);

    int Tc_100 = (6 * TReading) + TReading / 4;
    int Whole = Tc_100 / 100;
    int Fract = Tc_100 % 100;
  
    temp = "";
    if (Whole < 10) temp += " ";
    if (SignBit) temp += "-";
    else if (Whole < 100) temp += " ";
    temp += Whole;
    temp += ".";
    if (Fract < 10) temp += "0";
    temp += Fract;
  
    lcd.setCursor(8, 0);
    lcd.print(temp);

    ds.reset_search();

    needToSend = 1;
    readStatus = 0;
  }
}

