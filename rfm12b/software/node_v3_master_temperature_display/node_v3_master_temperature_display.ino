/****************************************************************************************************
To Do:
- Add concise version of following to comments on nodeData:
  - Type: byte, 1 byte (e.g. T for temperature node, L for light node, D for display node, R for relay node [not sure how this one is going to work yet])
  - Name: char[], up to 15 bytes (set by the master but stored by the node)
  - Most recent reading/status: byte[], up to 2 bytes
  - Local time of most recent reading/status: long, 4 bytes
  - TOTAL: 22 bytes
- The following constants are shared across master/node code, so they should really be added to a library:
  - MASTERID
  - MINPOLLCYCLETIME

Acknowledgements:
- const vs. define: http://www.velocityreviews.com/forums/t289978-const-vs-define-in-the-header-file.html
- Converting between long and byte: http://rclermont.blogspot.com/2010/02/convert-long-into-byte-array-in-cc.html
- Passing arrays to functions by reference: http://stackoverflow.com/questions/1106957/pass-array-by-reference-in-c
- Optional parameters in functions: http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1291796756
*****************************************************************************************************/

// Libraries:
#include <RF12.h>                       // Library for RF communication (RFM12B) - needs revising
#include <Ports.h>                      // Sub-library of RF12 (can probably be removed upon revision of RF12 library)
#include <OneWire.h>                    // Library for temperature readings (DS18B20)
#include <LiquidCrystal.h>

// Pins:
const int PIN_ONEWIRE = 19;             // Pin for DS18B20
const int PIN_LCD[] = {8, 7, 6, 5, 4, 3};

// Constants:
const int MASTERID = 31;                //
const int MINPOLLID = 1;                //
const int MAXPOLLID = 30;               //
const int MAXPOLLWAITTIME = 30;         //
const int MINPOLLCYCLETIME = 10000;     //
const int MAXPOLLRETRIES = 2;           //

// Initialisation:
OneWire ds(PIN_ONEWIRE);                // Initialise the DS18B20
LiquidCrystal lcd(PIN_LCD[0], 18, PIN_LCD[1], PIN_LCD[2], PIN_LCD[3], PIN_LCD[4], PIN_LCD[5]);
char nodeName[15] = "Bedroom";          // The name of the virtual temperature node

// Polling:
int pollStatus = 1;                     //
int pollID = MINPOLLID;                 //
int pollSingleCount = 0;                //
unsigned long firstPollTime = 0;        //
unsigned long pollCycleCount = 0;       //

// Outgoing Packets:
byte sendData[RF12_MAXDATA];            // Data to be sent to the node
int sendDataLen;                        // Stores the actual number of bytes to be sent to the node
byte sendDataFinal[RF12_MAXDATA];       // 
int sendDataFinalLen;                   //
boolean needToSend = false;             // Set to true when data is ready to be sent to the node
unsigned long lastSendTime = 0;         //
int sendCursor = MINPOLLID;             // Keeps track of the last reading sent to a display node

// Incoming Packets:
unsigned long recvTime;                 //
byte recvData[RF12_MAXDATA];            // Most recent data received
byte nodeData[MAXPOLLID+2][22];         // Stores up-to-date information about each node

// Readings:
MilliTimer readTimer;                   // Timer for keeping track of when temperature reading is due - might end up replacing this with a sleep timer
MilliTimer readDoneTimer;               // Timer for keeping track of when a temperature reading is complete - might end up replacing this with a 3-wire DS18B20 configuration and check constantly for completion
int readStatus = 1;                     // Keeps track of what stage the temperature reading is at:
                                        //   0:  Idle, waiting for next read time
                                        //   1:  Ready to read
                                        //   2:  Read instruction sent, waiting for read to complete
byte readingAddr[8];                    // Unique address of the DS18B20
byte readingData[9];                    // Data retrieved from the DS18B20

// Display:
MilliTimer scrollTimer;
int scrollID = MINPOLLID;
int lastScrollID;
unsigned long localReadTime;

// Special byte array functions:

void tfrByteArray(byte *fromArray, byte *toArray, int arraySize)
{
  for (int i = 0; i < arraySize; i++)
  {
    toArray[i] = fromArray[i];
  }
}

void clrByteArray(byte *clearArray, int arraySize)
{
  for (int i = 0; i < arraySize; i++)
  {
    clearArray[i] = 0;
  }
}

void addToByteArray(byte *toArray, int startPos, byte *addArray, int toPos, int fromPos = 0)
{
  for (int i = 0; i <= toPos - fromPos; i++)
  {
    toArray[i + startPos] = addArray[i + fromPos];
  }
}

void addIntToByteArray(byte *toArray, int startPos, int addInt)
{
  for (int i = 0; i < sizeof(int); i++)
  {
    toArray[i + startPos] = byte(addInt >> (i * 8) & 0XFF);
  }
}

void addLongToByteArray(byte *toArray, int startPos, unsigned long addLong)
{
  for (int i = 0; i < sizeof(long); i++)
  {
    toArray[i + startPos] = byte(addLong >> (i * 8) & 0XFF);
  }
}

