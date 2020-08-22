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

/*
 *  Print messages
 */


void dscKeybusInterface::printPanelMessage() {
  switch (panelData[0]) {
    case 0x05: printPanel_0x05(); return;  // Status: partitions 1-4
    case 0x0A: printPanel_0x0A(); return;  // Status in alarm/programming, partitions 1-4
    case 0x11: printPanel_0x11(); return;  // Keypad slot query
    case 0x16: printPanel_0x16(); return;  // Zone wiring
    case 0x1B: printPanel_0x1B(); return;  // Status: partitions 5-8
    case 0x1C: printPanel_0x1C(); return;  // Verify keypad Fire/Auxiliary/Panic
    case 0x27: printPanel_0x27(); return;  // Status with zones 1-8
    case 0x28: printPanel_0x28(); return;  // Zone expander query
    case 0x2D: printPanel_0x2D(); return;  // Status with zones 9-16
    case 0x34: printPanel_0x34(); return;  // Status with zones 17-24
    case 0x3E: printPanel_0x3E(); return;  // Status with zones 25-32
    case 0x4C: printPanel_0x4C(); return;  // Unknown Keybus query
    case 0x58: printPanel_0x58(); return;  // Unknown Keybus query
    case 0x5D: printPanel_0x5D(); return;  // Flash panel lights: status and zones 1-32, partition 1
    case 0x63: printPanel_0x63(); return;  // Flash panel lights: status and zones 1-32, partition 2
    case 0x64: printPanel_0x64(); return;  // Beep - one-time, partition 1
    case 0x69: printPanel_0x69(); return;  // Beep - one-time, partition 2
    case 0x75: printPanel_0x75(); return;  // Beep pattern - repeated, partition 1
    case 0x7A: printPanel_0x7A(); return;  // Beep pattern - repeated, partition 2
    case 0x7F: printPanel_0x7F(); return;  // Beep - one-time long beep, partition 1
    case 0x82: printPanel_0x82(); return;  // Beep - one-time long beep, partition 1
    case 0x87: printPanel_0x87(); return;  // Panel outputs
    case 0x8D: printPanel_0x8D(); return;  // User code programming key response, codes 17-32
    case 0x94: printPanel_0x94(); return;  // Unknown - immediate after entering *5 programming
    case 0xA5: printPanel_0xA5(); return;  // Date, time, system status messages - partitions 1-2
    case 0xB1: printPanel_0xB1(); return;  // Enabled zones 1-32
    case 0xBB: printPanel_0xBB(); return;  // Bell
    case 0xC3: printPanel_0xC3(); return;  // Keypad status
    case 0xCE: printPanel_0xCE(); return;  // Unknown command
    case 0xD5: printPanel_0xD5(); return;  // Keypad zone query
    case 0xE6: printPanel_0xE6(); return;  // Extended status commands: partitions 3-8, zones 33-64
    case 0xEB: printPanel_0xEB(); return;  // Date, time, system status messages - partitions 1-8
    default: {
      std::cout << "Unrecognized data" << std::endl;
      if (!validCRC()) {
        std::cout << "[No CRC or CRC Error]" << std::endl;
        return;
      }
      else std::cout << "[CRC OK]" << std::endl;
      return;
    }
  }
}


void dscKeybusInterface::printModuleMessage() {
  switch (moduleData[0]) {
    case 0x77: printModule_0x77(); return;  // Keypad fire alarm
    case 0xBB: printModule_0xBB(); return;  // Keypad auxiliary alarm
    case 0xDD: printModule_0xDD(); return;  // Keypad panic alarm
  }

  // Keypad and module responses to panel queries
  switch (currentCmd) {
    case 0x11: printModule_Panel_0x11(); return;  // Keypad slot query response
    case 0xD5: printModule_Panel_0xD5(); return;  // Keypad zone query response
  }

  // Keypad and module status update notifications
  if (moduleData[4] != 0xFF || moduleData[5] != 0xFF) {
    printModule_Notification();
    return;
  }

  // Keypad keys
  printModule_Keys();
}


/*
 *  Print panel messages
 */


 // Keypad lights for commands 0x05, 0x0A, 0x1B, 0x27, 0x2D, 0x34, 0x3E, 0x5D
 void dscKeybusInterface::printPanelLights(uint8_t panelByte) {
  if (panelData[panelByte] == 0) std::cout << "none " << std::endl; //std::cout << "none "));
  else {
    if (bitRead(panelData[panelByte],0)) std::cout << "Ready " << std::endl;
    if (bitRead(panelData[panelByte],1)) std::cout << "Armed " << std::endl;
    if (bitRead(panelData[panelByte],2)) std::cout << "Memory " << std::endl;
    if (bitRead(panelData[panelByte],3)) std::cout << "Bypass " << std::endl;
    if (bitRead(panelData[panelByte],4)) std::cout << "Trouble " << std::endl;
    if (bitRead(panelData[panelByte],5)) std::cout << "Program " << std::endl;
    if (bitRead(panelData[panelByte],6)) std::cout << "Fire " << std::endl;
    if (bitRead(panelData[panelByte],7)) std::cout << "Backlight " << std::endl;
  }
 }


// Messages for commands 0x05, 0x0A, 0x1B, 0x27, 0x2D, 0x34, 0x3E
void dscKeybusInterface::printPanelMessages(uint8_t panelByte) {
  switch (panelData[panelByte]) {
    case 0x01: std::cout << "Partition ready" << std::endl; break;
    case 0x02: std::cout << "Stay/away zones open" << std::endl; break;
    case 0x03: std::cout << "Zones open" << std::endl; break;
    case 0x04: std::cout << "Armed stay" << std::endl; break;
    case 0x05: std::cout << "Armed away" << std::endl; break;
    case 0x07: std::cout << "Failed to arm" << std::endl; break;
    case 0x08: std::cout << "Exit delay in progress" << std::endl; break;
    case 0x09: std::cout << "Arming with no entry delay" << std::endl; break;
    case 0x0B: std::cout << "Quick exit in progress" << std::endl; break;
    case 0x0C: std::cout << "Entry delay in progress" << std::endl; break;
    case 0x0D: std::cout << "Opening after alarm" << std::endl; break;
    case 0x10: std::cout << "Keypad lockout" << std::endl; break;
    case 0x11: std::cout << "Partition in alarm" << std::endl; break;
    case 0x14: std::cout << "Auto-arm in progress" << std::endl; break;
    case 0x15: std::cout << "Arming with bypassed zones" << std::endl; break;
    case 0x16: std::cout << "Armed with no entry delay" << std::endl; break;
    case 0x22: std::cout << "Recent closing" << std::endl; break;
    case 0x33: std::cout << "Command output in progress" << std::endl; break;
    case 0x3D: std::cout << "Disarmed after alarm in memory" << std::endl; break;
    case 0x3E: std::cout << "Partition disarmed" << std::endl; break;
    case 0x40: std::cout << "Keypad blanked" << std::endl; break;
    case 0x8A: std::cout << "Activate stay/away zones" << std::endl; break;
    case 0x8B: std::cout << "Quick exit" << std::endl; break;
    case 0x8E: std::cout << "Invalid option" << std::endl; break;
    case 0x8F: std::cout << "Invalid access code" << std::endl; break;
    case 0x9E: std::cout << "Enter * function code" << std::endl; break;
    case 0x9F: std::cout << "Enter access code" << std::endl; break;
    case 0xA0: std::cout << "*1: Zone bypass programming" << std::endl; break;
    case 0xA1: std::cout << "*2: Trouble menu" << std::endl; break;
    case 0xA2: std::cout << "*3: Alarm memory display" << std::endl; break;
    case 0xA3: std::cout << "Door chime enabled" << std::endl; break;
    case 0xA4: std::cout << "Door chime disabled" << std::endl; break;
    case 0xA5: std::cout << "Enter master code" << std::endl; break;
    case 0xA6: std::cout << "*5: Access codes" << std::endl; break;
    case 0xA7: std::cout << "*5: Enter new code" << std::endl; break;
    case 0xA9: std::cout << "*6: User functions" << std::endl; break;
    case 0xAA: std::cout << "*6: Time and Date" << std::endl; break;
    case 0xAB: std::cout << "*6: Auto-arm time" << std::endl; break;
    case 0xAC: std::cout << "*6: Auto-arm enabled" << std::endl; break;
    case 0xAD: std::cout << "*6: Auto-arm disabled" << std::endl; break;
    case 0xAF: std::cout << "*6: System test" << std::endl; break;
    case 0xB0: std::cout << "*6: Enable DLS" << std::endl; break;
    case 0xB2: std::cout << "*7: Command output" << std::endl; break;
    case 0xB7: std::cout << "Enter installer code" << std::endl; break;
    case 0xB8: std::cout << "*  pressed while armed" << std::endl; break;
    case 0xB9: std::cout << "*2: Zone tamper menu" << std::endl; break;
    case 0xBA: std::cout << "*2: Zones with low batteries" << std::endl; break;
    case 0xC6: std::cout << "*2: Zone fault menu" << std::endl; break;
    case 0xC8: std::cout << "*2: Service required menu" << std::endl; break;
    case 0xD0: std::cout << "*2: Handheld keypads with low batteries" << std::endl; break;
    case 0xD1: std::cout << "*2: Wireless keys with low batteries" << std::endl; break;
    case 0xE4: std::cout << "*8: Main menu" << std::endl; break;
    case 0xE5: std::cout << "Keypad slot assignment" << std::endl; break;
    case 0xE6: std::cout << "*8: Input required: 2 digits" << std::endl; break;
    case 0xE7: std::cout << "*8: Input required: 3 digits" << std::endl; break;
    case 0xE8: std::cout << "*8: Input required: 4 digits" << std::endl; break;
    case 0xEA: std::cout << "*8: Reporting code: 2 digits" << std::endl; break;
    case 0xEB: std::cout << "*8: Telephone number account code: 4 digits" << std::endl; break;
    case 0xEC: std::cout << "*8: Input required: 6 digits" << std::endl; break;
    case 0xED: std::cout << "*8: Input required: 32 digits" << std::endl; break;
    case 0xEE: std::cout << "*8: Input required: 1 option per zone" << std::endl; break;
    case 0xF0: std::cout << "Function key 1 programming" << std::endl; break;
    case 0xF1: std::cout << "Function key 2 programming" << std::endl; break;
    case 0xF2: std::cout << "Function key 3 programming" << std::endl; break;
    case 0xF3: std::cout << "Function key 4 programming" << std::endl; break;
    case 0xF4: std::cout << "Function key 5 programming" << std::endl; break;
    case 0xF8: std::cout << "Keypad programming" << std::endl; break;
    default:
      std::cout << "Unrecognized data" << std::endl;
      std::cout << ": 0x" << std::endl;
      if (panelData[panelByte] < 10) std::cout << "0" << std::endl;
      std::cout << panelData[panelByte] << std::endl; // HEX value
      break;
  }
}


