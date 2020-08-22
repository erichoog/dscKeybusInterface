/*
    DSC Keybus Interface

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dscKeybusInterface.h"

uint8_t dscKeybusInterface::dscClockPin;
uint8_t dscKeybusInterface::dscReadPin;
uint8_t dscKeybusInterface::dscWritePin;
char dscKeybusInterface::writeKey;
uint8_t dscKeybusInterface::writePartition;
uint8_t dscKeybusInterface::writeByte;
uint8_t dscKeybusInterface::writeBit;
bool dscKeybusInterface::virtualKeypad;
bool dscKeybusInterface::processModuleData;
uint8_t dscKeybusInterface::panelData[dscReadSize];
uint8_t dscKeybusInterface::panelByteCount;
uint8_t dscKeybusInterface::panelBitCount;
volatile bool dscKeybusInterface::writeReady;
volatile uint8_t dscKeybusInterface::moduleData[dscReadSize];
volatile bool dscKeybusInterface::moduleDataCaptured;
volatile uint8_t dscKeybusInterface::moduleByteCount;
volatile uint8_t dscKeybusInterface::moduleBitCount;
volatile bool dscKeybusInterface::writeAlarm;
volatile bool dscKeybusInterface::writeAsterisk;
volatile bool dscKeybusInterface::wroteAsterisk;
volatile bool dscKeybusInterface::bufferOverflow;
volatile uint8_t dscKeybusInterface::panelBufferLength;
volatile uint8_t dscKeybusInterface::panelBuffer[dscBufferSize][dscReadSize];
volatile uint8_t dscKeybusInterface::panelBufferBitCount[dscBufferSize];
volatile uint8_t dscKeybusInterface::panelBufferByteCount[dscBufferSize];
volatile uint8_t dscKeybusInterface::isrPanelData[dscReadSize];
volatile uint8_t dscKeybusInterface::isrPanelByteCount;
volatile uint8_t dscKeybusInterface::isrPanelBitCount;
volatile uint8_t dscKeybusInterface::isrPanelBitTotal;
volatile uint8_t dscKeybusInterface::isrModuleData[dscReadSize];
volatile uint8_t dscKeybusInterface::isrModuleByteCount;
volatile uint8_t dscKeybusInterface::isrModuleBitCount;
volatile uint8_t dscKeybusInterface::isrModuleBitTotal;
volatile uint8_t dscKeybusInterface::currentCmd;
volatile uint8_t dscKeybusInterface::statusCmd;
volatile unsigned long dscKeybusInterface::clockHighTime;
volatile unsigned long dscKeybusInterface::keybusTime;


dscKeybusInterface::dscKeybusInterface(uint8_t setClockPin, uint8_t setReadPin, uint8_t setWritePin) {
  dscClockPin = setClockPin;
  dscReadPin = setReadPin;
  dscWritePin = setWritePin;
  if (dscWritePin != 255) virtualKeypad = true;
  writeReady = true;
  processRedundantData = true;
  displayTrailingBits = false;
  processModuleData = false;
  writePartition = 1;
}


void dscKeybusInterface::begin() {

  struct gpiod_chip *output_chip;
	struct gpiod_line *output_line;

  pinMode(dscClockPin, INPUT);
  pinMode(dscReadPin, INPUT);
  if (virtualKeypad) pinMode(dscWritePin, OUTPUT);
  // stream = &_stream;

  // Platform-specific timers trigger a read of the data line 250us after the Keybus clock changes

  // Arduino Timer1 calls ISR(TIMER1_OVF_vect) from dscClockInterrupt() and is disabled in the ISR for a one-shot timer
  #if defined(__AVR__)
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 |= (1 << TOIE1);

  // esp8266 timer1 calls dscDataInterrupt() from dscClockInterrupt() as a one-shot timer
  #elif defined(ESP8266)
  timer1_isr_init();
  timer1_attachInterrupt(dscDataInterrupt);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  #endif

  // Generates an interrupt when the Keybus clock rises or falls - requires a hardware interrupt pin on Arduino
  attachInterrupt(digitalPinToInterrupt(dscClockPin), dscClockInterrupt, CHANGE);
}


bool dscKeybusInterface::handlePanel() {

  // Checks if Keybus data is detected and sets a status flag if data is not detected for 3s
  noInterrupts();
  if (millis() - keybusTime > 3000) keybusConnected = false;  // dataTime is set in dscDataInterrupt() when the clock resets
  else keybusConnected = true;
  interrupts();
  if (previousKeybus != keybusConnected) {
    previousKeybus = keybusConnected;
    keybusChanged = true;
    statusChanged = true;
    if (!keybusConnected) return true;
  }

  // Writes keys when multiple keys are sent as a char array
  if (writeKeysPending) writeKeys(writeKeysArray);

  // Skips processing if the panel data buffer is empty
  if (panelBufferLength == 0) return false;

  // Copies data from the buffer to panelData[]
  static uint8_t panelBufferIndex = 1;
  uint8_t dataIndex = panelBufferIndex - 1;
  for (uint8_t i = 0; i < dscReadSize; i++) panelData[i] = panelBuffer[dataIndex][i];
  panelBitCount = panelBufferBitCount[dataIndex];
  panelByteCount = panelBufferByteCount[dataIndex];
  panelBufferIndex++;

  // Resets counters when the buffer is cleared
  noInterrupts();
  if (panelBufferIndex > panelBufferLength) {
    panelBufferIndex = 1;
    panelBufferLength = 0;
  }
  interrupts();

  // Waits at startup for the 0x05 status command or a command with valid CRC data to eliminate spurious data.
  static bool firstClockCycle = true;
  if (firstClockCycle) {
    if (validCRC() || panelData[0] == 0x05) firstClockCycle = false;
    else return false;
  }

  // Skips redundant data sent constantly while in installer programming
  static uint8_t previousCmd0A[dscReadSize];
  static uint8_t previousCmdE6_20[dscReadSize];
  switch (panelData[0]) {
    case 0x0A:  // Status in programming
      if (redundantPanelData(previousCmd0A, panelData)) return false;
      break;

    case 0xE6:
      if (panelData[2] == 0x20 && redundantPanelData(previousCmdE6_20, panelData)) return false;  // Status in programming, zone lights 33-64
      break;
  }
  if (dscPartitions > 4) {
    static uint8_t previousCmdE6_03[dscReadSize];
    if (panelData[0] == 0xE6 && panelData[2] == 0x03 && redundantPanelData(previousCmdE6_03, panelData, 8)) return false;  // Status in alarm/programming, partitions 5-8
  }

  // Skips redundant data from periodic commands sent at regular intervals, skipping is a configurable
  // option and the default behavior to help see new Keybus data when decoding the protocol
  if (!processRedundantData) {
    static uint8_t previousCmd11[dscReadSize];
    static uint8_t previousCmd16[dscReadSize];
    static uint8_t previousCmd27[dscReadSize];
    static uint8_t previousCmd2D[dscReadSize];
    static uint8_t previousCmd34[dscReadSize];
    static uint8_t previousCmd3E[dscReadSize];
    static uint8_t previousCmd5D[dscReadSize];
    static uint8_t previousCmd63[dscReadSize];
    static uint8_t previousCmdB1[dscReadSize];
    static uint8_t previousCmdC3[dscReadSize];
    switch (panelData[0]) {
      case 0x11:  // Keypad slot query
        if (redundantPanelData(previousCmd11, panelData)) return false;
        break;

      case 0x16:  // Zone wiring
        if (redundantPanelData(previousCmd16, panelData)) return false;
        break;

      case 0x27:  // Status with zone 1-8 info
        if (redundantPanelData(previousCmd27, panelData)) return false;
        break;

      case 0x2D:  // Status with zone 9-16 info
        if (redundantPanelData(previousCmd2D, panelData)) return false;
        break;

      case 0x34:  // Status with zone 17-24 info
        if (redundantPanelData(previousCmd34, panelData)) return false;
        break;

      case 0x3E:  // Status with zone 25-32 info
        if (redundantPanelData(previousCmd3E, panelData)) return false;
        break;

      case 0x5D:  // Flash panel lights: status and zones 1-32
        if (redundantPanelData(previousCmd5D, panelData)) return false;
        break;

      case 0x63:  // Flash panel lights: status and zones 33-64
        if (redundantPanelData(previousCmd63, panelData)) return false;
        break;

      case 0xB1:  // Enabled zones 1-32
        if (redundantPanelData(previousCmdB1, panelData)) return false;
        break;

      case 0xC3:  // Unknown command
        if (redundantPanelData(previousCmdC3, panelData)) return false;
        break;
    }
  }

  // Processes valid panel data
  switch (panelData[0]) {
    case 0x05:
    case 0x1B: processPanelStatus(); break;
    case 0x27: processPanel_0x27(); break;
    case 0x2D: processPanel_0x2D(); break;
    case 0x34: processPanel_0x34(); break;
    case 0x3E: processPanel_0x3E(); break;
    case 0xA5: processPanel_0xA5(); break;
    case 0xE6: if (dscPartitions > 2) processPanel_0xE6(); break;
    case 0xEB: if (dscPartitions > 2) processPanel_0xEB(); break;
  }

  return true;
}


bool dscKeybusInterface::handleModule() {
  if (!moduleDataCaptured) return false;
  moduleDataCaptured = false;

  if (moduleBitCount < 8) return false;

  // Skips periodic keypad slot query responses
  if (!processRedundantData && currentCmd == 0x11) {
    bool redundantData = true;
    uint8_t checkedBytes = dscReadSize;
    static uint8_t previousSlotData[dscReadSize];
    for (uint8_t i = 0; i < checkedBytes; i++) {
      if (previousSlotData[i] != moduleData[i]) {
        redundantData = false;
        break;
      }
    }
    if (redundantData) return false;
    else {
      for (uint8_t i = 0; i < dscReadSize; i++) previousSlotData[i] = moduleData[i];
      return true;
    }
  }

  // Determines if a keybus message is a response to a panel command
  switch (currentCmd) {
    case 0x11:
    case 0x28:
    case 0xD5: queryResponse = true; break;
    default: queryResponse = false; break;
  }

  return true;
}


// Sets up writes if multiple keys are sent as a char array
void dscKeybusInterface::write(const char * receivedKeys) {
  writeKeysArray = receivedKeys;
  if (writeKeysArray[0] != '\0') writeKeysPending = true;
  writeKeys(writeKeysArray);
}


// Writes multiple keys from a char array
void dscKeybusInterface::writeKeys(const char * writeKeysArray) {
  static uint8_t writeCounter = 0;
  if (writeKeysPending && writeReady && writeCounter < strlen(writeKeysArray)) {
    if (writeKeysArray[writeCounter] != '\0') {
      write(writeKeysArray[writeCounter]);
      writeCounter++;
      if (writeKeysArray[writeCounter] == '\0') {
        writeKeysPending = false;
        writeCounter = 0;
      }
    }
  }
}


// Specifies the key value to be written by dscClockInterrupt() and selects the write partition.  This includes a 500ms
// delay after alarm keys to resolve errors when additional keys are sent immediately after alarm keys.
void dscKeybusInterface::write(const char receivedKey) {
  static unsigned long previousTime;
  static bool setPartition;
  // Sets the write partition if set by virtual keypad key '/'
  if (setPartition) {
    setPartition = false;
    if (receivedKey >= '1' && receivedKey <= '8') {
      writePartition = receivedKey - 48;
    }
    return;
  }

  // Sets the binary to write for virtual keypad keys
  if (writeReady && millis() - previousTime > 500) {
    bool validKey = true;
    switch (receivedKey) {
      case '/': setPartition = true; validKey = false; break;
      case '0': writeKey = 0x00; break;
      case '1': writeKey = 0x05; break;
      case '2': writeKey = 0x0A; break;
      case '3': writeKey = 0x0F; break;
      case '4': writeKey = 0x11; break;
      case '5': writeKey = 0x16; break;
      case '6': writeKey = 0x1B; break;
      case '7': writeKey = 0x1C; break;
      case '8': writeKey = 0x22; break;
      case '9': writeKey = 0x27; break;
      case '*': writeKey = 0x28; writeAsterisk = true; break;
      case '#': writeKey = 0x2D; break;
      case 'F':
      case 'f': writeKey = 0x77; writeAlarm = true; break;                    // Keypad fire alarm
      case 's':
      case 'S': writeKey = 0xAF; writeArm[writePartition - 1] = true; break;  // Arm stay
      case 'w':
      case 'W': writeKey = 0xB1; writeArm[writePartition - 1] = true; break;  // Arm away
      case 'n':
      case 'N': writeKey = 0xB6; writeArm[writePartition - 1] = true; break;  // Arm with no entry delay (night arm)
      case 'A':
      case 'a': writeKey = 0xBB; writeAlarm = true; break;                    // Keypad auxiliary alarm
      case 'c':
      case 'C': writeKey = 0xBB; break;                                       // Door chime
      case 'r':
      case 'R': writeKey = 0xDA; break;                                       // Reset
      case 'P':
      case 'p': writeKey = 0xDD; writeAlarm = true; break;                    // Keypad panic alarm
      case 'x':
      case 'X': writeKey = 0xE1; break;                                       // Exit
      case '[': writeKey = 0xD5; break;                                       // Command output 1
      case ']': writeKey = 0xDA; break;                                       // Command output 2
      case '{': writeKey = 0x70; break;                                       // Command output 3
      case '}': writeKey = 0xEC; break;                                       // Command output 4
      default: {
        validKey = false;
        break;
      }
    }

    // Sets the writing position in dscClockInterrupt() for the currently set partition
    if (dscPartitions < writePartition) writePartition = 1;
    switch (writePartition) {
      case 1:
      case 5: {
        writeByte = 2;
        writeBit = 9;
        break;
      }
      case 2:
      case 6: {
        writeByte = 3;
        writeBit = 17;
        break;
      }
      case 3:
      case 7: {
        writeByte = 8;
        writeBit = 57;
        break;
      }
      case 4:
      case 8: {
        writeByte = 9;
        writeBit = 65;
        break;
      }
      default: {
        writeByte = 2;
        writeBit = 9;
        break;
      }
    }

    if (writeAlarm) previousTime = millis();  // Sets a marker to time writes after keypad alarm keys
    if (validKey) writeReady = false;         // Sets a flag indicating that a write is pending, cleared by dscClockInterrupt()
  }
}


bool dscKeybusInterface::redundantPanelData(uint8_t previousCmd[], volatile uint8_t currentCmd[], uint8_t checkedBytes) {
  bool redundantData = true;
  for (uint8_t i = 0; i < checkedBytes; i++) {
    if (previousCmd[i] != currentCmd[i]) {
      redundantData = false;
      break;
    }
  }
  if (redundantData) return true;
  else {
    for (uint8_t i = 0; i < dscReadSize; i++) previousCmd[i] = currentCmd[i];
    return false;
  }
}


bool dscKeybusInterface::validCRC() {
  uint8_t byteCount = (panelBitCount - 1) / 8;
  int dataSum = 0;
  for (uint8_t panelByte = 0; panelByte < byteCount; panelByte++) {
    if (panelByte != 1) dataSum += panelData[panelByte];
  }
  if (dataSum % 256 == panelData[byteCount]) return true;
  else return false;
}


// Called as an interrupt when the DSC clock changes to write data for virtual keypad and setup timers to read
// data after an interval.
#if defined(__AVR__)
void dscKeybusInterface::dscClockInterrupt() {
#elif defined(ESP8266)
void ICACHE_RAM_ATTR dscKeybusInterface::dscClockInterrupt() {
#endif

  // Data sent from the panel and keypads/modules has latency after a clock change (observed up to 160us for keypad data).
  // The following sets up a timer for both Arduino/AVR and Arduino/esp8266 that will call dscDataInterrupt() in
  // 250us to read the data line.

  // AVR Timer1 calls dscDataInterrupt() via ISR(TIMER1_OVF_vect) when the Timer1 counter overflows
  #if defined(__AVR__)
  TCNT1=61535;            // Timer1 counter start value, overflows at 65535 in 250us
  TCCR1B |= (1 << CS10);  // Sets the prescaler to 1

  // esp8266 timer1 calls dscDataInterrupt() directly as set in begin()
  #elif defined(ESP8266)
  timer1_write(1250);
  #endif


  static unsigned long previousClockHighTime;
  if (digitalRead(dscClockPin) == HIGH) {
    if (virtualKeypad) digitalWrite(dscWritePin, LOW);  // Restores the data line after a virtual keypad write
    previousClockHighTime = micros();
  }

  else {
    clockHighTime = micros() - previousClockHighTime;  // Tracks the clock high time to find the reset between commands

    // Virtual keypad
    if (virtualKeypad) {
      static bool writeStart = false;
      static bool writeRepeat = false;
      static bool writeCmd = false;

      if (writePartition <= 4 && statusCmd == 0x05) writeCmd = true;
      else if (writePartition > 4 && statusCmd == 0x1B) writeCmd = true;
      else writeCmd = false;

      // Writes a F/A/P alarm key and repeats the key on the next immediate command from the panel (0x1C verification)
      if ((writeAlarm && !writeReady) || writeRepeat) {

        // Writes the first bit by shifting the alarm key data right 7 bits and checking bit 0
        if (isrPanelBitTotal == 1) {
          if (!((writeKey >> 7) & 0x01)) {
            digitalWrite(dscWritePin, HIGH);
          }
          writeStart = true;  // Resolves a timing issue where some writes do not begin at the correct bit
        }

        // Writes the remaining alarm key data
        else if (writeStart && isrPanelBitTotal > 1 && isrPanelBitTotal <= 8) {
          if (!((writeKey >> (8 - isrPanelBitTotal)) & 0x01)) digitalWrite(dscWritePin, HIGH);
          // Resets counters when the write is complete
          if (isrPanelBitTotal == 8) {
            writeReady = true;
            writeStart = false;
            writeAlarm = false;

            // Sets up a repeated write for alarm keys
            if (!writeRepeat) writeRepeat = true;
            else writeRepeat = false;
          }
        }
      }

      // Writes a regular key unless waiting for a response to the '*' key or the panel is sending a query command
      else if (!writeReady && !wroteAsterisk && isrPanelByteCount == writeByte && writeCmd) {
        // Writes the first bit by shifting the key data right 7 bits and checking bit 0
        if (isrPanelBitTotal == writeBit) {
          if (!((writeKey >> 7) & 0x01)) digitalWrite(dscWritePin, HIGH);
          writeStart = true;  // Resolves a timing issue where some writes do not begin at the correct bit
        }

        // Writes the remaining alarm key data
        else if (writeStart && isrPanelBitTotal > writeBit && isrPanelBitTotal <= writeBit + 7) {
          if (!((writeKey >> (7 - isrPanelBitCount)) & 0x01)) digitalWrite(dscWritePin, HIGH);

          // Resets counters when the write is complete
          if (isrPanelBitTotal == writeBit + 7) {
            if (writeAsterisk) wroteAsterisk = true;  // Delays writing after pressing '*' until the panel is ready
            else writeReady = true;
            writeStart = false;
          }
        }
      }
    }
  }
}

// Interrupt function called by AVR Timer1 and esp8266 timer1 after 250us to read the data line
#if defined(__AVR__)
void dscKeybusInterface::dscDataInterrupt() {
#elif defined(ESP8266)
void ICACHE_RAM_ATTR dscKeybusInterface::dscDataInterrupt() {
#endif

  static bool skipData = false;

  // Panel sends data while the clock is high
  if (digitalRead(dscClockPin) == HIGH) {

    // Stops processing Keybus data at the dscReadSize limit
    if (isrPanelByteCount >= dscReadSize) skipData = true;

    else {
      if (isrPanelBitCount < 8) {
        // Data is captured in each byte by shifting left by 1 bit and writing to bit 0
        isrPanelData[isrPanelByteCount] <<= 1;
        if (digitalRead(dscReadPin) == HIGH) {
          isrPanelData[isrPanelByteCount] |= 1;
        }
      }

      if (isrPanelBitTotal == 8) {
        // Tests for a status command, used in dscClockInterrupt() to ensure keys are only written during a status command
        switch (isrPanelData[0]) {
          case 0x05:
          case 0x0A: statusCmd = 0x05; break;
          case 0x1B: statusCmd = 0x1B; break;
          default: statusCmd = 0; break;
        }

        // Stores the stop bit by itself in byte 1 - this aligns the Keybus bytes with panelData[] bytes
        isrPanelBitCount = 0;
        isrPanelByteCount++;
      }

      // Increments the bit counter if the byte is incomplete
      else if (isrPanelBitCount < 7) {
        isrPanelBitCount++;
      }

      // Byte is complete, set the counters for the next byte
      else {
        isrPanelBitCount = 0;
        isrPanelByteCount++;
      }

      isrPanelBitTotal++;
    }
  }

  // Keypads and modules send data while the clock is low
  else {
    static bool moduleDataDetected = false;

    // Keypad and module data is not buffered and skipped if the panel data buffer is filling
    if (processModuleData && isrModuleByteCount < dscReadSize && panelBufferLength <= 1) {

      // Data is captured in each byte by shifting left by 1 bit and writing to bit 0
      if (isrModuleBitCount < 8) {
        isrModuleData[isrModuleByteCount] <<= 1;
        if (digitalRead(dscReadPin) == HIGH) {
          isrModuleData[isrModuleByteCount] |= 1;
        }
        else moduleDataDetected = true;  // Keypads and modules send data by pulling the data line low
      }

      // Stores the stop bit by itself in byte 1 - this aligns the Keybus bytes with moduleData[] bytes
      if (isrModuleBitTotal == 7) {
        isrModuleData[1] = 1;  // Sets the stop bit manually to 1 in byte 1
        isrModuleBitCount = 0;
        isrModuleByteCount += 2;
      }

      // Increments the bit counter if the byte is incomplete
      else if (isrModuleBitCount < 7) {
        isrModuleBitCount++;
      }

      // Byte is complete, set the counters for the next byte
      else {
        isrModuleBitCount = 0;
        isrModuleByteCount++;
      }

      isrModuleBitTotal++;
    }

    // Saves data and resets counters after the clock cycle is complete (high for at least 1ms)
    if (clockHighTime > 1000) {
      keybusTime = millis();

      // Skips incomplete and redundant data from status commands - these are sent constantly on the keybus at a high
      // rate, so they are always skipped.  Checking is required in the ISR to prevent flooding the buffer.
      if (isrPanelBitTotal < 8) skipData = true;
      else switch (isrPanelData[0]) {
        static uint8_t previousCmd05[dscReadSize];
        static uint8_t previousCmd1B[dscReadSize];
        case 0x05:  // Status: partitions 1-4
          if (redundantPanelData(previousCmd05, isrPanelData, isrPanelByteCount)) skipData = true;
          break;

        case 0x1B:  // Status: partitions 5-8
          if (redundantPanelData(previousCmd1B, isrPanelData, isrPanelByteCount)) skipData = true;
          break;
      }

      // Stores new panel data in the panel buffer
      currentCmd = isrPanelData[0];
      if (panelBufferLength == dscBufferSize) bufferOverflow = true;
      else if (!skipData && panelBufferLength < dscBufferSize) {
        for (uint8_t i = 0; i < dscReadSize; i++) panelBuffer[panelBufferLength][i] = isrPanelData[i];
        panelBufferBitCount[panelBufferLength] = isrPanelBitTotal;
        panelBufferByteCount[panelBufferLength] = isrPanelByteCount;
        panelBufferLength++;
      }

      // Resets the panel capture data and counters
      for (uint8_t i = 0; i < dscReadSize; i++) isrPanelData[i] = 0;
      isrPanelBitTotal = 0;
      isrPanelBitCount = 0;
      isrPanelByteCount = 0;
      skipData = false;

      if (processModuleData) {

        // Stores new keypad and module data - this data is not buffered
        if (moduleDataDetected) {
          moduleDataDetected = false;
          moduleDataCaptured = true;  // Sets a flag for handleModules()
          for (uint8_t i = 0; i < dscReadSize; i++) moduleData[i] = isrModuleData[i];
          moduleBitCount = isrModuleBitTotal;
          moduleByteCount = isrModuleByteCount;
        }

        // Resets the keypad and module capture data and counters
        for (uint8_t i = 0; i < dscReadSize; i++) isrModuleData[i] = 0;
        isrModuleBitTotal = 0;
        isrModuleBitCount = 0;
        isrModuleByteCount = 0;
      }
    }
  }
}

bool bitRead(uint8_t byte_to_read, int position_from_right) {
  return (byte_to_read >> position_from_right) & 1;
}

void bitWrite(uint8_t byte_to_write, uint8_t position_from_right, int bitValue) {
    int mask = 1 << position_from_right; 
    return (byte_to_write & ~mask) | ((bitValue << position_from_right) & mask); 
}

uint64_t millis() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