void subtractLongFromByteArray(byte *fromArray, int startPos, unsigned long newLong)
{
  unsigned long oldLong = extractLongFromByteArray(fromArray, startPos);
  addLongToByteArray(fromArray, startPos, newLong - oldLong);
}

int extractIntFromByteArray(byte *fromArray, int startPos)
{
  int extractInt = 0;
  for (int i = 0; i < sizeof(int); i++)
  {
    extractInt += (unsigned int)(fromArray[i + startPos]) << (i * 8);
  }
  return extractInt;
}

unsigned long extractLongFromByteArray(byte *fromArray, int startPos)
{
  unsigned long extractLong = 0;
  for (int i = 0; i < sizeof(long); i++)
  {
    extractLong += (unsigned long)(fromArray[i + startPos]) << (i * 8);
  }
  return extractLong;
}

String extractStringFromByteArray(byte *fromArray, int toPos, int fromPos = 0)
{
  String extractString = "";
  for (int i = 0; i <= toPos - fromPos; i++)
  {
    if (fromArray[i + fromPos] != 0) extractString += fromArray[i + fromPos];
  }
  return extractString;
}

String extractTempFromByteArray(byte *fromArray, int startPos)
{
  int extractInt = extractIntFromByteArray(fromArray, startPos);
  int Tc_100 = extractInt * 6 + extractInt / 4;
  
  String extractTemp = "";
  extractTemp += Tc_100 / 100;
  extractTemp += ".";
  extractTemp += abs(Tc_100) % 100 < 10 ? "0" : "";
  extractTemp += abs(Tc_100) % 100;

  return extractTemp;
}

// Main Program:

void setup()
{
  pinMode(9, OUTPUT);
  analogWrite(9, 64);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Starting up...");

  Serial.begin(57600);                  // Initialise serial port at 57600 baud
  Serial.print("\nMaster started.\n\n");
                                        //

  rf12_initialize(MASTERID, RF12_433MHZ);
                                        // Initialise the RFM12B

  nodeData[MAXPOLLID+1][0] = 'T';
  addToByteArray(nodeData[MAXPOLLID+1], 1, (byte*)nodeName, strlen(nodeName));

  scrollTimer.set(1);
  firstPollTime = millis();             //
}

void loop()
{
  processRecv();                        // Manage any data received from a node
  // processRead();                     // Eventually this will be for reading any messages from serial port (e.g. to turn on a light)
  processSend();                        // Manage any data to be sent to a node
  processRead();
  processDisplay();
}

// Subroutines:

void processRecv()
{
  if (rf12_recvDone() && rf12_crc == 0) // && rf12_hdr == pollID)
                                        // Check if a packet has been received from the node being polled
  {
    getRecvData();                      // If so, get the data
    processRecvAction();                // Process data and take any required action
  }
}

void getRecvData()
{
  recvTime = millis();

  clrByteArray(recvData, sizeof(recvData));
                                        // Clear variable in preparation for reading in new data - don't really need to do this, but we should in case future processes need it
  
  for (int i = 0; i < rf12_len; i++)
  {
    recvData[i] = rf12_data[i];         // Read each byte received
  }
}

void processRecvAction()
{
  Serial.print("R");
  Serial.print((int)rf12_hdr);
  Serial.print(":");
  Serial.print(recvData[0]); // May need to cast as char

  pollSingleCount++;
  pollStatus = 1;

  switch(recvData[0])
  {
    case 'E':
      incrementPollID();
      break;
    case 'I':
      addToByteArray(nodeData[pollID], 0, recvData, strlen((char*)recvData) - 1, 1);
      for (int i = 1; i < strlen((char*)recvData); i++)
      {
        Serial.print(recvData[i]);
      }
      break;
    case 'R':
      addToByteArray(nodeData[pollID], 16, recvData, 10, 5);
      subtractLongFromByteArray(nodeData[pollID], 18, recvTime); // Should probably also take transmission time into account here
      Serial.print(extractLongFromByteArray(recvData, 1));
      Serial.print(",");
      Serial.print(extractIntFromByteArray(nodeData[pollID], 16));
      Serial.print(":"); // Debug line
      Serial.print(extractTempFromByteArray(nodeData[pollID], 16)); // Debug line
      Serial.print(",");
      Serial.print(extractLongFromByteArray(recvData, 7)); // Debug line
      Serial.print(":"); // Debug line
      Serial.print(extractLongFromByteArray(nodeData[pollID], 18)); // Debug line
      Serial.print(":"); // Debug line
      Serial.print(millis() - extractLongFromByteArray(nodeData[pollID], 18));
      break;
    case 'P':
      break;
  }
  Serial.print("\n");
}

void incrementPollID()
{
  sendCursor = MINPOLLID;
  if (pollID == MAXPOLLID)
  {
    pollStatus = 0;
    Serial.print("\n");
  } else
  {
    pollStatus = 1;
    pollSingleCount = 0;
    pollID++;
  }
}

void processSend()
{
  processSendData();
  processSendAction();
}