// Status messages for commands 0xA5, 0xEB
void dscKeybusInterface::printPanelStatus0(uint8_t panelByte) {
  bool decoded = true;
  switch (panelData[panelByte]) {
    /*
     *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
     *  10100101 0 00011000 01001111 10110000 11101100 01001001 11111111 11110000 [0xA5] 03/29/2018 16:59 | Duress alarm
     *  10100101 0 00011000 01001111 11001110 10111100 01001010 11111111 11011111 [0xA5] 03/30/2018 14:47 | Disarmed after alarm in memory
     *  10100101 0 00011000 01001111 11001010 01000100 01001011 11111111 01100100 [0xA5] 03/30/2018 10:17 | Partition in alarm
     *  10100101 0 00011000 01010000 01001001 10111000 01001100 11111111 01011001 [0xA5] 04/02/2018 09:46 | Zone expander supervisory alarm
     *  10100101 0 00011000 01010000 01001010 00000000 01001101 11111111 10100011 [0xA5] 04/02/2018 10:00 | Zone expander supervisory restored
     *  10100101 0 00011000 01001111 01110010 10011100 01001110 11111111 01100111 [0xA5] 03/27/2018 18:39 | Keypad Fire alarm
     *  10100101 0 00011000 01001111 01110010 10010000 01001111 11111111 01011100 [0xA5] 03/27/2018 18:36 | Keypad Aux alarm
     *  10100101 0 00011000 01001111 01110010 10001000 01010000 11111111 01010101 [0xA5] 03/27/2018 18:34 | Keypad Panic alarm
     *  10100101 0 00010001 01101101 01100000 00000100 01010001 11111111 11010111 [0xA5] 11/11/2011 00:01 | Keypad status check?   // Power-on +124s, keypad sends status update immediately after this
     *  10100101 0 00011000 01001111 01110010 10011100 01010010 11111111 01101011 [0xA5] 03/27/2018 18:39 | Keypad Fire alarm restored
     *  10100101 0 00011000 01001111 01110010 10010000 01010011 11111111 01100000 [0xA5] 03/27/2018 18:36 | Keypad Aux alarm restored
     *  10100101 0 00011000 01001111 01110010 10001000 01010100 11111111 01011001 [0xA5] 03/27/2018 18:34 | Keypad Panic alarm restored
     *  10100101 0 00011000 01001111 11110110 00110100 10011000 11111111 11001101 [0xA5] 03/31/2018 22:13 | Keypad lockout
     *  10100101 0 00011000 01001111 11101011 10100100 10111110 11111111 01011000 [0xA5] 03/31/2018 11:41 | Armed partial: Zones bypassed
     *  10100101 0 00011000 01001111 11101011 00011000 10111111 11111111 11001101 [0xA5] 03/31/2018 11:06 | Armed special: quick-arm/auto-arm/keyswitch/wireless key/DLS
     *  10100101 0 00010001 01101101 01100000 00101000 11100101 11111111 10001111 [0xA5] 11/11/2011 00:10 | Auto-arm cancelled
     *  10100101 0 00011000 01001111 11110111 01000000 11100110 11111111 00101000 [0xA5] 03/31/2018 23:16 | Disarmed special: keyswitch/wireless key/DLS
     *  10100101 0 00011000 01001111 01101111 01011100 11100111 11111111 10111101 [0xA5] 03/27/2018 15:23 | Panel battery trouble
     *  10100101 0 00011000 01001111 10110011 10011000 11101000 11111111 00111110 [0xA5] 03/29/2018 19:38 | AC power failure  // Sent after delay in *8 [370]
     *  10100101 0 00011000 01001111 01110100 01010000 11101001 11111111 10111000 [0xA5] 03/27/2018 20:20 | Bell trouble
     *  10100101 0 00011000 01001111 11000000 10001000 11101100 11111111 00111111 [0xA5] 03/30/2018 00:34 | Telephone line trouble
     *  10100101 0 00011000 01001111 01101111 01110000 11101111 11111111 11011001 [0xA5] 03/27/2018 15:28 | Panel battery restored
     *  10100101 0 00011000 01010000 00100000 01011000 11110000 11111111 01110100 [0xA5] 04/01/2018 00:22 | AC power restored  // Sent after delay in *8 [370]
     *  10100101 0 00011000 01001111 01110100 01011000 11110001 11111111 11001000 [0xA5] 03/27/2018 20:22 | Bell restored
     *  10100101 0 00011000 01001111 11000000 10001000 11110100 11111111 01000111 [0xA5] 03/30/2018 00:34 | Telephone line restored
     *  10100101 0 00011000 01001111 11100001 01011000 11111111 11111111 01000011 [0xA5] 03/31/2018 01:22 | System test
     */
    // 0x09 - 0x28: Zone alarm, zones 1-32
    // 0x29 - 0x48: Zone alarm restored, zones 1-32
    case 0x49: std::cout << "Duress alarm" << std::endl; break;
    case 0x4A: std::cout << "Disarmed after alarm in memory" << std::endl; break;
    case 0x4B: std::cout << "Partition in alarm" << std::endl; break;
    case 0x4C: std::cout << "Zone expander supervisory alarm" << std::endl; break;
    case 0x4D: std::cout << "Zone expander supervisory restored" << std::endl; break;
    case 0x4E: std::cout << "Keypad Fire alarm" << std::endl; break;
    case 0x4F: std::cout << "Keypad Aux alarm" << std::endl; break;
    case 0x50: std::cout << "Keypad Panic alarm" << std::endl; break;
    case 0x51: std::cout << "Auxiliary input alarm" << std::endl; break;
    case 0x52: std::cout << "Keypad Fire alarm restored" << std::endl; break;
    case 0x53: std::cout << "Keypad Aux alarm restored" << std::endl; break;
    case 0x54: std::cout << "Keypad Panic alarm restored" << std::endl; break;
    case 0x55: std::cout << "Auxilary input alarm restored" << std::endl; break;
    // 0x56 - 0x75: Zone tamper, zones 1-32
    // 0x76 - 0x95: Zone tamper restored, zones 1-32
    case 0x98: std::cout << "Keypad lockout" << std::endl; break;
    // 0x99 - 0xBD: Armed by access code
    case 0xBE: std::cout << "Armed partial: Zones bypassed" << std::endl; break;
    case 0xBF: std::cout << "Armed special: quick-arm/auto-arm/keyswitch/wireless key/DLS" << std::endl; break;
    // 0xC0 - 0xE4: Disarmed by access code
    case 0xE5: std::cout << "Auto-arm cancelled" << std::endl; break;
    case 0xE6: std::cout << "Disarmed special: keyswitch/wireless key/DLS" << std::endl; break;
    case 0xE7: std::cout << "Panel battery trouble" << std::endl; break;
    case 0xE8: std::cout << "Panel AC power failure" << std::endl; break;
    case 0xE9: std::cout << "Bell trouble" << std::endl; break;
    case 0xEA: std::cout << "Power on +16s" << std::endl; break;
    case 0xEC: std::cout << "Telephone line trouble" << std::endl; break;
    case 0xEF: std::cout << "Panel battery restored" << std::endl; break;
    case 0xF0: std::cout << "Panel AC power restored" << std::endl; break;
    case 0xF1: std::cout << "Bell restored" << std::endl; break;
    case 0xF4: std::cout << "Telephone line restored" << std::endl; break;
    case 0xFF: std::cout << "System test" << std::endl; break;
    default: decoded = false;
  }
  if (decoded) return;

  /*
   *  Zone alarm, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00011000 01001111 01001001 11011000 00001001 11111111 00110101 [0xA5] 03/26/2018 09:54 | Zone alarm: 1
   *  10100101 0 00011000 01001111 01001010 00100000 00001110 11111111 10000011 [0xA5] 03/26/2018 10:08 | Zone alarm: 6
   *  10100101 0 00011000 01001111 10010100 11001000 00010000 11111111 01110111 [0xA5] 03/28/2018 20:50 | Zone alarm: 8
   */
  if (panelData[panelByte] >= 0x09 && panelData[panelByte] <= 0x28) {
    std::cout << "Zone alarm: " << std::endl;
    std::cout << panelData[panelByte] - 0x08 << std::endl;
    return;
  }

  /*
   *  Zone alarm restored, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00011000 01001111 10010100 11001100 00101001 11111111 10010100 [0xA5] 03/28/2018 20:51 | Zone alarm restored: 1
   *  10100101 0 00011000 01001111 10010100 11010100 00101110 11111111 10100001 [0xA5] 03/28/2018 20:53 | Zone alarm restored: 6
   *  10100101 0 00011000 01001111 10010100 11010000 00110000 11111111 10011111 [0xA5] 03/28/2018 20:52 | Zone alarm restored: 8
   */
  if (panelData[panelByte] >= 0x29 && panelData[panelByte] <= 0x48) {
    std::cout << "Zone alarm restored: " << std::endl;
    std::cout << panelData[panelByte] - 0x28 << std::endl;
    return;
  }

  /*
   *  Zone tamper, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00000001 01000100 00100010 01011100 01010110 11111111 10111101 [0xA5] 01/01/2001 02:23 | Zone tamper: 1
   *  10100101 0 00000001 01000100 00100010 01011100 01010111 11111111 10111101 [0xA5] 01/01/2001 02:23 | Zone tamper: 2
   *  10100101 0 00010001 01101101 01101011 10010000 01011011 11111111 01111000 [0xA5] 11/11/2011 11:36 | Zone tamper: 6
   */
  if (panelData[panelByte] >= 0x56 && panelData[panelByte] <= 0x75) {
    std::cout << "Zone tamper: " << std::endl;
    std::cout << panelData[6] - 0x55 << std::endl;
    return;
  }

  /*
   *  Zone tamper restored, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00000001 01000100 00100010 01011100 01110110 11111111 11011101 [0xA5] 01/01/2001 02:23 | Zone tamper restored: 1
   *  10100101 0 00000001 01000100 00100010 01011100 01111000 11111111 11011101 [0xA5] 01/01/2001 02:23 | Zone tamper restored: 2
   *  10100101 0 00010001 01101101 01101011 10010000 01111011 11111111 10011000 [0xA5] 11/11/2011 11:36 | Zone tamper restored: 6
   */
  if (panelData[panelByte] >= 0x76 && panelData[panelByte] <= 0x95) {
    std::cout << "Zone tamper restored: " << std::endl;
    std::cout << panelData[panelByte] - 0x75 << std::endl;
    return;
  }

  /*
   *  Armed by access code
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00011000 01001101 00001000 10010000 10011001 11111111 00111010 [0xA5] 03/08/2018 08:36 | Armed by user code 1
   *  10100101 0 00011000 01001101 00001000 10111100 10111011 11111111 10001000 [0xA5] 03/08/2018 08:47 | Armed by master code 40
   */
  if (panelData[panelByte] >= 0x99 && panelData[panelByte] <= 0xBD) {
    uint8_t dscCode = panelData[panelByte] - 0x98;
    if (dscCode >= 35) dscCode += 5;
    std::cout << "Armed by " << std::endl;
    switch (dscCode) {
      case 33: std::cout << "duress " << std::endl; break;
      case 34: std::cout << "duress " << std::endl; break;
      case 40: std::cout << "master " << std::endl; break;
      case 41: std::cout << "supervisor " << std::endl; break;
      case 42: std::cout << "supervisor " << std::endl; break;
      default: std::cout << "user " << std::endl; break;
    }
    std::cout << "code " << std::endl;
    std::cout << dscCode << std::endl;
    return;
  }

  /*
   *  Disarmed by access code
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00011000 01001101 00001000 11101100 11000000 11111111 10111101 [0xA5] 03/08/2018 08:59 | Disarmed by user code 1
   *  10100101 0 00011000 01001101 00001000 10110100 11100010 11111111 10100111 [0xA5] 03/08/2018 08:45 | Disarmed by master code 40
   */
  if (panelData[panelByte] >= 0xC0 && panelData[panelByte] <= 0xE4) {
    uint8_t dscCode = panelData[panelByte] - 0xBF;
    if (dscCode >= 35) dscCode += 5;
    std::cout << "Disarmed by " << std::endl;
    switch (dscCode) {
      case 33: std::cout << "duress " << std::endl; break;
      case 34: std::cout << "duress " << std::endl; break;
      case 40: std::cout << "master " << std::endl; break;
      case 41: std::cout << "supervisor " << std::endl; break;
      case 42: std::cout << "supervisor " << std::endl; break;
      default: std::cout << "user " << std::endl; break;
    }
    std::cout << "code " << std::endl;
    std::cout << dscCode << std::endl;
    return;
  }

  std::cout << "Unrecognized data" << std::endl;
}


