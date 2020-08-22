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

#ifndef dscKeybusInterface_h
#define dscKeybusInterface_h

// #include <Arduino.h>
#include <stdint.h>
#include <linux/gpio.h>
#include <string.h>
#include <iostream>
#include <chrono>


// #if defined(__AVR__)
const uint8_t dscPartitions = 4;   // Maximum number of partitions - requires 19 bytes of memory per partition
const uint8_t dscZones = 8;        // Maximum number of zone groups, 8 zones per group - requires 6 bytes of memory per zone group
const uint8_t dscBufferSize = 10;  // Number of commands to buffer if the sketch is busy - requires dscReadSize + 2 bytes of memory per command

// #elif defined(ESP8266)
// const uint8_t dscPartitions = 8;
// const uint8_t dscZones = 8;
// const uint8_t dscBufferSize = 50;
// #endif

const uint8_t dscReadSize = 16;   // Maximum size of a Keybus command


class dscKeybusInterface {

  public:

    // Initializes writes as disabled by default
    dscKeybusInterface(uint8_t setClockPin, uint8_t setReadPin, uint8_t setWritePin = 255);

    void begin();                                     // Initializes the stream output to stdout by default
    bool handlePanel();                               // Returns true if valid panel data is available
    bool handleModule();                              // Returns true if valid keypad or module data is available
    static volatile bool writeReady;                  // True if the library is ready to write a key
    void write(const char receivedKey);               // Writes a single key
    void write(const char * receivedKeys);            // Writes multiple keys from a char array
    void printPanelBinary(bool printSpaces = true);   // Includes spaces between bytes by default
    void printPanelCommand();                         // Prints the panel command as hex
    void printPanelMessage();                         // Prints the decoded panel message
    void printModuleBinary(bool printSpaces = true);  // Includes spaces between bytes by default
    void printModuleMessage();                        // Prints the decoded keypad or module message

    // Set to a partition number for virtual keypad
    static uint8_t writePartition;

    // These can be configured in the sketch setup() before begin()
    bool hideKeypadDigits;          // Controls if keypad digits are hidden for publicly posted logs (default: false)
    bool processRedundantData;      // Controls if repeated periodic commands are processed and displayed (default: false)
    static bool processModuleData;  // Controls if keypad and module data is processed and displayed (default: false)
    bool displayTrailingBits;       // Controls if bits read as the clock is reset are displayed, appears to be spurious data (default: false)

    // Panel time
    bool timeAvailable;             // True after the panel sends the first timestamped message
    uint8_t hour, minute, day, month;
    int year;

    // These contain the current LED state and status message for each partition based on command 0x05 for
    // partitions 1-4 and command 0x1B for partitions 5-8.  See printPanelLights() and printPanelMessages()
    // in dscKeybusPrintData.cpp to see how this data translates to the LED status and status message.
    uint8_t status[dscPartitions];
    uint8_t lights[dscPartitions];

    // Status tracking
    bool statusChanged;                   // True after any status change
    bool keybusConnected, keybusChanged;  // True if data is detected on the Keybus
    bool accessCodePrompt;                // True if the panel is requesting an access code
    bool trouble, troubleChanged;
    bool powerTrouble, powerChanged;
    bool batteryTrouble, batteryChanged;
    bool keypadFireAlarm, keypadAuxAlarm, keypadPanicAlarm;
    bool ready[dscPartitions], readyChanged[dscPartitions];
    bool armed[dscPartitions], armedAway[dscPartitions], armedStay[dscPartitions];
    bool noEntryDelay[dscPartitions], armedChanged[dscPartitions];
    bool alarm[dscPartitions], alarmChanged[dscPartitions];
    bool exitDelay[dscPartitions], exitDelayChanged[dscPartitions];
    bool entryDelay[dscPartitions], entryDelayChanged[dscPartitions];
    bool fire[dscPartitions], fireChanged[dscPartitions];
    bool openZonesStatusChanged;
    uint8_t openZones[dscZones], openZonesChanged[dscZones];    // Zone status is stored in an array using 1 bit per zone, up to 64 zones
    bool alarmZonesStatusChanged;
    uint8_t alarmZones[dscZones], alarmZonesChanged[dscZones];  // Zone alarm status is stored in an array using 1 bit per zone, up to 64 zones

    // Panel and keypad data is stored in an array: command [0], stop bit by itself [1], followed by the remaining
    // data.  panelData[] and moduleData[] can be accessed directly within the sketch.
    //
    // panelData[] example:
    //   Byte 0     Byte 2   Byte 3   Byte 4   Byte 5
    //   00000101 0 10000001 00000001 10010001 11000111 [0x05] Status lights: Ready Backlight | Partition ready
    //            ^ Byte 1 (stop bit)
    static uint8_t panelData[dscReadSize];
    static volatile uint8_t moduleData[dscReadSize];

    // True if dscBufferSize needs to be increased
    static volatile bool bufferOverflow;

    // Timer interrupt function to capture data - declared as public for use by AVR Timer2
    static void dscDataInterrupt();

  private:

    void processPanelStatus();
    void processPanelStatus0(uint8_t partition, uint8_t panelByte);
    void processPanelStatus2(uint8_t partition, uint8_t panelByte);
    void processPanelStatus4(uint8_t partition, uint8_t panelByte);
    void processPanel_0x27();
    void processPanel_0x2D();
    void processPanel_0x34();
    void processPanel_0x3E();
    void processPanel_0xA5();
    void processPanel_0xE6();
    void processPanel_0xE6_0x09();
    void processPanel_0xE6_0x0B();
    void processPanel_0xE6_0x0D();
    void processPanel_0xE6_0x0F();
    void processPanel_0xEB();

    void printPanelLights(uint8_t panelByte);
    void printPanelMessages(uint8_t panelByte);
    void printPanelBitNumbers(uint8_t panelByte, uint8_t startNumber);
    void printPanelStatus0(uint8_t panelByte);
    void printPanelStatus1(uint8_t panelByte);
    void printPanelStatus2(uint8_t panelByte);
    void printPanelStatus3(uint8_t panelByte);
    void printPanelStatus4(uint8_t panelByte);
    void printPanelStatus14(uint8_t panelByte);
    void printPanel_0x05();
    void printPanel_0x0A();
    void printPanel_0x11();
    void printPanel_0x16();
    void printPanel_0x1B();
    void printPanel_0x1C();
    void printPanel_0x27();
    void printPanel_0x28();
    void printPanel_0x2D();
    void printPanel_0x34();
    void printPanel_0x3E();
    void printPanel_0x4C();
    void printPanel_0x58();
    void printPanel_0x5D();
    void printPanel_0x63();
    void printPanel_0x64();
    void printPanel_0x69();
    void printPanel_0x75();
    void printPanel_0x7A();
    void printPanel_0x7F();
    void printPanel_0x82();
    void printPanel_0x87();
    void printPanel_0x8D();
    void printPanel_0x94();
    void printPanel_0xA5();
    void printPanel_0xB1();
    void printPanel_0xBB();
    void printPanel_0xC3();
    void printPanel_0xCE();
    void printPanel_0xD5();
    void printPanel_0xE6();
    void printPanel_0xE6_0x03();
    void printPanel_0xE6_0x09();
    void printPanel_0xE6_0x0B();
    void printPanel_0xE6_0x0D();
    void printPanel_0xE6_0x0F();
    void printPanel_0xE6_0x17();
    void printPanel_0xE6_0x18();
    void printPanel_0xE6_0x19();
    void printPanel_0xE6_0x1A();
    void printPanel_0xE6_0x1D();
    void printPanel_0xE6_0x20();
    void printPanel_0xE6_0x2B();
    void printPanel_0xE6_0x2C();
    void printPanel_0xE6_0x41();
    void printPanel_0xEB();

    void printModule_0x77();
    void printModule_0xBB();
    void printModule_0xDD();
    void printModule_Panel_0x11();
    void printModule_Panel_0xD5();
    void printModule_Notification();
    void printModule_Keys();

    bool validCRC();
    void writeKeys(const char * writeKeysArray);
    static void dscClockInterrupt();
    static bool redundantPanelData(uint8_t previousCmd[], volatile uint8_t currentCmd[], uint8_t checkedBytes = dscReadSize);
    bool bitRead(uint8_t byte_to_read, int position_from_right);
    void bitWrite(uint8_t byte_to_write, uint8_t position_from_right, int bitValue);
    uint64_t millis();

    const char* writeKeysArray;
    bool writeKeysPending;
    bool writeArm[dscPartitions];
    bool queryResponse;
    bool previousTrouble;
    bool previousKeybus;
    uint8_t previousLights[dscPartitions], previousStatus[dscPartitions];
    bool previousReady[dscPartitions];
    bool previousExitDelay[dscPartitions], previousEntryDelay[dscPartitions];
    bool previousArmed[dscPartitions], previousAlarm[dscPartitions];
    bool previousFire[dscPartitions];
    uint8_t previousOpenZones[dscZones], previousAlarmZones[dscZones];

    static uint8_t dscClockPin;
    static uint8_t dscReadPin;
    static uint8_t dscWritePin;
    static uint8_t writeByte, writeBit;
    static bool virtualKeypad;
    static char writeKey;
    static uint8_t panelBitCount, panelByteCount;
    static volatile bool writeAlarm, writeAsterisk, wroteAsterisk;
    static volatile bool moduleDataCaptured;
    static volatile unsigned long clockHighTime, keybusTime;
    static volatile uint8_t panelBufferLength;
    static volatile uint8_t panelBuffer[dscBufferSize][dscReadSize];
    static volatile uint8_t panelBufferBitCount[dscBufferSize], panelBufferByteCount[dscBufferSize];
    static volatile uint8_t moduleBitCount, moduleByteCount;
    static volatile uint8_t currentCmd, statusCmd;
    static volatile uint8_t isrPanelData[dscReadSize], isrPanelBitTotal, isrPanelBitCount, isrPanelByteCount;
    static volatile uint8_t isrModuleData[dscReadSize], isrModuleBitTotal, isrModuleBitCount, isrModuleByteCount;
};

#endif  // dscKeybusInterface_h