void processSendData()
{
  if (pollStatus == 1)
  {
    if (nodeData[pollID][0] == 0)
    {
      sendData[0] = 'I';
      sendDataLen = 1;
      needToSend = true;
      pollStatus = 2;
    }
    else if (nodeData[pollID][1] == 'D')
    {
      sendReading();
    }
    else
    {
      sendData[0] = 'P';
      sendDataLen = 1;
      needToSend = true;
      pollStatus = 2;
    }
  }
  
  if (pollStatus > 1 && !needToSend && millis() - lastSendTime >= MAXPOLLWAITTIME)
  {
    if (pollStatus < MAXPOLLRETRIES + 2)
    {
      pollStatus++;
      needToSend = true;
    } else
    {
      incrementPollID();
    }
  }
  
  if (pollStatus == 0 && millis() - firstPollTime >= MINPOLLCYCLETIME)
  {
    pollID = MINPOLLID;
    pollStatus = 1;
    pollSingleCount = 0;
    pollCycleCount++;
    firstPollTime = millis();
  }
}

void sendReading()
{
  while (extractLongFromByteArray(nodeData[scrollID], 18) == 0 && sendCursor <= MAXPOLLID)
  {
    sendCursor++;
  }
  if (sendCursor <= MAXPOLLID)
  {
    incrementPollID();
  }
  else
  {
    sendData[0] = 'R';
    addIntToByteArray(sendData, 1, sendCursor);
    addToByteArray(sendData, 3, nodeData[sendCursor], 21);
    sendDataLen = 25;
    needToSend = true;
    pollStatus = 2;
  }
}

void processSendAction()
{
  if (needToSend && rf12_canSend())
  {
    tfrByteArray(sendData, sendDataFinal, sendDataLen);
    sendDataFinalLen = sendDataLen;

    if (sendDataFinal[0] == 'R')
    {
      subtractLongFromByteArray(sendDataFinal, 21, millis());
    }
    if (sendDataFinal[0] == 'I' || sendDataFinal[0] == 'P')
    {
      addIntToByteArray(sendData, 1, pollSingleCount);
      addLongToByteArray(sendData, 3, pollCycleCount);
      sendDataFinalLen += 6;
    }

    Serial.print("S");
    Serial.print(pollID);
    Serial.print(":");
    for (int i = 0; i < sendDataLen; i++)
    {
      Serial.print(sendDataFinal[i]);
    }
    Serial.print("\n");

    byte header = 0;
    header |= RF12_HDR_DST | (byte)pollID;
    rf12_sendStart(header, sendDataFinal, sendDataFinalLen);
    lastSendTime = millis();
    needToSend = false;
  }
}

void processRead()
{
  if (readStatus == 0 && readTimer.poll(10000))
  {
    readStatus = 1;
  }
  
  if (readStatus == 1)
  {
    getReading();
  }

  if (readStatus == 2 && readDoneTimer.poll())
  {
    processReading();
  }
}

void getReading()
{
  if (!ds.search(readingAddr))
  {
    ds.reset_search();
    return;
  }
  
  if (ds.crc8(readingAddr, 7) != readingAddr[7])
  {
    return;
  }
  
  if (readingAddr[0] != 0x28)
  {
    return;
  }

  ds.reset();
  ds.select(readingAddr);
  ds.write(0x44, 1);
  
  readDoneTimer.set(1000);
  readStatus = 2;
}

void processReading()
{
  ds.reset();
  ds.select(readingAddr);
  ds.write(0xBE);
  
  for (int i = 0; i < 9; i++)
  {
    readingData[i] = ds.read();
  }
  
  if (ds.crc8(readingData, 8) != readingData[8])
  {
    return;
  }
  
  addLongToByteArray(nodeData[MAXPOLLID+1], 18, millis());
  addToByteArray(nodeData[MAXPOLLID+1], 16, readingData, 1);
  
  ds.reset_search();
  readStatus = 0;
}

void processDisplay()
{
  if (scrollTimer.poll())
  {
    lastScrollID = scrollID;
    do
    {
      scrollID++;
      if (scrollID > MAXPOLLID+1) scrollID = MINPOLLID;
      if (scrollID == lastScrollID) break;
    } while (nodeData[scrollID][0] != 'T' || extractLongFromByteArray(nodeData[scrollID], 18) == 0);
    if (nodeData[scrollID][0] == 'T' && extractLongFromByteArray(nodeData[scrollID], 18) != 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(extractStringFromByteArray(nodeData[scrollID], 15, 1));
      lcd.print(":");
      lcd.setCursor(0, 1);
      lcd.print(extractTempFromByteArray(nodeData[scrollID], 16));
      lcd.print((char)223);
      lcd.print("C ");
      localReadTime = millis() - extractLongFromByteArray(nodeData[scrollID], 18);
      if (localReadTime < 60000)
      {
        lcd.print(localReadTime / 1000);
        lcd.print("s ago");
      }
      else if (localReadTime < 3600000)
      {
        lcd.print(localReadTime / 60000);
        lcd.print("m ago");
      }
      else
      {
        lcd.print(localReadTime / 3600000);
        lcd.print("h ago");
      }
      scrollTimer.set(3000);
    }
    else
    {
      scrollTimer.set(1);
    }
  }
}