// Status messages for commands 0xA5, 0xEB
void dscKeybusInterface::printPanelStatus1(uint8_t panelByte) {
  switch (panelData[panelByte]) {
    /*
     *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
     *  10100101 0 00011000 01001111 11001010 10001001 00000011 11111111 01100001 [0xA5] 03/30/2018 10:34 | Cross zone alarm
     *  10100101 0 00010001 01101101 01101010 00000001 00000100 11111111 10010001 [0xA5] 11/11/2011 10:00 | Delinquency alarm
     *  10100101 0 00010001 01101101 01100000 10101001 00100100 00000000 01010000 [0xA5] 11/11/2011 00:42 | Auto-arm cancelled by duress code 33
     *  10100101 0 00010001 01101101 01100000 10110101 00100101 00000000 01011101 [0xA5] 11/11/2011 00:45 | Auto-arm cancelled by duress code 34
     *  10100101 0 00010001 01101101 01100000 00101001 00100110 00000000 11010010 [0xA5] 11/11/2011 00:10 | Auto-arm cancelled by master code 40
     *  10100101 0 00010001 01101101 01100000 10010001 00100111 00000000 00111011 [0xA5] 11/11/2011 00:36 | Auto-arm cancelled by supervisor code 41
     *  10100101 0 00010001 01101101 01100000 10111001 00101000 00000000 01100100 [0xA5] 11/11/2011 00:46 | Auto-arm cancelled by supervisor code 42
     *  10100101 0 00011000 01001111 10100000 10011101 00101011 00000000 01110100 [0xA5] 03/29/2018 00:39 | Armed by auto-arm
     *  10100101 0 00011000 01001101 00001010 00001101 10101100 00000000 11001101 [0xA5] 03/08/2018 10:03 | Exit *8 programming
     *  10100101 0 00011000 01001101 00001001 11100001 10101101 00000000 10100001 [0xA5] 03/08/2018 09:56 | Enter *8
     *  10100101 0 00010001 01101101 01100010 11001101 11010000 00000000 00100010 [0xA5] 11/11/2011 02:51 | Command output 4
     */
    case 0x03: std::cout << "Cross zone alarm" << std::endl; return;
    case 0x04: std::cout << "Delinquency alarm" << std::endl; return;
    case 0x24: std::cout << "Auto-arm cancelled by duress code 33" << std::endl; return;
    case 0x25: std::cout << "Auto-arm cancelled by duress code 34" << std::endl; return;
    case 0x26: std::cout << "Auto-arm cancelled by master code 40" << std::endl; return;
    case 0x27: std::cout << "Auto-arm cancelled by supervisor code 41" << std::endl; return;
    case 0x28: std::cout << "Auto-arm cancelled by supervisor code 42" << std::endl; return;
    case 0x2B: std::cout << "Armed by auto-arm" << std::endl; return;
    // 0x6C - 0x8B: Zone fault restored, zones 1-32
    // 0x8C - 0xAB: Zone fault, zones 1-32
    case 0xAC: std::cout << "Exit *8 programming" << std::endl; return;
    case 0xAD: std::cout << "Enter *8 programming" << std::endl; return;
    // 0xB0 - 0xCF: Zones bypassed, zones 1-32
    case 0xD0: std::cout << "Command output 4" << std::endl; return;
  }

  /*
   *  Zone fault restored, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00010001 01101101 01101011 01000001 01101100 11111111 00111010 [0xA5] 11/11/2011 11:16 | Zone fault restored: 1
   *  10100101 0 00010001 01101101 01101011 01010101 01101101 11111111 01001111 [0xA5] 11/11/2011 11:21 | Zone fault restored: 2
   *  10100101 0 00010001 01101101 01101011 10000101 01101111 11111111 10000001 [0xA5] 11/11/2011 11:33 | Zone fault restored: 4
   *  10100101 0 00010001 01101101 01101011 10001001 01110000 11111111 10000110 [0xA5] 11/11/2011 11:34 | Zone fault restored: 5
   */
  if (panelData[panelByte] >= 0x6C && panelData[panelByte] <= 0x8B) {
    std::cout << "Zone fault restored: " << std::endl;
    std::cout << panelData[panelByte] - 0x6B << std::endl;
    return;
  }

  /*
   *  Zone fault, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00010001 01101101 01101011 00111101 10001100 11111111 01010110 [0xA5] 11/11/2011 11:15 | Zone fault: 1
   *  10100101 0 00010001 01101101 01101011 01010101 10001101 11111111 01101111 [0xA5] 11/11/2011 11:21 | Zone fault: 2
   *  10100101 0 00010001 01101101 01101011 10000001 10001111 11111111 10011101 [0xA5] 11/11/2011 11:32 | Zone fault: 3
   *  10100101 0 00010001 01101101 01101011 10001001 10010000 11111111 10100110 [0xA5] 11/11/2011 11:34 | Zone fault: 4
   */
  if (panelData[panelByte] >= 0x8C && panelData[panelByte] <= 0xAB) {
    std::cout << "Zone fault: " << std::endl;
    std::cout << panelData[panelByte] - 0x8B << std::endl;
    return;
  }

  /*
   *  Zones bypassed, zones 1-32
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00011000 01001111 10110001 10101001 10110001 00000000 00010111 [0xA5] 03/29/2018 17:42 | Zone bypassed: 2
   *  10100101 0 00011000 01001111 10110001 11000001 10110101 00000000 00110011 [0xA5] 03/29/2018 17:48 | Zone bypassed: 6
   */
  if (panelData[panelByte] >= 0xB0 && panelData[panelByte] <= 0xCF) {
    std::cout << "Zone bypassed: " << std::endl;
    std::cout << panelData[panelByte] - 0xAF << std::endl;
    return;
  }

  std::cout << "Unrecognized data" << std::endl;
}


// Status messages for commands 0xA5, 0xEB
void dscKeybusInterface::printPanelStatus2(uint8_t panelByte) {
  switch (panelData[panelByte]) {

    /*
     *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
     *  10100101 0 00011000 01001111 10101111 10000110 00101010 00000000 01101011 [0xA5] 03/29/2018 15:33 | Quick exit
     *  10100101 0 00010001 01101101 01110101 00111010 01100011 00000000 00110101 [0xA5] 11/11/2011 21:14 | Keybus fault restored
     *  10100101 0 00011000 01001111 11110111 01110110 01100110 00000000 11011111 [0xA5] 03/31/2018 23:29 | Enter *1 zone bypass programming
     *  10100101 0 00010001 01101101 01100010 11001110 01101001 00000000 10111100 [0xA5] 11/11/2011 02:51 | Command output 3
     *  10100101 0 00011000 01010000 01000000 00000010 10001100 00000000 11011011 [0xA5] 04/02/2018 00:00 | Loss of system time
     *  10100101 0 00011000 01001111 10101110 00001110 10001101 00000000 01010101 [0xA5] 03/29/2018 14:03 | Power on
     *  10100101 0 00011000 01010000 01000000 00000010 10001110 00000000 11011101 [0xA5] 04/02/2018 00:00 | Panel factory default
     *  10100101 0 00011000 01001111 11101010 10111010 10010011 00000000 01000011 [0xA5] 03/31/2018 10:46 | Disarmed by keyswitch
     *  10100101 0 00011000 01001111 11101010 10101110 10010110 00000000 00111010 [0xA5] 03/31/2018 10:43 | Armed by keyswitch
     *  10100101 0 00011000 01001111 10100000 01100010 10011000 00000000 10100110 [0xA5] 03/29/2018 00:24 | Armed by quick-arm
     *  10100101 0 00010001 01101101 01100000 00101110 10011001 00000000 01001010 [0xA5] 11/11/2011 00:11 | Activate stay/away zones
     *  10100101 0 00011000 01001111 00101101 00011010 10011010 00000000 11101101 [0xA5] 03/25/2018 13:06 | Armed: stay
     *  10100101 0 00011000 01001111 00101101 00010010 10011011 00000000 11100110 [0xA5] 03/25/2018 13:04 | Armed: away
     *  10100101 0 00011000 01001111 00101101 10011010 10011100 00000000 01101111 [0xA5] 03/25/2018 13:38 | Armed without entry delay
     *  10100101 0 00011000 01001111 00101100 11011110 11000011 00000000 11011001 [0xA5] 03/25/2018 12:55 | Enter *5 programming
     *  10100101 0 00011000 01001111 00101110 00000010 11100110 00000000 00100010 [0xA5] 03/25/2018 14:00 | Enter *6 programming
     */
    case 0x2A: std::cout << "Quick exit" << std::endl; return;
    case 0x63: std::cout << "Keybus fault restored" << std::endl; return;
    case 0x66: std::cout << "Enter *1 zone bypass programming" << std::endl; return;
    case 0x67: std::cout << "Command output 1" << std::endl; return;
    case 0x68: std::cout << "Command output 2" << std::endl; return;
    case 0x69: std::cout << "Command output 3" << std::endl; return;
    case 0x8C: std::cout << "Loss of system time" << std::endl; return;
    case 0x8D: std::cout << "Power on" << std::endl; return;
    case 0x8E: std::cout << "Panel factory default" << std::endl; return;
    case 0x93: std::cout << "Disarmed by keyswitch" << std::endl; return;
    case 0x96: std::cout << "Armed by keyswitch" << std::endl; return;
    case 0x97: std::cout << "Armed by keypad away" << std::endl; return;
    case 0x98: std::cout << "Armed by quick-arm" << std::endl; return;
    case 0x99: std::cout << "Activate stay/away zones" << std::endl; return;
    case 0x9A: std::cout << "Armed: stay" << std::endl; return;
    case 0x9B: std::cout << "Armed: away" << std::endl; return;
    case 0x9C: std::cout << "Armed without entry delay" << std::endl; return;
    case 0xC3: std::cout << "Enter *5 programming" << std::endl; return;
    // 0xC6 - 0xE5: Auto-arm cancelled by user code
    case 0xE6: std::cout << "Enter *6 programming" << std::endl; return;
    // 0xE9 - 0xF0: Supervisory restored, keypad slots 1-8
    // 0xF1 - 0xF8: Supervisory trouble, keypad slots 1-8
  }

  /*
   *  Auto-arm cancelled by user code
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00010001 01101101 01100000 00111110 11000110 00000000 10000111 [0xA5] 11/11/2011 00:15 | Auto-arm cancelled by user code 1
   *  10100101 0 00010001 01101101 01100000 01111010 11100101 00000000 11100010 [0xA5] 11/11/2011 00:30 | Auto-arm cancelled by user code 32
   */
  if (panelData[panelByte] >= 0xC6 && panelData[panelByte] <= 0xE5) {
    std::cout << "Auto-arm cancelled by user code " << std::endl;
    std::cout << panelData[panelByte] - 0xC5 << std::endl;
    return;
  }

  /*
   *  Supervisory restored, keypad slots 1-8
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00010001 01101101 01110100 10001110 11101001 11111111 00001101 [0xA5] 11/11/2011 20:35 | Supervisory - module detected: Keypad slot 1
   *  10100101 0 00010001 01101101 01110100 00110010 11110000 11111111 10111000 [0xA5] 11/11/2011 20:12 | Supervisory - module detected: Keypad slot 8
   */
  if (panelData[panelByte] >= 0xE9 && panelData[panelByte] <= 0xF0) {
    std::cout << "Supervisory - module detected: Keypad slot " << std::endl;
    std::cout << panelData[panelByte] - 0xE8 << std::endl;
    return;
  }

  /*
   *  Supervisory trouble, keypad slots 1-8
   *
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00010001 01101101 01110100 10000110 11110001 11111111 00001101 [0xA5] 11/11/2011 20:33 | Supervisory - module trouble: Keypad slot 1
   *  10100101 0 00010001 01101101 01110100 00101110 11111000 11111111 10111100 [0xA5] 11/11/2011 20:11 | Supervisory - module trouble: Keypad slot 8
   */
  if (panelData[panelByte] >= 0xF1 && panelData[panelByte] <= 0xF8) {
    std::cout << "Supervisory - module trouble: Keypad slot " << std::endl;
    std::cout << panelData[panelByte] - 0xF0 << std::endl;
    return;
  }

  std::cout << "Unrecognized data" << std::endl;
}


// Status messages for commands 0xA5, 0xEB
void dscKeybusInterface::printPanelStatus3(uint8_t panelByte) {
  std::cout << "Unrecognized data" << std::endl;
  std::cout << " :" << std::endl;
  std::cout << panelByte << std::endl; // HEX Value
}


// Status messages for command 0xEB
void dscKeybusInterface::printPanelStatus4(uint8_t panelByte) {
  if (panelData[panelByte] <= 0x1F) {
    std::cout << "Zone alarm: " << std::endl;
    std::cout << panelData[panelByte] + 33 << std::endl;
    return;
  }

  if (panelData[panelByte] >= 0x20 && panelData[panelByte] <= 0x3F) {
    std::cout << "Zone alarm restored: " << std::endl;
    std::cout << panelData[panelByte] + 1 << std::endl;
    return;
  }

  if (panelData[panelByte] >= 0x40 && panelData[panelByte] <= 0x5F) {
    std::cout << "Zone tamper: " << std::endl;
    std::cout << panelData[panelByte] - 31 << std::endl;
    return;
  }

  if (panelData[panelByte] >= 0x60 && panelData[panelByte] <= 0x7F) {
    std::cout << "Zone tamper restored: " << std::endl;
    std::cout << panelData[panelByte] - 63 << std::endl;
    return;
  }


  std::cout << "Unrecognized data" << std::endl;
}


// Status messages for command 0xEB
void dscKeybusInterface::printPanelStatus14(uint8_t panelByte) {
  if (panelData[panelByte] >= 0x40 && panelData[panelByte] <= 0x5F) {
    std::cout << "Zone fault restored: " << std::endl;
    std::cout << panelData[panelByte] - 31 << std::endl;
    return;
  }

  if (panelData[panelByte] >= 0x60 && panelData[panelByte] <= 0x7F) {
    std::cout << "Zone fault: " << std::endl;
    std::cout << panelData[panelByte] - 63 << std::endl;
    return;
  }


  std::cout << "Unrecognized data" << std::endl;
}


