/****************************************************************************************************
To Do:
- Add serial messages for debugging
- Create a proper ack system for incrementing the sendCursor

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

// Pins:
const int PIN_STATUSGREEN = 6;          // Pin for status LED (green mode)
const int PIN_STATUSRED = 7;            // Pin for status LED (red mode)
const int PIN_ONEWIRE = 8;              // Pin for DS18B20

// Constants:
const int MASTERID = 31;                //
const int MINPOLLCYCLETIME = 10000;     //
const int MAXREADINGS = 100;            // Maximum number of readings to store between transmissions
const int STATUSONTIME = 50;            // Number of milliseconds status LED should be on
const int STATUSOFFTIME = 950;          // Number of milliseconds status LED should be off

// Initialisation:
OneWire ds(PIN_ONEWIRE);                // Initialise the DS18B20
boolean initialised = false;            // The first transmission needs to include initialisation details such as node name
int nodeID = 2;                         // The ID of the node (1 to 31) - should be set by a DIP switch and read during setup()
char nodeName[15] = "Nursery";          // The name of the node - should be stored in EEPROM and editable via master
                                        // In the meantime, current hard-coded nodes are:
                                        //   1: Bedroom
                                        //   2: Nursery
                                        //   3: Outside

// Incoming Packets:
unsigned long lastRecvTime = 0;         //
byte recvData[RF12_MAXDATA];            // Data received from the master
byte lastRecvData[RF12_MAXDATA];        // Last data received from the master
char recvAction;                        // Action requested from the master

// Outgoing Packets:
byte sendData[RF12_MAXDATA];            // Data to be sent to the master
int sendDataLen;                        // Stores the actual number of bytes to be sent to the master
byte sendDataFinal[RF12_MAXDATA];       // 
boolean needToSend = false;             // Set to true when data is ready to be sent to the master
int sendCursor = -1;                    // Keeps track of the last reading sent to the master

// Readings:
MilliTimer readTimer;                   // Timer for keeping track of when temperature reading is due - might end up replacing this with a sleep timer
MilliTimer readDoneTimer;               // Timer for keeping track of when a temperature reading is complete - might end up replacing this with a 3-wire DS18B20 configuration and check constantly for completion
int readStatus = 1;                     // Keeps track of what stage the temperature reading is at:
                                        //   0:  Idle, waiting for next read time
                                        //   1:  Ready to read
                                        //   2:  Read instruction sent, waiting for read to complete
byte readingAddr[8];                    // Unique address of the DS18B20
byte readingData[9];                    // Data retrieved from the DS18B20
unsigned long readingID = 0;            // Unique ID assigned to each reading
int readingCursor = -1;                 // Keeps track of where the latest reading is stored
byte readings[MAXREADINGS][10];         // Stores readings before they are transmitted
                                        //   MSB
                                        //     4 bytes for ID
                                        //     2 bytes for reading
                                        //     4 bytes for timestamp
                                        //   LSB

// Status:
boolean statusOn = false;
unsigned long lastStatusChange = 0;

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

unsigned long extractLongFromByteArray(byte *fromArray, int startPos)
{
  unsigned long extractLong = 0;
  for (int i = 0; i < sizeof(long); i++)
  {
    extractLong += (unsigned long)(fromArray[i + startPos]) << (i * 8);
  }
  return extractLong;
}

// Main program:

void setup()
{
  pinMode(PIN_STATUSGREEN, OUTPUT);     // Initialise the status LED (green mode)
  pinMode(PIN_STATUSRED, OUTPUT);       // Initialise the status LED (red mode)
  digitalWrite(PIN_STATUSRED, HIGH);
  
  // getNodeID();                       // Eventually this will be for reading the node ID set by the DIP switch
  // getNodeName();                     // Eventually this will be for retrieving the name stored in EEPROM

  rf12_initialize(nodeID, RF12_433MHZ); // Initialise the RFM12B
}

void loop()
{
  processRecv();                        // Manage any data received from the master
  processSend();                        // Manage any data to be sent to the master
  processRead();                        // Periodically take readings from DS18B20
  processStatus();                      // Periodically flash the status LED
}

// Subroutines:

void processRecv()
{
  if (rf12_recvDone() && rf12_crc == 0) // && !needToSend
                                        // Check if a packet has been received from the master
  {
    lastRecvTime = millis();
    getRecvData();                      // If so, get the data
    getRecvAction();                    // Analyse the data to determine action required
    processRecvAction();                // Take required action
  }
}

void getRecvData()
{
  tfrByteArray(recvData, lastRecvData, sizeof(recvData));
                                        // Store the last data received
  clrByteArray(recvData, sizeof(recvData));
                                        // Clear variable in preparation for reading in new data - don't really need to do this, but we should in case future processes need it

  for (int i = 0; i < rf12_len; i++)
  {
    recvData[i] = rf12_data[i];         // Read each byte received
  }
}

void getRecvAction()
{
  if (recvData == lastRecvData)
  {
    recvAction = 'R';                   // If the master is resending the last request, resend the last packet back to the master
  }
  else if (!initialised)
  {
    recvAction = 'I';                   // If the node has just been switched on (or reset), send initialisation details to the master
  }
  else
  {
    recvAction = recvData[0];           // If the request is unique and the node is initialised, retrieve the action code from the data received from the master
  }
}

void processRecvAction()
{
  switch(recvAction)
  {
    case 'I':
      sendInit();                       // Send initialisation details to the master
      break;
    case 'P':
      sendReading();                    // Send a reading to the master
      break;
    case 'N':
      // changeName();                  // Eventually this will be for changing the name stored in EEPROM
      break;
    case 'R':
      sendLast();                       // Resend last packet to the master
      break;
  }
}

void sendInit()
{
  sendData[0] = 'I';                    // Tell the master this is an initialisation packet
  sendData[1] = 'T';                    // Tell the master this is a temperature node - should combine this into the byte above; should also define all of these packet headers as byte constants
  addToByteArray(sendData, 2, (byte*)nodeName, strlen(nodeName) - 1);
                                        // Send the name of the node to the master
  sendDataLen = 2 + strlen(nodeName);   //
  needToSend = true;                    // Prepare to send packet
  initialised = true;                   // The node has been initialised; from now on only send initialisation details if the master requests them
}

void sendReading()
{
  // Need to put something here (or in getRecvAction) to see if we got an ack for the last reading transmission. If not, we should just resend the last packet.
  if (sendCursor == readingCursor)
  {
    sendData[0] = 'E';                  // If we have already transmitted the latest reading, tell the master we have no more data to send (end transmission)
    sendDataLen = 1;
  }
  else
  {
    if (sendCursor == -1 || lastRecvData[0] == 'P')
    {
      sendCursor = (sendCursor + 1) % MAXREADINGS;
                                        //
    }
    sendData[0] = 'R';                  // Tell the master this is a packet containing a reading
    addToByteArray(sendData, 1, readings[sendCursor], 9);
                                        // Send the earliest untransmitted reading to the master
    sendDataLen = 11;                   //
  }
  needToSend = true;                    // Prepare to send packet
}

void sendLast()
{
  needToSend = true;                    // Prepare to send packet; this is all we need to do, as we are resending data that has already been defined
}

void processSend()
{
  if (needToSend && rf12_canSend())     // Make sure that we have data to send and that the airwaves are free
  {
    tfrByteArray(sendData, sendDataFinal, sizeof(sendData));

    if (sendDataFinal[0] == 'R')
    {
      subtractLongFromByteArray(sendDataFinal, 7, millis());
    }
    
    byte header = 0;
    header |= RF12_HDR_DST | (byte)MASTERID;
    rf12_sendStart(header, sendDataFinal, sendDataLen);
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
  
  incrementReadingCursor();
  addLongToByteArray(readings[readingCursor], 6, millis());
  addToByteArray(readings[readingCursor], 4, readingData, 1);
  addLongToByteArray(readings[readingCursor], 0, readingID);
  readingID++;
  
  ds.reset_search();
  readStatus = 0;
}

void incrementReadingCursor()
{
  readingCursor = (readingCursor + 1) % MAXREADINGS;
  if (readingCursor == sendCursor)
  {
    sendCursor = (sendCursor + 1) % MAXREADINGS;
  }
}

void processStatus()
{
  unsigned long statusChangeElapsed = millis() - lastStatusChange;

  if (statusOn && statusChangeElapsed >= STATUSONTIME)
  {
    digitalWrite(PIN_STATUSGREEN, LOW);
    digitalWrite(PIN_STATUSRED, LOW);
    statusOn = false;
    lastStatusChange = millis();
  }

  if (!statusOn && statusChangeElapsed >= STATUSOFFTIME)
  {
    if (readingCursor == -1)
    {
      digitalWrite(PIN_STATUSRED, HIGH);
    }
    else if (sendCursor == -1 || millis() - lastRecvTime > MINPOLLCYCLETIME * 1.5)
    {
      digitalWrite(PIN_STATUSRED, HIGH);
      statusOn = true;
    }
    else
    {
      digitalWrite(PIN_STATUSGREEN, HIGH);
      statusOn = true;
    }
    lastStatusChange = millis();
  }
}
