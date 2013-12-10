/****************************************************************************************************

*****************************************************************************************************/

// Libraries:
#include <RF12.h>                       // Library for RF communication (RFM12B) - needs revising
#include <Ports.h>                      // Sub-library of RF12 (can probably be removed upon revision of RF12 library)
#include <LiquidCrystal.h>

// Pins:
const int PIN_LCD[] = {8, 7, 6, 5, 4, 3};

// Constants:
const int MASTERID = 31;                // 

// Initialisation:
LiquidCrystal lcd(PIN_LCD[0], PIN_LCD[1], PIN_LCD[2], PIN_LCD[3], PIN_LCD[4], PIN_LCD[5]);
boolean initialised = false;            // The first transmission needs to include initialisation details such as node name
int nodeID = 30;                        // The ID of the node (1 to 30) - should be set by a DIP switch and read during setup()
char nodeName[15] = "Display";          // The name of the node - should be stored in EEPROM and editable via master
                                        // In the meantime, current hard-coded nodes are:
                                        //   1: Bedroom
                                        //   2: Nursery
                                        //   3: Outside

// Incoming Packets:
unsigned long recvTime;                 //
byte recvData[RF12_MAXDATA];            // Data received from the master
byte nodeData[MAXPOLLID][22];           // Stores up-to-date information about each node

// Outgoing Packets:
byte sendData[RF12_MAXDATA];            // Data to be sent to the master
int sendDataLen;                        // Stores the actual number of bytes to be sent to the master
boolean needToSend = false;             // Set to true when data is ready to be sent to the master

// LCD:

// Special byte array functions:

void tfrByteArray(byte *fromArray, byte *toArray)
{
  for (int i = 0; i < sizeof(toArray); i++)
  {
    if (i < sizeof(fromArray))
    {
      toArray[i] = fromArray[i];
    }
    else
    {
      toArray[i] = 0;
    }
  }
}

void clrByteArray(byte *clearArray)
{
  for (int i = 0; i < sizeof(clearArray); i++)
  {
    clearArray[i] = 0;
  }
}

void addToByteArray(byte *toArray, int startPos, byte *addArray, int fromPos = 0, int toPos = -1)
{
  if (toPos == -1)
  {
    toPos = sizeof(addArray) - 1;
  }
  for (int i = fromPos; i <= toPos && i < sizeof(toArray) + startPos; i++)
  {
    toArray[i + startPos] = addArray[i];
  }
}

void addLongToByteArray(byte *toArray, int startPos, unsigned long addLong)
{
  for (int i = 0; i < sizeof(long) && i < sizeof(toArray) + startPos; i++)
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
  unsigned long extractLong;
  for (int i = 0; i < sizeof(long) && i < sizeof(fromArray) + startPos; i++)
  {
    extractLong += fromArray[i] << (i * 8);
  }
  return extractLong;
}

// Main program:

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  rf12_initialize(nodeID, RF12_433MHZ); // Initialise the RFM12B
  
  // getNodeID();                       // Eventually this will be for reading the node ID set by the DIP switch
}

void loop()
{
  processRecv();                        // Manage any data received from the master
  processSend();                        // Manage any data to be sent to the master
  processLCD();                         // 
}

// Subroutines:

void processRecv()
{
  if (rf12_recvDone() && rf12_crc == 0) // Check if a packet has been received from the master
  {
    getRecvData();                      // If so, get the data
    getRecvAction();                    // Analyse the data to determine action required
    processRecvAction();                // Take required action
  }
}

void getRecvData()
{
  tfrByteArray(recvData, lastRecvData); // Store the last data received
  clrByteArray(recvData);               // Clear variable in preparation for reading in new data - don't really need to do this, but we should in case future processes need it
  
  for (int i = 0; i < rf12_len; ++i)
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

    lcd.setCursor(0, 0);
    lcd.print("Connected.      ");
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
  addToByteArray(sendData, 2, (byte*)nodeName);
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
    sendCursor = (sendCursor + 1) % MAXREADINGS;
                                        //
    sendData[0] = 'R';                  // Tell the master this is a packet containing a reading
    addToByteArray(sendData, 1, readings[sendCursor]);
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
    if (sendData[0] == 'R')
    {
      subtractLongFromByteArray(sendData, 7, millis());
    }
    
    rf12_sendStart(MASTERID, sendData, sendDataLen);
    needToSend = false;
  }
}

void processLCD()
{
}