// Prints individual bits as a number for partitions and zones
void dscKeybusInterface::printPanelBitNumbers(uint8_t panelByte, uint8_t startNumber) {
  for (uint8_t bit = 0; bit < 8; bit++) {
    if (bitRead(panelData[panelByte],bit)) {
      std::cout << startNumber + bit << std::endl;
      std::cout << " " << std::endl;
    }
  }
}


/*
 *  0x05: Status - partitions 1-4
 *  Interval: constant
 *  CRC: no
 *  Byte 2: Partition 1 lights
 *  Byte 3: Partition 1 status
 *  Byte 4: Partition 2 lights
 *  Byte 5: Partition 2 status
 *
 *  PC5020/PC1616/PC1832/PC1864:
 *  Byte 6: Partition 3 lights
 *  Byte 7: Partition 3 status
 *  Byte 8: Partition 4 lights
 *  Byte 9: Partition 4 status
 *
 *  // PC1555MX, PC5015
 *  00000101 0 10000001 00000001 10010001 11000111 [0x05] Partition 1 | Lights: Ready Backlight | Partition ready | Partition 2: disabled
 *  00000101 0 10010000 00000011 10010001 11000111 [0x05] Status lights: Trouble Backlight | Partition not ready
 *  00000101 0 10001010 00000100 10010001 11000111 [0x05] Status lights: Armed Bypass Backlight | Armed stay
 *  00000101 0 10000010 00000101 10010001 11000111 [0x05] Status lights: Armed Backlight | Armed away
 *  00000101 0 10001011 00001000 10010001 11000111 [0x05] Status lights: Ready Armed Bypass Backlight | Exit delay in progress
 *  00000101 0 10000010 00001100 10010001 11000111 [0x05] Status lights: Armed Backlight | Entry delay in progress
 *  00000101 0 10000001 00010000 10010001 11000111 [0x05] Status lights: Ready Backlight | Keypad lockout
 *  00000101 0 10000010 00010001 10010001 11000111 [0x05] Status lights: Armed Backlight | Partition in alarm
 *  00000101 0 10000001 00110011 10010001 11000111 [0x05] Status lights: Ready Backlight | Partition busy
 *  00000101 0 10000001 00111110 10010001 11000111 [0x05] Status lights: Ready Backlight | Partition disarmed
 *  00000101 0 10000001 01000000 10010001 11000111 [0x05] Status lights: Ready Backlight | Keypad blanked
 *  00000101 0 10000001 10001111 10010001 11000111 [0x05] Status lights: Ready Backlight | Invalid access code
 *  00000101 0 10000000 10011110 10010001 11000111 [0x05] Status lights: Backlight | Quick armed pressed
 *  00000101 0 10000001 10100011 10010001 11000111 [0x05] Status lights: Ready Backlight | Door chime enabled
 *  00000101 0 10000001 10100100 10010001 11000111 [0x05] Status lights: Ready Backlight | Door chime disabled

 *  00000101 0 10000010 00001101 10010001 11000111 [0x05] Status lights: Armed Backlight   *  Delay zone tripped after previous alarm tripped
 *  00000101 0 10000000 00111101 10010001 11000111 [0x05] Status lights: Backlight   *  Disarmed after previous alarm tripped
 *  00000101 0 10000000 00100010 10010001 11000111 [0x05] Status lights: Backlight   *  Disarmed after previous alarm tripped +4s
 *  00000101 0 10000010 10100110 10010001 11000111 [0x05] Status lights: Armed Backlight   *  In *5 programming
 *
 *  // PC5020, PC1616, PC1832, PC1864
 *  00000101 0 10000000 00000011 10000010 00000101 10000010 00000101 00000000 11000111 [0x05] Status lights: Backlight | Zones open
 */
void dscKeybusInterface::printPanel_0x05() {
  std::cout << "Partition 1: " << std::endl;
  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);

  if (panelData[5] == 0xC7) {
    std::cout << " | Partition 2: disabled" << std::endl;
  }
  else {
    std::cout << " | Partition 2: " << std::endl;
    printPanelLights(4);
    std::cout << "- " << std::endl;
    printPanelMessages(5);
  }

  if (panelByteCount > 9) {
    if (panelData[7] == 0xC7) {
      std::cout << " | Partition 3: disabled" << std::endl;
    }
    else {
      std::cout << " | Partition 3: " << std::endl;
      printPanelLights(6);
      std::cout << "- " << std::endl;
      printPanelMessages(7);
    }

    if (panelData[9] == 0xC7) {
      std::cout << " | Partition 4: disabled" << std::endl;
    }
    else {
      std::cout << " | Partition 4: " << std::endl;
      printPanelLights(8);
      std::cout << "- " << std::endl;
      printPanelMessages(9);
    }
  }
}


/*
 *  0x0A: Status in alarm, programming
 *  Interval: constant in *8 programming
 *  CRC: yes
 *  Byte 2: Partition 1 lights
 *  Byte 3: Partition 1 status
 *  Byte 4-7: Zone lights
 *  Byte 8: Zone lights for *5 access codes 33,34,41,42
 *
 *  00001010 0 10000010 11100100 00000000 00000000 00000000 00000000 00000000 01110000 [0x0A] Status lights: Armed | Zone lights: none
 *  00001010 0 10000001 11101110 01100101 00000000 00000000 00000000 00000000 11011110 [0x0A] Status lights: Ready | Zone lights: 1 3 6 7
 */
void dscKeybusInterface::printPanel_0x0A() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);

  bool zoneLights = false;
  std::cout << " | Zone lights: " << std::endl;
  for (uint8_t panelByte = 4; panelByte < 8; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte-4) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }

  if (panelData[8] != 0 && panelData[8] != 128) {
    zoneLights = true;
    if (bitRead(panelData[8],0)) std::cout << "33 " << std::endl;
    if (bitRead(panelData[8],1)) std::cout << "34 " << std::endl;
    if (bitRead(panelData[8],3)) std::cout << "41 " << std::endl;
    if (bitRead(panelData[8],4)) std::cout << "42 " << std::endl;
  }

  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0x11: Keypad slot query
 *  Interval: 30s
 *  CRC: no
 *
 *  00010001 0 10101010 10101010 10101010 10101010 10101010 [0x11] Keypad slot query
 *  11111111 1 00111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1
 *  11111111 1 11111111 11111100 11111111 11111111 11111111 [Keypad] Slot 8
 */
void dscKeybusInterface::printPanel_0x11() {
  std::cout << "Keypad slot query" << std::endl;
}


/*
 *  0x16: Zone wiring
 *  Interval: 4min
 *  CRC: yes
 *  Byte 2: TBD, identical with PC1555MX, PC5015, PC1832
 *  Byte 3: TBD, different between PC1555MX, PC5015, PC1832
 *  Byte 4 bits 2-7: TBD, identical with PC1555MX and PC5015
 *
 *  00010110 0 00001110 00100011 11010001 00011001 [0x16] PC1555MX | Zone wiring: NC | Exit *8 programming
 *  00010110 0 00001110 00100011 11010010 00011001 [0x16] PC1555MX | Zone wiring: EOL | Exit *8 programming
 *  00010110 0 00001110 00100011 11010011 00011001 [0x16] PC1555MX | Zone wiring: DEOL | Exit *8 programming
 *  00010110 0 00001110 00100011 11100001 00101000 [0x16] PC1555MX | Zone wiring: NC | In *8
 *  00010110 0 00001110 00100011 11100110 00101101 [0x16] PC1555MX | Zone wiring: EOL | Enter *8 programming
 *  00010110 0 00001110 00100011 11110010 00111001 [0x16] PC1555MX | Zone wiring: EOL | Armed, Exit *8 +15s, Power-on +2m
 *  00010110 0 00001110 00100011 11110111 00111101 [0x16] PC1555MX | Zone wiring: DEOL | Interval 4m
 *  00010110 0 00001110 00010000 11110011 00100111 [0x16] PC5015 | Zone wiring: DEOL | Armed, Exit *8 +15s, Power-on +2m
 *  00010110 0 00001110 01000010 10110101 00011011 [0x16] PC1832/1864 | Zone wiring: NC | Interval 4m
 *  00010110 0 00001110 01000010 10110001 00010111 [0x16] PC1832/1864 | Zone wiring: NC | Armed
 */
void dscKeybusInterface::printPanel_0x16() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  if (panelData[2] == 0x0E) {

    switch (panelData[3]) {
      case 0x10: std::cout << "PC5015 " << std::endl; break;
      case 0x23: std::cout << "PC1555MX " << std::endl; break;
      case 0x42: std::cout << "PC1832/1864 " << std::endl; break;
      default: std::cout << "Unknown panel " << std::endl; break;
    }

    switch (panelData[4] & 0x03) {
      case 0x01: std::cout << "| Zone wiring: NC " << std::endl; break;
      case 0x02: std::cout << "| Zone wiring: EOL " << std::endl; break;
      case 0x03: std::cout << "| Zone wiring: DEOL " << std::endl; break;
    }

    switch (panelData[4] >> 2) {
      case 0x2C: std::cout << "| Armed" << std::endl; break;
      case 0x2D: std::cout << "| Interval 4m" << std::endl; break;
      case 0x34: std::cout << "| Exit *8 programming" << std::endl; break;
      case 0x39: std::cout << "| *8 programming" << std::endl; break;
      case 0x3C: std::cout << "| Armed, Exit *8 +15s, Power-on +2m" << std::endl; break;
      case 0x3D: std::cout << "| Interval 4m" << std::endl; break;
      default: std::cout << "| Unrecognized data" << std::endl; break;
    }
  }
  else std::cout << "Unrecognized data" << std::endl;
}


/*
 *  0x1B: Status - partitions 5-8
 *  Interval: constant
 *  CRC: no
 *  Byte 2: Partition 5 lights
 *  Byte 3: Partition 5 status
 *  Byte 4: Partition 6 lights
 *  Byte 5: Partition 6 status
 *  Byte 6: Partition 7 lights
 *  Byte 7: Partition 7 status
 *  Byte 8: Partition 8 lights
 *  Byte 9: Partition 8 status
 *
 *  00011011 0 10010001 00000001 00010000 11000111 00010000 11000111 00010000 11000111 [0x1B]
 */
void dscKeybusInterface::printPanel_0x1B() {

  if (panelData[3] == 0xC7) {
    std::cout << "Partition 5: disabled" << std::endl;
  }
  else {
    std::cout << "Partition 5: " << std::endl;
    printPanelLights(2);
    std::cout << "- " << std::endl;
    printPanelMessages(3);
  }

  if (panelData[5] == 0xC7) {
    std::cout << " | Partition 6: disabled" << std::endl;
  }
  else {
    std::cout << " | Partition 6: " << std::endl;
    printPanelLights(4);
    std::cout << "- " << std::endl;
    printPanelMessages(5);
  }

  if (panelData[7] == 0xC7) {
    std::cout << " | Partition 7: disabled" << std::endl;
  }
  else {
    std::cout << " | Partition 7: " << std::endl;
    printPanelLights(6);
    std::cout << "- " << std::endl;
    printPanelMessages(7);
  }

  if (panelData[9] == 0xC7) {
    std::cout << " | Partition 8: disabled" << std::endl;
  }
  else {
    std::cout << " | Partition 8: " << std::endl;
    printPanelLights(8);
    std::cout << "- " << std::endl;
    printPanelMessages(9);
  }
}


/*
 *  0x1C: Verify keypad Fire/Auxiliary/Panic
 *  Interval: immediate after keypad button press
 *  CRC: no
 *
 *  01110111 1 11111111 11111111 11111111 11111111 [Keypad] Fire alarm
 *  00011100 0  [0x1C] Verify keypad Fire/Auxiliary/Panic
 *  01110111 1  [Keypad] Fire alarm
 */
void dscKeybusInterface::printPanel_0x1C() {
  std::cout << "Verify keypad Fire/Auxiliary/Panic" << std::endl;
}


/*
 *  0x27: Status with zones 1-8
 *  Interval: 4m
 *  CRC: yes
 *  Byte 2: Partition 1 lights
 *  Byte 3: Partition 1 status
 *  Byte 4: Partition 2 lights
 *  Byte 5: Partition 2 status
 *  Byte 6: Zones 1-8
 *
 *  00100111 0 10000001 00000001 10010001 11000111 00000000 00000001 [0x27] Status lights: Ready Backlight | Zones lights: none   // Unarmed, zones closed
 *  00100111 0 10000001 00000001 10010001 11000111 00000010 00000011 [0x27] Status lights: Ready Backlight | Zones lights: 2   // Unarmed, zone 2 open
 *  00100111 0 10001010 00000100 10010001 11000111 00000000 00001101 [0x27] Status lights: Armed Bypass Backlight | Zones lights: none  // Armed stay  // Periodic while armed
 *  00100111 0 10001010 00000100 11111111 11111111 00000000 10110011 [0x27] Status lights: Armed Bypass Backlight | Zones lights: none  // Armed stay +1s
 *  00100111 0 10000010 00000101 10010001 11000111 00000000 00000110 [0x27] Status lights: Armed Backlight | Zones lights: none  // Armed away  // Periodic while armed
 *  00100111 0 10000010 00000101 11111111 11111111 00000000 10101100 [0x27] Status lights: Armed Backlight | Zones lights: none  // Armed away +1s
 *  00100111 0 10000010 00001100 10010001 11000111 00000001 00001110 [0x27] Status lights: Armed Backlight | Zones lights: 1  // Delay zone 1 tripped, entrance delay
 *  00100111 0 10000010 00010001 10010001 11000111 00000001 00010011 [0x27] Status lights: Armed Backlight | Zones lights: 1  // Periodic after delay zone 1 tripped, alarm on   *  Periodic after fire alarm, alarm on
 *  00100111 0 10000010 00001101 10010001 11000111 00000001 00001111 [0x27] Status lights: Armed Backlight | Zones lights: 1  // Immediate after delay zone 1 tripped after previous alarm tripped
 *  00100111 0 10000010 00010001 11011011 11111111 00000010 10010110 [0x27] Status lights: Armed Backlight | Zones lights: 2  // Instant zone 2 tripped away
 *  00100111 0 00000001 00000001 11111111 11111111 00000000 00100111 [0x27] Status lights: Ready | Zones open 1-8: none  // Immediate after power on after panel reset
 *  00100111 0 10010001 00000001 11111111 11111111 00000000 10110111 [0x27] Status lights: Ready Trouble Backlight | Zones open 1-8: none  // +15s after exit *8
 *  00100111 0 10010001 00000001 10100000 00000000 00000000 01011001 [0x27] Status lights: Ready Trouble Backlight | Zones open 1-8: none  // +33s after power on after panel reset
 *  00100111 0 10010000 00000011 11111111 11111111 00111111 11110111 [0x27] Status lights: Trouble Backlight | Zones open: 1 2 3 4 5 6  // +122s after power on after panel reset
 *  00100111 0 10010000 00000011 10010001 11000111 00111111 01010001 [0x27] Status lights: Trouble Backlight | Zones open: 1 2 3 4 5 6  // +181s after power on after panel reset
 *  00100111 0 10000000 00000011 10000010 00000101 00011101 01001110 [0x27] Status lights: Backlight | Zones open | Zones 1-8 open: 1 3 4 5  // PC1832
 */
void dscKeybusInterface::printPanel_0x27() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1: " << std::endl;
  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);

  if (panelData[5] == 0xC7) {
    std::cout << " | Partition 2: disabled" << std::endl;
  }
  else if (panelData[5] != 0xFF) {
    std::cout << " | Partition 2: " << std::endl;
    printPanelLights(4);
    std::cout << "- " << std::endl;
    printPanelMessages(5);
  }

  std::cout << " | Zones 1-8 open: " << std::endl;
  if (panelData[6] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(6,1);
  }
}

/*
 *  0x28: Zone expander query
 *  Interval: after zone expander status notification
 *  CRC: no
 *
 *  11111111 1 11111111 11111111 10111111 11111111 [Zone Expander] Status notification
 *  00101000 0 11111111 11111111 11111111 11111111 11111111 [0x28] Zone expander query
 *  11111111 1 01010111 01010101 11111111 11111111 01101111 [Zone Expander] Status
 */
void dscKeybusInterface::printPanel_0x28() {
  std::cout << "Zone expander query" << std::endl;
}


/*
 *  0x2D: Status with zones 9-16
 *  Interval: 4m
 *  CRC: yes
 *  Byte 2: Partition 1 lights
 *  Byte 3: Partition 1 status
 *  Byte 4: Partition 2 lights
 *  Byte 5: Partition 2 status
 *  Byte 6: Zones 9-16
 *
 *  00101101 0 10000000 00000011 10000001 11000111 00000001 11111001 [0x2D] Status lights: Backlight | Partition not ready | Open zones: 9
 *  00101101 0 10000000 00000011 10000010 00000101 00000000 00110111 [0x2D] Status lights: Backlight | Zones open | Zones 9-16 open: none  // PC1832
 */
void dscKeybusInterface::printPanel_0x2D() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1: " << std::endl;
  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);

  if (panelData[5] == 0xC7) {
    std::cout << " | Partition 2: disabled" << std::endl;
  }
  else if (panelData[5] != 0xFF) {
    std::cout << " | Partition 2: " << std::endl;
    printPanelLights(4);
    std::cout << "- " << std::endl;
    printPanelMessages(5);
  }

  std::cout << " | Zones 9-16 open: " << std::endl;
  if (panelData[6] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(6,9);
  }
}


/*
 *  0x34: Status with zones 17-24
 *  Interval: 4m
 *  CRC: yes
 *  Byte 2: Partition 1 lights
 *  Byte 3: Partition 1 status
 *  Byte 4: Partition 2 lights
 *  Byte 5: Partition 2 status
 *  Byte 6: Zones 17-24
 */
void dscKeybusInterface::printPanel_0x34() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1: " << std::endl;
  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);

  if (panelData[5] == 0xC7) {
    std::cout << " | Partition 2: disabled" << std::endl;
  }
  else if (panelData[5] != 0xFF) {
    std::cout << " | Partition 2: " << std::endl;
    printPanelLights(4);
    std::cout << "- " << std::endl;
    printPanelMessages(5);
  }

  std::cout << " | Zones 17-24 open: " << std::endl;
  if (panelData[6] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(6,17);
  }
}


/*
 *  0x3E: Status with zones 25-32
 *  Interval: 4m
 *  CRC: yes
 *  Byte 2: Partition 1 lights
 *  Byte 3: Partition 1 status
 *  Byte 4: Partition 2 lights
 *  Byte 5: Partition 2 status
 *  Byte 6: Zones 25-32
 */
void dscKeybusInterface::printPanel_0x3E() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1: " << std::endl;
  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);

  if (panelData[5] == 0xC7) {
    std::cout << " | Partition 2: disabled" << std::endl;
  }
  else if (panelData[5] != 0xFF) {
    std::cout << " | Partition 2: " << std::endl;
    printPanelLights(4);
    std::cout << "- " << std::endl;
    printPanelMessages(5);
  }

  std::cout << " | Zones 25-32 open: " << std::endl;
  if (panelData[6] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(6,25);
  }
}


/*
 *  0x4C: Keybus query
 *  Interval: immediate after exiting *8 programming, immediate on keypad query
 *  CRC: no
 *
 *  11111111 1 11111111 11111111 11111110 11111111 [Keypad] Keybus notification
 *  01001100 0 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 [0x4C] Keybus query
 */
void dscKeybusInterface::printPanel_0x4C() {
  std::cout << "Keybus query" << std::endl;
}


/*
 *  0x58: Keybus query - valid response produces 0xA5 command, byte 6 0xB1-0xC0
 *  Interval: immediate after power on after panel reset
 *  CRC: no
 *
 *  11111111 1 11111111 11111111 11111111 11011111
 *  01011000 0 10101010 10101010 10101010 10101010 [0x58] Keybus query
 *  11111111 1 11111100 11111111 11111111 11111111
 *  10100101 0 00011000 01010101 01000000 11010111 10110011 11111111 11011011 [0xA5] 05/10/2018 00:53 | Unrecognized data, add to 0xA5_Byte7_0xFF, Byte 6: 0xB3
 */
void dscKeybusInterface::printPanel_0x58() {
  std::cout << "Keybus query" << std::endl;
}


/*
 *  0x5D: Flash panel lights: status and zones 1-32, partition 1
 *  Interval: 30s
 *  CRC: yes
 *  Byte 2: Status lights
 *  Byte 3: Zones 1-8
 *  Byte 4: Zones 9-16
 *  Byte 5: Zones 17-24
 *  Byte 6: Zones 25-32
 *
 *  01011101 0 00000000 00000000 00000000 00000000 00000000 01011101 [0x5D] Partition 1 | Status lights flashing: none | Zones 1-32 flashing: none
 *  01011101 0 00100000 00000000 00000000 00000000 00000000 01111101 [0x5D] Partition 1 | Status lights flashing: Program | Zones 1-32 flashing: none
 *  01011101 0 00000000 00100000 00000000 00000000 00000000 01111101 [0x5D] Partition 1 | Status lights flashing: none  | Zones 1-32 flashing: none 6
 *  01011101 0 00000100 00100000 00000000 00000000 00000000 10000001 [0x5D] Partition 1 | Status lights flashing: Memory | Zones 1-32 flashing: 6
 *  01011101 0 00000000 00000000 00000001 00000000 00000000 01011110 [0x5D] Partition 1 | Status lights flashing: none | Zones flashing: 9
 */
void dscKeybusInterface::printPanel_0x5D() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1 | Status lights flashing: " << std::endl;
  printPanelLights(2);

  bool zoneLights = false;
  std::cout << "| Zones 1-32 flashing: " << std::endl;
  for (uint8_t panelByte = 3; panelByte <= 6; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte-3) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0x63: Flash panel lights: status and zones 1-32, partition 2
 *  Interval: 30s
 *  CRC: yes
 *  Byte 2: Status lights
 *  Byte 3: Zones 1-8
 *  Byte 4: Zones 9-16
 *  Byte 5: Zones 17-24
 *  Byte 6: Zones 25-32
 *
 *  01100011 0 00000000 00000000 00000000 00000000 00000000 01100011 [0x63] Partition 2 | Status lights flashing: none | Zones 1-32 flashing: none
 *  01100011 0 00000100 10000000 00000000 00000000 00000000 11100111 [0x63] Partition 2 | Status lights flashing:Memory | Zones 1-32 flashing: 8
 */
void dscKeybusInterface::printPanel_0x63() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 2 | Status lights flashing: " << std::endl;
  printPanelLights(2);

  bool zoneLights = false;
  std::cout << "| Zones 1-32 flashing: " << std::endl;
  for (uint8_t panelByte = 3; panelByte <= 6; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte-3) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0x64: Beep - one-time, partition 1
 *  CRC: yes
 *
 *  01100100 0 00001100 01110000 [0x64] Partition 1 | Beep: 6 beeps
 */
void dscKeybusInterface::printPanel_0x64() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1 | Beep: " << std::endl;
  switch (panelData[2]) {
    case 0x04: std::cout << "2 beeps" << std::endl; break;
    case 0x06: std::cout << "3 beeps" << std::endl; break;
    case 0x08: std::cout << "4 beeps" << std::endl; break;
    case 0x0C: std::cout << "6 beeps" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0x69: Beep - one-time, partition 2
 *  CRC: yes
 *
 *  01101001 0 00001100 01110101 [0x69] Partition 2 | Beep: 6 beeps
 */
void dscKeybusInterface::printPanel_0x69() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 2 | Beep: " << std::endl;
  switch (panelData[2]) {
    case 0x04: std::cout << "2 beeps" << std::endl; break;
    case 0x06: std::cout << "3 beeps" << std::endl; break;
    case 0x08: std::cout << "4 beeps" << std::endl; break;
    case 0x0C: std::cout << "6 beeps" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0x75: Beep pattern - repeated, partition 1
 *  CRC: yes
 *
 *  01110101 0 10000000 11110101 [0x75] Partition 1 | Beep pattern: solid tone
 *  01110101 0 00000000 01110101 [0x75] Partition 1 | Beep pattern: off
 */
void dscKeybusInterface::printPanel_0x75() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1 | Beep pattern: " << std::endl;
  switch (panelData[2]) {
    case 0x00: std::cout << "off" << std::endl; break;
    case 0x11: std::cout << "single beep (exit delay)" << std::endl; break;
    case 0x31: std::cout << "triple beep (exit delay)" << std::endl; break;
    case 0x80: std::cout << "solid tone" << std::endl; break;
    case 0xB1: std::cout << "triple beep (entrance delay)" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0x7A: Beep pattern - repeated, partition 2
 *  CRC: yes
 *
 *  01111010 0 00000000 01111010 [0x7A] Partition 2 | Beep pattern: off
 */
void dscKeybusInterface::printPanel_0x7A() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 2 | Beep pattern: " << std::endl;
  switch (panelData[2]) {
    case 0x00: std::cout << "off" << std::endl; break;
    case 0x11: std::cout << "single beep (exit delay)" << std::endl; break;
    case 0x31: std::cout << "triple beep (exit delay)" << std::endl; break;
    case 0x80: std::cout << "solid tone" << std::endl; break;
    case 0xB1: std::cout << "triple beep (entrance delay)" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0x7F: Beep - one-time
 *  CRC: yes
 *
 *  01111111 0 00000001 10000000 [0x7F] Beep: long beep
 */
void dscKeybusInterface::printPanel_0x7F() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 1 | " << std::endl;
  switch (panelData[2]) {
    case 0x01: std::cout << "Beep: long beep" << std::endl; break;
    case 0x02: std::cout << "Beep: long beep | Failed to arm" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0x82: Beep - one-time
 *  CRC: yes
 *
 *  01111111 0 00000001 10000000 [0x82] Beep: long beep
 */
void dscKeybusInterface::printPanel_0x82() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Partition 2 | " << std::endl;
  switch (panelData[2]) {
    case 0x01: std::cout << "Beep: long beep" << std::endl; break;
    case 0x02: std::cout << "Beep: long beep | Failed to arm" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0x87: Panel outputs
 *  CRC: yes
 *
 *  10000111 0 00000000 00000000 10000111 [0x87] Panel output: Bell off | PGM1 off | PGM2 off
 *  10000111 0 11111111 11110000 01110110 [0x87] Panel output: Bell on | PGM1 off | PGM2 off
 *  10000111 0 11111111 11110010 01111000 [0x87] Panel output: Bell on | PGM1 off | PGM2 on
 *  10000111 0 00000000 00000001 10001000 [0x87] Panel output: Bell off | PGM1 on | PGM2 off
 *  10000111 0 00000000 00001000 10001111 [0x87] Panel output: Bell off | Unrecognized command: Add to 0x87
 */
void dscKeybusInterface::printPanel_0x87() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Panel output:" << std::endl;
  switch (panelData[2] & 0xF0) {
    case 0xF0: std::cout << " Bell on" << std::endl; break;
    default: std::cout << " Bell off" << std::endl; break;
  }

  if ((panelData[3] & 0x0F) <= 0x03) {
    if (bitRead(panelData[3],0)) std::cout << " | PGM1 on" << std::endl;
    else std::cout << " | PGM1 off" << std::endl;

    if (bitRead(panelData[3],1)) std::cout << " | PGM2 on" << std::endl;
    else std::cout << " | PGM2 off" << std::endl;
  }
  else std::cout << " | Unrecognized data" << std::endl;

  if ((panelData[2] & 0x0F) != 0x0F) {
    if (bitRead(panelData[2],0)) std::cout << " | PGM3 on" << std::endl;
    else std::cout << " | PGM3 off" << std::endl;

    if (bitRead(panelData[2],1)) std::cout << " | PGM4 on" << std::endl;
    else std::cout << " | PGM4 off" << std::endl;
  }
}


/*
 *  0x8D: User code programming key response, codes 17-32
 *  CRC: yes
 *  Byte 2: TBD
 *  Byte 3: TBD
 *  Byte 4: TBD
 *  Byte 5: TBD
 *  Byte 6: TBD
 *  Byte 7: TBD
 *  Byte 8: TBD
 *
 *  10001101 0 00110001 00000001 00000000 00010111 11111111 11111111 11111111 11010011 [0x8D]   // Code 17 Key 1
 *  10001101 0 00110001 00000100 00000000 00011000 11111111 11111111 11111111 11010111 [0x8D]   // Code 18 Key 1
 *  10001101 0 00110001 00000100 00000000 00010010 11111111 11111111 11111111 11010001 [0x8D]   // Code 18 Key 2
 *  10001101 0 00110001 00000101 00000000 00111000 11111111 11111111 11111111 11111000 [0x8D]   // Code 18 Key 3
 *  10001101 0 00110001 00000101 00000000 00110100 11111111 11111111 11111111 11110100 [0x8D]   // Code 18 Key 4
 *  10001101 0 00110001 00100101 00000000 00001001 11111111 11111111 11111111 11101001 [0x8D]   // Code 29 Key 0
 *  10001101 0 00110001 00100101 00000000 00000001 11111111 11111111 11111111 11100001 [0x8D]   // Code 29 Key 1
 *  10001101 0 00110001 00110000 00000000 00000000 11111111 11111111 11111111 11101011 [0x8D]   // Message after 4th key entered
 */
void dscKeybusInterface::printPanel_0x8D() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "User code programming key response" << std::endl;
}


/*
 *  0x94: ???
 *  Interval: immediate after entering *5 access code programming
 *  CRC: no
 *  Byte 2: TBD
 *  Byte 3: TBD
 *  Byte 4: TBD
 *  Byte 5: TBD
 *  Byte 6: TBD
 *  Byte 7: TBD
 *  Byte 8: TBD
 *  Byte 9: TBD
 *
 *  10010100 0 00010001 00000000 00000000 10100101 00000000 00000000 00000000 00010111 10100000 [0x94] Unknown command 1
 *  10010100 0 00010001 00000000 00000000 10100101 00000000 00000000 00000000 01001100 11111100 [0x94] Unknown command 2
 */
void dscKeybusInterface::printPanel_0x94() {
  switch (panelData[9]) {
    case 0x17: std::cout << "Unknown command 1" << std::endl; break;
    case 0x4C: std::cout << "Unknown command 2" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl;
  }
}


/*
 *  0xA5: Date, time, system status messages - partitions 1-2
 *  CRC: yes
 */
void dscKeybusInterface::printPanel_0xA5() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  /*
   *  Date and time
   *  Interval: 4m
   *             YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
   *  10100101 0 00011000 00001110 11101101 10000000 00000000 00000000 00111000 [0xA5] 03/23/2018 13:32 | Timestamp
   */
  uint8_t dscYear3 = panelData[2] >> 4;
  uint8_t dscYear4 = panelData[2] & 0x0F;
  uint8_t dscMonth = panelData[3] << 2; dscMonth >>=4;
  uint8_t dscDay1 = panelData[3] << 6; dscDay1 >>= 3;
  uint8_t dscDay2 = panelData[4] >> 5;
  uint8_t dscDay = dscDay1 | dscDay2;
  uint8_t dscHour = panelData[4] & 0x1F;
  uint8_t dscMinute = panelData[5] >> 2;

  if (dscYear3 >= 7) std::cout << "19" << std::endl;
  else std::cout << "20" << std::endl;
  std::cout << dscYear3 << std::endl;
  std::cout << dscYear4 << std::endl;
  std::cout << "." << std::endl;
  if (dscMonth < 10) std::cout << "0" << std::endl;
  std::cout << dscMonth << std::endl;
  std::cout << "." << std::endl;
  if (dscDay < 10) std::cout << "0" << std::endl;
  std::cout << dscDay << std::endl;
  std::cout << " " << std::endl;
  if (dscHour < 10) std::cout << "0" << std::endl;
  std::cout << dscHour << std::endl;
  std::cout << ":" << std::endl;
  if (dscMinute < 10) std::cout << "0" << std::endl;
  std::cout << dscMinute << std::endl;

  if (panelData[6] == 0 && panelData[7] == 0) {
    std::cout << " | Timestamp" << std::endl;
    return;
  }

  switch (panelData[3] >> 6) {
    case 0x00: std::cout << " | " << std::endl; break;
    case 0x01: std::cout << " | Partition 1 | " << std::endl; break;
    case 0x02: std::cout << " | Partition 2 | " << std::endl; break;
  }

  switch (panelData[5] & 0x03) {
    case 0x00: printPanelStatus0(6); return;
    case 0x01: printPanelStatus1(6); return;
    case 0x02: printPanelStatus2(6); return;
    case 0x03: printPanelStatus3(6); return;
  }
}


/*
 *  0xB1: Enabled zones 1-32, partitions 1,2
 *  Interval: 4m
 *  CRC: yes
 *  Bytes 2-5: partition 1
 *  Bytes 6-9: partition 2
 *
 *  10110001 0 11111111 00000000 00000000 00000000 00000000 00000000 00000000 00000000 10110000 [0xB1] Enabled zones - Partition 1: 1 2 3 4 5 6 7 8 | Partition 2: none
 *  10110001 0 10010001 10001010 01000001 10100100 00000000 00000000 00000000 00000000 10110001 [0xB1] Enabled zones - Partition 1: 1 5 8 10 12 16 17 23 27 30 32 | Partition 2: none
 *  10110001 0 11111111 00000000 00000000 11111111 00000000 00000000 00000000 00000000 10101111 [0xB1] Enabled zones - Partition 1: 1 2 3 4 5 6 7 8 25 26 27 28 29 30 31 32 | Partition 2: none
 *  10110001 0 01111111 11111111 00000000 00000000 10000000 00000000 00000000 00000000 10101111 [0xB1] Enabled zones - Partition 1: 1 2 3 4 5 6 7 9 10 11 12 13 14 15 16 | Partition 2: 8
 */
void dscKeybusInterface::printPanel_0xB1() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  bool enabledZones = false;
  std::cout << "Enabled zones 1-32 | Partition 1: " << std::endl;
  for (uint8_t panelByte = 2; panelByte <= 5; panelByte++) {
    if (panelData[panelByte] != 0) {
      enabledZones = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte - 2) * 8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!enabledZones) std::cout << "none " << std::endl;

  enabledZones = false;
  std::cout << "| Partition 2: " << std::endl;
  for (uint8_t panelByte = 6; panelByte <= 9; panelByte++) {
    if (panelData[panelByte] != 0) {
      enabledZones = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte - 6) * 8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!enabledZones) std::cout << "none" << std::endl;
}


/*
 *  0xBB: Bell
 *  Interval: immediate after alarm tripped except silent zones
 *  CRC: yes
 *
 *  10111011 0 00100000 00000000 11011011 [0xBB] Bell: on
 *  10111011 0 00000000 00000000 10111011 [0xBB] Bell: off
 */
void dscKeybusInterface::printPanel_0xBB() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  std::cout << "Bell: " << std::endl;
  if (bitRead(panelData[2],5)) std::cout << "on" << std::endl;
  else std::cout << "off" << std::endl;
}


/*
 *  0xC3: Keypad status
 *  Interval: 30s (PC1616/PC1832/PC1864)
 *  CRC: yes
 *
 *  11000011 0 00010000 11111111 11010010 [0xC3] Unknown command 1: Power-on +33s
 *  11000011 0 00110000 11111111 11110010 [0xC3] Keypad lockout
 *  11000011 0 00000000 11111111 11000010 [0xC3] Keypad ready
 */
void dscKeybusInterface::printPanel_0xC3() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  if (panelData[3] == 0xFF) {
    switch (panelData[2]) {
      case 0x00: std::cout << "Keypad ready" << std::endl; break;
      case 0x10: std::cout << "Unknown command 1: Power-on +33s" << std::endl; break;
      case 0x30:
      case 0x40: std::cout << "Keypad lockout" << std::endl; break;
      default: std::cout << "Unrecognized data" << std::endl; break;
    }
  }
  else std::cout << "Unrecognized data" << std::endl;
}


/*
 *  0xCE: Unknown command
 *  CRC: yes
 *
 * 11001110 0 00000001 10100000 00000000 00000000 00000000 01101111 [0xCE]  // Partition 1 exit delay
 * 11001110 0 00000001 10110001 00000000 00000000 00000000 10000000 [0xCE]  // Partition 1 armed stay
 * 11001110 0 00000001 10110011 00000000 00000000 00000000 10000010 [0xCE]  // Partition 1 armed away
 * 11001110 0 00000001 10100100 00000000 00000000 00000000 01110011 [0xCE]  // Partition 2 armed away
 * 11001110 0 01000000 11111111 11111111 11111111 11111111 00001010 [0xCE]  // Partition 1,2 activity
 */
void dscKeybusInterface::printPanel_0xCE() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  switch (panelData[2]) {
    case 0x01: {
      switch (panelData[3]) {
        case 0xA0: std::cout << "Partition 1,2 exit delay, partition 1,2 disarmed" << std::endl; break;
        case 0xA4: std::cout << "Partition 2 armed away" << std::endl; break;
        case 0xB1: std::cout << "Partition 1 armed stay" << std::endl; break;
        case 0xB3: std::cout << "Partition 1 armed away" << std::endl; break;
        default: std::cout << "Unrecognized data" << std::endl; break;
      }
      break;
    }
    case 0x40: std::cout << "Partition 1,2 activity" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}

/*
 *  0xD5: Keypad zone query
 *  CRC: no
 *
 *  11111111 1 11111111 11111111 11111111 11111011 [Keypad] Status notification
 *  11010101 0 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 [0xD5] Keypad zone query
 *  11111111 1 11111111 11111111 11111111 11111111 11111111 11111111 11111111 00001111 [Keypad] Slot 8
 */
void dscKeybusInterface::printPanel_0xD5() {
  std::cout << "Keypad zone query" << std::endl;
}


/*
 *  0xE6: Status, partitions 1-8
 *  CRC: yes
 *  Panels: PC5020, PC1616, PC1832, PC1864
 */
void dscKeybusInterface::printPanel_0xE6() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  switch (panelData[2]) {
    case 0x03: printPanel_0xE6_0x03(); break;  // Status in alarm/programming, partitions 5-8
    case 0x09: printPanel_0xE6_0x09(); break;  // Zones 33-40 status
    case 0x0B: printPanel_0xE6_0x0B(); break;  // Zones 41-48 status
    case 0x0D: printPanel_0xE6_0x0D(); break;  // Zones 49-56 status
    case 0x0F: printPanel_0xE6_0x0F(); break;  // Zones 57-64 status
    case 0x17: printPanel_0xE6_0x17(); break;  // Flash panel lights: status and zones 1-32, partitions 1-8
    case 0x18: printPanel_0xE6_0x18(); break;  // Flash panel lights: status and zones 33-64, partitions 1-8
    case 0x19: printPanel_0xE6_0x19(); break;  // Beep - one-time, partitions 3-8
    case 0x1A: printPanel_0xE6_0x1A(); break;  // Unknown command
    case 0x1D: printPanel_0xE6_0x1D(); break;  // Beep pattern, partitions 3-8
    case 0x20: printPanel_0xE6_0x20(); break;  // Status in programming, zone lights 33-64
    case 0x2B: printPanel_0xE6_0x2B(); break;  // Enabled zones 1-32, partitions 3-8
    case 0x2C: printPanel_0xE6_0x2C(); break;  // Enabled zones 33-64, partitions 3-8
    case 0x41: printPanel_0xE6_0x41(); break;  // Status in access code programming, zone lights 65-95
    default: std::cout << "Unrecognized data" << std::endl;
  }
}


/*
 *  0xE6_0x03: Status in alarm/programming, partitions 5-8
 */
void dscKeybusInterface::printPanel_0xE6_0x03() {
  printPanelLights(2);
  std::cout << "- " << std::endl;
  printPanelMessages(3);
}


/*
 *  0xE6_0x09: Zones 33-40 status
 */
void dscKeybusInterface::printPanel_0xE6_0x09() {
  std::cout << "Zones 33-40 open: " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,33);
  }
}


/*
 *  0xE6_0x0B: Zones 41-48 status
 */
void dscKeybusInterface::printPanel_0xE6_0x0B() {
  std::cout << "Zones 41-48 open: " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,41);
  }
}


/*
 *  0xE6_0x0D: Zones 49-56 status
 */
void dscKeybusInterface::printPanel_0xE6_0x0D() {
  std::cout << "Zones 49-56 open: " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,49);
  }
}


/*
 *  0xE6_0x0F: Zones 57-64 status
 */
void dscKeybusInterface::printPanel_0xE6_0x0F() {
  std::cout << "Zones 57-64 open: " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,57);
  }
}


/*
 *  0xE6_0x17: Flash panel lights: status and zones 1-32, partitions 1-8
 *
 *  11100110 0 00010111 00000100 00000000 00000100 00000000 00000000 00000000 00000101 [0xE6] Partition 3 |  // Zone 3
 */
void dscKeybusInterface::printPanel_0xE6_0x17() {
  std::cout << "Partition " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,1);
  }

  std::cout << "| Status lights flashing: " << std::endl;
  printPanelLights(4);

  bool zoneLights = false;
  std::cout << "| Zones 1-32 flashing: " << std::endl;
  for (uint8_t panelByte = 5; panelByte <= 8; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte-5) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0xE6_0x18: Flash panel lights: status and zones 33-64, partitions 1-8
 *
 *  11100110 0 00011000 00000001 00000000 00000001 00000000 00000000 00000000 00000000 [0xE6] Partition 1 |  // Zone 33
 *  11100110 0 00011000 00000001 00000100 00000000 00000000 00000000 10000000 10000011 [0xE6] Partition 1 |  // Zone 64
 */
void dscKeybusInterface::printPanel_0xE6_0x18() {
  std::cout << "Partition " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,1);
  }

  std::cout << "| Status lights flashing: " << std::endl;
  printPanelLights(4);

  bool zoneLights = false;
  std::cout << "| Zones 33-64 flashing: " << std::endl;
  for (uint8_t panelByte = 5; panelByte <= 8; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 33) + ((panelByte-5) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0xE6_0x19: Beep - one time, partitions 3-8
 */
void dscKeybusInterface::printPanel_0xE6_0x19() {
  std::cout << "Partition " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,1);
  }

  std::cout << "| Beep: " << std::endl;
  switch (panelData[4]) {
    case 0x04: std::cout << "2 beeps" << std::endl; break;
    case 0x06: std::cout << "3 beeps" << std::endl; break;
    case 0x08: std::cout << "4 beeps" << std::endl; break;
    case 0x0C: std::cout << "6 beeps" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


void dscKeybusInterface::printPanel_0xE6_0x1A() {
  std::cout << "0x1A: " << std::endl;
  std::cout << "Unrecognized data" << std::endl;
}


/*
 *  0xE6_0x1D: Beep pattern, partitions 3-8
 */
void dscKeybusInterface::printPanel_0xE6_0x1D() {
  std::cout << "Partition " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,1);
  }

  std::cout << "| Beep pattern: " << std::endl;
  switch (panelData[4]) {
    case 0x00: std::cout << "off" << std::endl; break;
    case 0x11: std::cout << "single beep (exit delay)" << std::endl; break;
    case 0x31: std::cout << "triple beep (exit delay)" << std::endl; break;
    case 0x80: std::cout << "solid tone" << std::endl; break;
    case 0xB1: std::cout << "triple beep (entrance delay)" << std::endl; break;
    default: std::cout << "Unrecognized data" << std::endl; break;
  }
}


/*
 *  0xE6_0x20: Status in programming, zone lights 33-64
 *  Interval: constant in *8 programming
 *  CRC: yes
 */
void dscKeybusInterface::printPanel_0xE6_0x20() {
  std::cout << "Status lights: " << std::endl;
  printPanelLights(3);
  std::cout << "- " << std::endl;
  printPanelMessages(4);

  bool zoneLights = false;
  std::cout << " | Zone lights: " << std::endl;
  for (uint8_t panelByte = 5; panelByte <= 8; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 33) + ((panelByte-5) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }

  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0xE6_0x2B: Enabled zones 1-32, partitions 3-8
 */
void dscKeybusInterface::printPanel_0xE6_0x2B() {
  std::cout << "Partition " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,1);
  }

  bool enabledZones = false;
  std::cout << "| Enabled zones  1-32: " << std::endl;
  for (uint8_t panelByte = 4; panelByte <= 7; panelByte++) {
    if (panelData[panelByte] != 0) {
      enabledZones = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 1) + ((panelByte - 4) * 8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!enabledZones) std::cout << "none" << std::endl;
}


/*
 *  0xE6_0x2C: Enabled zones 33-64, partitions 1-8
 */
void dscKeybusInterface::printPanel_0xE6_0x2C() {
  std::cout << "Partition " << std::endl;
  if (panelData[3] == 0) std::cout << "none" << std::endl;
  else {
    printPanelBitNumbers(3,1);
  }

  bool enabledZones = false;
  std::cout << "| Enabled zones 33-64: " << std::endl;
  for (uint8_t panelByte = 4; panelByte <= 7; panelByte++) {
    if (panelData[panelByte] != 0) {
      enabledZones = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 33) + ((panelByte - 4) * 8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }
  if (!enabledZones) std::cout << "none" << std::endl;
}


/*
 *  0xE6_0x41: Status in programming, zone lights 65-95
 *  CRC: yes
 */
void dscKeybusInterface::printPanel_0xE6_0x41() {
  std::cout << "Status lights: " << std::endl;
  printPanelLights(3);
  std::cout << "- " << std::endl;
  printPanelMessages(4);

  bool zoneLights = false;
  std::cout << " | Zone lights: " << std::endl;
  for (uint8_t panelByte = 5; panelByte <= 8; panelByte++) {
    if (panelData[panelByte] != 0) {
      zoneLights = true;
      for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
        if (bitRead(panelData[panelByte],zoneBit)) {
          std::cout << (zoneBit + 65) + ((panelByte-5) *  8) << std::endl;
          std::cout << " " << std::endl;
        }
      }
    }
  }

  if (!zoneLights) std::cout << "none" << std::endl;
}


/*
 *  0xEB: Date, time, system status messages - partitions 1-8
 *  CRC: yes
 *
 *                     YYY1YYY2   MMMMDD DDDHHHHH MMMMMM
 * 11101011 0 00000001 00011000 00011000 10001010 00101100 00000000 10111011 00000000 10001101 [0xEB] 06/04/2018 10:11 | Partition: 1  // Armed stay
 * 11101011 0 00000001 00011000 00011000 10001010 00111000 00000000 10111011 00000000 10011001 [0xEB] 06/04/2018 10:14 | Partition: 1  // Armed away
 * 11101011 0 00000001 00011000 00011000 10001010 00111000 00000010 10011011 00000000 01111011 [0xEB] 06/04/2018 10:14 | Partition: 1  // Armed away
 * 11101011 0 00000001 00011000 00011000 10001010 00110100 00000000 11100010 00000000 10111100 [0xEB] 06/04/2018 10:13 | Partition: 1  // Disarmed
 * 11101011 0 00000001 00011000 00011000 10001111 00101000 00000100 00000000 10010001 01101000 [0xEB] 06/04/2018 15:10 | Partition: 1 | Unrecognized data, add to printPanelStatus0, Byte 8: 0x00
 * 11101011 0 00000001 00000001 00000100 01100000 00010100 00000100 01000000 10000001 00101010 [0xEB] 2001.01.03 00:05 | Partition 1 | Zone tamper: 33
 * 11101011 0 00000001 00000001 00000100 01100000 00001000 00000100 01011111 10000001 00111101 [0xEB] 2001.01.03 00:02 | Partition 1 | Zone tamper: 64
 * 11101011 0 00000001 00000001 00000100 01100000 00011000 00000100 01100000 11111111 11001100 [0xEB] 2001.01.03 00:06 | Partition 1 | Zone tamper restored: 33
 * 11101011 0 00000000 00000001 00000100 01100000 01001000 00010100 01100000 10000001 10001101 [0xEB] 2001.01.03 00:18 | Zone fault: 33
 * 11101011 0 00000000 00000001 00000100 01100000 01001100 00010100 01000000 11111111 11101111 [0xEB] 2001.01.03 00:19 | Zone fault restored: 33
 * 11101011 0 00000000 00000001 00000100 01100000 00001100 00010100 01011111 11111111 11001110 [0xEB] 2001.01.03 00:03 | Zone fault restored: 64
 */
void dscKeybusInterface::printPanel_0xEB() {
  if (!validCRC()) {
    std::cout << "[CRC Error]" << std::endl;
    return;
  }

  uint8_t dscYear3 = panelData[3] >> 4;
  uint8_t dscYear4 = panelData[3] & 0x0F;
  uint8_t dscMonth = panelData[4] << 2; dscMonth >>=4;
  uint8_t dscDay1 = panelData[4] << 6; dscDay1 >>= 3;
  uint8_t dscDay2 = panelData[5] >> 5;
  uint8_t dscDay = dscDay1 | dscDay2;
  uint8_t dscHour = panelData[5] & 0x1F;
  uint8_t dscMinute = panelData[6] >> 2;

  if (dscYear3 >= 7) std::cout << "19" << std::endl;
  else std::cout << "20" << std::endl;
  std::cout << dscYear3 << std::endl;
  std::cout << dscYear4 << std::endl;
  std::cout << "." << std::endl;
  if (dscMonth < 10) std::cout << "0" << std::endl;
  std::cout << dscMonth << std::endl;
  std::cout << "." << std::endl;
  if (dscDay < 10) std::cout << "0" << std::endl;
  std::cout << dscDay << std::endl;
  std::cout << " " << std::endl;
  if (dscHour < 10) std::cout << "0" << std::endl;
  std::cout << dscHour << std::endl;
  std::cout << ":" << std::endl;
  if (dscMinute < 10) std::cout << "0" << std::endl;
  std::cout << dscMinute << std::endl;

  if (panelData[2] == 0) std::cout << " | " << std::endl;
  else {
    std::cout << " | Partition " << std::endl;
    printPanelBitNumbers(2,1);
    std::cout << "| " << std::endl;
  }

  switch (panelData[7]) {
    case 0x00: printPanelStatus0(8); return;
    case 0x01: printPanelStatus1(8); return;
    case 0x02: printPanelStatus2(8); return;
    case 0x03: printPanelStatus3(8); return;
    case 0x04: printPanelStatus4(8); return;
    case 0x14: printPanelStatus14(8); return;
  }
}


/*
 *  Print keypad and module messages
 */


/*
 *  Keypad: Fire alarm
 *
 *  01110111 1 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Fire alarm
 */
void dscKeybusInterface::printModule_0x77() {
  std::cout << "[Keypad] Fire alarm" << std::endl;
}


/*
 *  Keypad: Auxiliary alarm
 *
 *  10111011 1 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Aux alarm
 */
void dscKeybusInterface::printModule_0xBB() {
  std::cout << "[Keypad] Auxiliary alarm" << std::endl;
}


/*
 *  Keypad: Panic alarm
 *
 *  11011101 1 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Panic alarm
 */
void dscKeybusInterface::printModule_0xDD() {
  std::cout << "[Keypad] Panic alarm" << std::endl;
}


/*
 *  Keybus status notifications
 */

void dscKeybusInterface::printModule_Notification() {
  switch (moduleData[4]) {
    // Zone expander: status update notification, panel responds with 0x28
    // 11111111 1 11111111 11111111 10111111 11111111 [Zone Expander] Status notification
    // 00101000 0 11111111 11111111 11111111 11111111 11111111 [0x28] Zone expander query
    case 0xBF:
      std::cout << "[Zone Expander] Status notification" << std::endl;
      break;

    // Keypad: Unknown Keybus notification, panel responds with 0x4C query
    // 11111111 1 11111111 11111111 11111110 11111111 [Keypad] Unknown Keybus notification
    // 01001100 0 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 [0x4C] Unknown Keybus query
    case 0xFE:
      std::cout << "[Keypad] Unknown Keybus notification" << std::endl;
      break;
  }

  switch (moduleData[5]) {
    // Keypad: zone status update notification, panel responds with 0xD5 query
    // 11111111 1 11111111 11111111 11111111 11111011 [Keypad] Zone status notification
    // 11010101 0 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 [0xD5] Keypad zone query
    case 0xFB:
      std::cout << "[Keypad] Zone status notification" << std::endl;
      break;
  }
}


/*
 *  Keypad: Slot query response
 *
 *  00010001 0 10101010 10101010 10101010 10101010 10101010 [0x11] Keypad slot query
 *  11111111 1 00111111 11111111 11111111 11111111 11111111 [Keypad] Slots active: 1
 */
void dscKeybusInterface::printModule_Panel_0x11() {
  std::cout << "[Keypad] Slots active: " << std::endl;
  if ((moduleData[2] & 0xC0) == 0) std::cout << "1 " << std::endl;
  if ((moduleData[2] & 0x30) == 0) std::cout << "2 " << std::endl;
  if ((moduleData[2] & 0x0C) == 0) std::cout << "3 " << std::endl;
  if ((moduleData[2] & 0x03) == 0) std::cout << "4 " << std::endl;
  if ((moduleData[3] & 0xC0) == 0) std::cout << "5 " << std::endl;
  if ((moduleData[3] & 0x30) == 0) std::cout << "6 " << std::endl;
  if ((moduleData[3] & 0x0C) == 0) std::cout << "7 " << std::endl;
  if ((moduleData[3] & 0x03) == 0) std::cout << "8 " << std::endl;
}


/*
 *  Keypad: Panel 0xD5 zone query response
 *  Bytes 2-9: Keypad slots 1-8
 *  Bits 2,3: TBD
 *  Bits 6,7: TBD
 *
 *  11111111 1 11111111 11111111 11111111 11111011 [Keypad] Status update
 *  11010101 0 10101010 10101010 10101010 10101010 10101010 10101010 10101010 10101010 [0xD5] Keypad zone query
 *
 *  11111111 1 00000011 11111111 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1 | Zone open
 *  11111111 1 00001111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1 | Zone open  // Exit *8 programming
 *  11111111 1 11111111 11111111 11111111 11111111 11111111 11111111 11111111 00001111 [Keypad] Slot 8 | Zone open  // Exit *8 programming
 *
 *  11111111 1 11110000 11111111 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1   //Zone closed while unconfigured
 *  11111111 1 11110011 11111111 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1   //Zone closed while unconfigured after opened once
 *  11111111 1 00110000 11111111 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1 | Zone closed // NC
 *  11111111 1 00111100 11111111 11111111 11111111 11111111 11111111 11111111 11111111 [Keypad] Slot 1 | Zone closed  //After exiting *8 programming after NC
 */
void dscKeybusInterface::printModule_Panel_0xD5() {
  std::cout << "[Keypad] " << std::endl;
  bool firstData = true;
  for (uint8_t moduleByte = 2; moduleByte <= 9; moduleByte++) {
    uint8_t slotData = moduleData[moduleByte];
    if (slotData < 0xFF) {
      if (firstData) std::cout << "Slot " << std::endl;
      else std::cout << " | Slot " << std::endl;
      std::cout << moduleByte - 1 << std::endl;
      if ((slotData & 0x03) == 0x03 && (slotData & 0x30) == 0) std::cout << " zone open" << std::endl;
      if ((slotData & 0x03) == 0 && (slotData & 0x30) == 0x30) std::cout << " zone closed" << std::endl;
      firstData = false;
    }
  }
}


/*
 *  Keypad: keys
 *
 *  11111111 1 00000101 11111111 11111111 11111111 [Keypad] 1
 *  11111111 1 00101101 11111111 11111111 11111111 [Keypad] #
 */
void dscKeybusInterface::printModule_Keys() {
  std::cout << "[Keypad] " << std::endl;

  uint8_t keyByte = 2;
  if (currentCmd == 0x05) {
    if (moduleData[2] != 0xFF) {
      std::cout << "Partition 1 | Key: " << std::endl;
    }
    else if (moduleData[3] != 0xFF) {
      std::cout << "Partition 2 | Key: " << std::endl;
      keyByte = 3;
    }
    else if (moduleData[8] != 0xFF) {
      std::cout << "Partition 3 | Key: " << std::endl;
      keyByte = 8;
    }

    else if (moduleData[9] != 0xFF) {
      std::cout << "Partition 4 | Key: " << std::endl;
      keyByte = 9;
    }
  }
  else if (currentCmd == 0x1B) {
    if (moduleData[2] != 0xFF) {
      std::cout << "Partition 5 | Key: " << std::endl;
    }
    else if (moduleData[3] != 0xFF) {
      std::cout << "Partition 6 | Key: " << std::endl;
      keyByte = 3;
    }
    else if (moduleData[8] != 0xFF) {
      std::cout << "Partition 7 | Key: " << std::endl;
      keyByte = 8;
    }

    else if (moduleData[9] != 0xFF) {
      std::cout << "Partition 8 | Key: " << std::endl;
      keyByte = 9;
    }
  }

  if (hideKeypadDigits && (moduleData[2] <= 0x27 || moduleData[3] <= 0x27 || moduleData[8] <= 0x27 || moduleData[9] <= 0x27)) {
    std::cout << "[Digit]" << std::endl;
    return;
  }

  switch (moduleData[keyByte]) {
    case 0x00: std::cout << "0" << std::endl; break;
    case 0x05: std::cout << "1" << std::endl; break;
    case 0x0A: std::cout << "2" << std::endl; break;
    case 0x0F: std::cout << "3" << std::endl; break;
    case 0x11: std::cout << "4" << std::endl; break;
    case 0x16: std::cout << "5" << std::endl; break;
    case 0x1B: std::cout << "6" << std::endl; break;
    case 0x1C: std::cout << "7" << std::endl; break;
    case 0x22: std::cout << "8" << std::endl; break;
    case 0x27: std::cout << "9" << std::endl; break;
    case 0x28: std::cout << "*" << std::endl; break;
    case 0x2D: std::cout << "#" << std::endl; break;
    case 0x52: std::cout << "Identified voice prompt help" << std::endl; break;
    case 0x70: std::cout << "Command output 3" << std::endl; break;
    case 0xAF: std::cout << "Arm stay" << std::endl; break;
    case 0xB1: std::cout << "Arm away" << std::endl; break;
    case 0xB6: std::cout << "*9 No entry delay arm, requires access code" << std::endl; break;
    case 0xBB: std::cout << "Door chime configuration" << std::endl; break;
    case 0xBC: std::cout << "*6 System test" << std::endl; break;
    case 0xC3: std::cout << "*1 Zone bypass programming" << std::endl; break;
    case 0xC4: std::cout << "*2 Trouble menu" << std::endl; break;
    case 0xC9: std::cout << "*3 Alarm memory display" << std::endl; break;
    case 0xCE: std::cout << "*5 Programming, requires master code" << std::endl; break;
    case 0xD0: std::cout << "*6 Programming, requires master code" << std::endl; break;
    case 0xD5: std::cout << "Command output 1" << std::endl; break;
    case 0xDA: std::cout << "Reset / Command output 2" << std::endl; break;
    case 0xDF: std::cout << "General voice prompt help" << std::endl; break;
    case 0xE1: std::cout << "Quick exit" << std::endl; break;
    case 0xE6: std::cout << "Activate stay/away zones" << std::endl; break;
    case 0xEB: std::cout << "Function key [20] Future Use" << std::endl; break;
    case 0xEC: std::cout << "Command output 4" << std::endl; break;
    case 0xF7: std::cout << "Left/right arrow" << std::endl; break;
    default:
      std::cout << "Unrecognized key: 0x" << std::endl;
      std::cout << moduleData[keyByte] << std::endl;
      break;
  }
}


/*
 * Print binary
 */

void dscKeybusInterface::printPanelBinary(bool printSpaces) {
  for (uint8_t panelByte = 0; panelByte < panelByteCount; panelByte++) {
    if (panelByte == 1) std::cout << panelData[panelByte] << std::endl;  // Prints the stop bit
    else {
      for (uint8_t mask = 0x80; mask; mask >>= 1) {
        if (mask & panelData[panelByte]) std::cout << "1" << std::endl;
        else std::cout << "0" << std::endl;
      }
    }
    if (printSpaces && (panelByte != panelByteCount - 1 || displayTrailingBits)) std::cout << " " << std::endl;
  }

  if (displayTrailingBits) {
    uint8_t trailingBits = (panelBitCount - 1) % 8;
    if (trailingBits > 0) {
      for (int i = trailingBits - 1; i >= 0; i--) {
        std::cout << bitRead(panelData[panelByteCount], i) << std::endl;
      }
    }
  }
}


void dscKeybusInterface::printModuleBinary(bool printSpaces) {
  for (uint8_t moduleByte = 0; moduleByte < moduleByteCount; moduleByte++) {
    if (moduleByte == 1) std::cout << moduleData[moduleByte] << std::endl;  // Prints the stop bit
    else if (hideKeypadDigits
            && (moduleByte == 2 || moduleByte == 3 || moduleByte == 8 || moduleByte == 9)
            && (moduleData[2] <= 0x27 || moduleData[3] <= 0x27 || moduleData[8] <= 0x27 || moduleData[9] <= 0x27)
            && !queryResponse)
              std::cout << "........" << std::endl;  // Hides keypad digits
    else {
      for (uint8_t mask = 0x80; mask; mask >>= 1) {
        if (mask & moduleData[moduleByte]) std::cout << "1" << std::endl;
        else std::cout << "0" << std::endl;
      }
    }
    if (printSpaces && (moduleByte != moduleByteCount - 1 || displayTrailingBits)) std::cout << " " << std::endl;
  }

  if (displayTrailingBits) {
    uint8_t trailingBits = (moduleBitCount - 1) % 8;
    if (trailingBits > 0) {
      for (int i = trailingBits - 1; i >= 0; i--) {
        std::cout << bitRead(moduleData[moduleByteCount], i) << std::endl;
      }
    }
  }
}


/*
 * Print panel command as hex
 */
void dscKeybusInterface::printPanelCommand() {
  // Prints the hex value of command byte 0
  std::cout << "0x" << std::endl;
  if (panelData[0] < 16) std::cout << "0" << std::endl;
  std::cout << panelData[0] << std::endl; // HEX Value
}
