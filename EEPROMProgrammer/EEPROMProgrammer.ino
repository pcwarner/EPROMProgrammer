// Used to set clock prescaler
#include <avr/power.h>

#define UNKNOWN_DIRECTION 0xff
#define MAX_COMMAND_LINE_LENGTH 20
#define MAX_GIVE_UP 2000
#define ADDRESS_ARRAY_SIZE 4
#define DATA_ARRAY_SIZE 2
#define EEPROM_ARRAY_SIZE 7
#define MAX_EEPROM_NAME_SIZE 10
#define MAX_NUMBER_OF_PARTS 4
#define MAX_SIZE_OF_A_PART 11
#define END_OF_LINE 10
#define MAX_NUMBER_OF_ADDRESS_BITS 19
#define MAX_NUMBER_OF_DATA_BITS 8
#define MAX_NUMBER_OF_CTRL_PINS 4
#define PWR_ADDRESS_LINE_11 111


#define EEPROM_UNKNOWN 0
#define EEPROM_28C16 1
#define EEPROM_28C64 2
#define EEPROM_28C256 3
#define EEPROM_28C512 4
#define EEPROM_SST39SF010 5
#define EEPROM_SST39SF020 6
#define EEPROM_SST39SF040 7

const char EEPROM_NAMES[EEPROM_ARRAY_SIZE][MAX_EEPROM_NAME_SIZE + 1] = { "28C16", "28C64", "28C256", "28C512", "SST39SF010", "SST39SF020", "SST39SF040" };
const byte EEPROM_MAX_ADDRESS_LINE[EEPROM_ARRAY_SIZE] = { 10, 12, 14, 15, 16, 17, 18 };
const long EEPROM_SIZES[EEPROM_ARRAY_SIZE] = { 2048, 8192, 32768, 65536, 131072, 264144, 524288 };

const byte EEPROM_ADDRESS_PINS[EEPROM_ARRAY_SIZE][MAX_NUMBER_OF_ADDRESS_BITS] = {
//  A0    A1  A2   A3   A4   A5   A6   A7   A8   A9  A10  A11  A12  A13  A14  A15  A16  A17  A18
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 255, 255, 255, 255, 255, 255, 255, 255 }, // 28C16
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 102,  12, 255, 255, 255, 255, 255, 255 }, // 28C64
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 102,  12, 104,  15, 255, 255, 255, 255 }, // 28C256
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 102,  12, 104, 103,  15, 255, 255, 255 }, // 28C512
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 102,  12, 104, 103,  15,  13, 255, 255 }, // SST39SF010
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 102,  12, 104, 103,  15,  13,  11, 255 }, // SST39SF020
  {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 102,  12, 104, 103,  15,  13,  11,  14 }  // SST39SF040
};

const byte EEPROM_CTRL_PINS[EEPROM_ARRAY_SIZE][MAX_NUMBER_OF_CTRL_PINS] = {
//  OUTPUT    CHIP   WRITE   PWR 
  {     A3,     A4,      2,    4                 }, // 28C16
  {     A3,     A4,      3,  PWR_ADDRESS_LINE_11 }, // 28C64
  {     A3,     A4,      3,  PWR_ADDRESS_LINE_11 }, // 28C256
  {     A3,     A4,     A5,  255                 }, // 28C512
  {     A3,     A4,     A5,  255                 }, // SST39SF010
  {     A3,     A4,     A5,  255                 }, // SST39SF020
  {     A3,     A4,     A5,  255                 }  // SST39SF040
};

// Define pins used for the shift register
const int SHIFT_ENABLE = A0;  // Move buffer to register pin
const int SHIFT_CLK = A1;     // Shift register clock pin
const int SHIFT_DATA = A2;    // Shift register data pin

// Define pins used to control access to EEPROM
byte outputEnable = 0;
byte chipEnable = 0;
byte writeEnable = 0;
byte powerEnable = 0;

const int DATA_PIN_0 = 5;   // Data bit 0 pin
const int DATA_PIN_1 = 6;   // Data bit 1 pin
const int DATA_PIN_2 = 7;   // Data bit 2 pin
const int DATA_PIN_3 = 8;   // Data bit 3 pin
const int DATA_PIN_4 = 9;   // Data bit 4 pin
const int DATA_PIN_5 = 10;  // Data bit 5 pin
const int DATA_PIN_6 = 11;  // Data bit 6 pin
const int DATA_PIN_7 = 12;  // Data bit 7 pin

const int DATA_PIN_ARRAY[MAX_NUMBER_OF_DATA_BITS] = { DATA_PIN_0, DATA_PIN_1, DATA_PIN_2, DATA_PIN_3, DATA_PIN_4, DATA_PIN_5, DATA_PIN_6, DATA_PIN_7 };
const int DATA_MASK_ARRAY[MAX_NUMBER_OF_DATA_BITS] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000 };
const long ADDRESS_MASK_ARRAY[MAX_NUMBER_OF_ADDRESS_BITS] = { 
  0b0000000000000000001,
  0b0000000000000000010,
  0b0000000000000000100,
  0b0000000000000001000,
  0b0000000000000010000,
  0b0000000000000100000,
  0b0000000000001000000,
  0b0000000000010000000,
  0b0000000000100000000,
  0b0000000001000000000,
  0b0000000010000000000,
  0b0000000100000000000,
  0b0000001000000000000,
  0b0000010000000000000,
  0b0000100000000000000,
  0b0001000000000000000,
  0b0010000000000000000,
  0b0100000000000000000,
  0b1000000000000000000
};

const int ENABLE = LOW;    // 0v is enable - These are the reverse to what you would expect
const int DISABLE = HIGH;  // 5v is disable

#define READ_COMMAND 'R'
#define WRITE_COMMAND 'W'
#define UPDATE_COMMAND 'U'
#define SET_COMMAND 'S'
#define GET_COMMAND 'G'
#define PROTECT_COMMAND 'P'
#define ERASE_COMMAND 'E'
#define TEST_COMMAND 'T'
#define HELP_COMMAND 'H'

#define MAIN_METHOD 0
#define READ_METHOD 1
#define WRITE_METHOD 2
#define UPDATE_METHOD 3
#define SET_METHOD 4
#define GET_METHOD 5
#define PROTECT_METHOD 6
#define ERASE_METHOD 7
#define TEST_METHOD 8
#define HELP_METHOD 9
#define OTHER_METHOD 10

const char COMMAND_NAMES[11][8] = {"MAIN", "READ", "WRITE", "UPDATE", "SET", "GET", "PROTECT", "ERASE", "TEST", "HELP", "OTHER" };

// Global variables
int dataDirection = 9999;
int eepromType = EEPROM_UNKNOWN;
int maxAddressLine = 0;

int split(char input[], char splitChar, char output[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  int j = 0;
  int k = -1;
  int l = strlen(input);

  for (int i = 0; i < l; i++) {
    char c = input[i];
    if (c == splitChar) {
      output[j][k + 1] = 0;
      j++;
      if (j > MAX_NUMBER_OF_PARTS - 1) {
        return -1;
      }
      k = -1;
    } else {
      k++;
      if (k > MAX_SIZE_OF_A_PART - 1) {
        return -1;
      }
      output[j][k] = c;
      output[j][k + 1] = 0;
    }
  }
  return j + 1;
}

int char2HEX(char digit) {
  int value = int(toupper(digit));
  if (value >= 48 && value <= 57) {  // Numbers
    return value - (int)'0';
  }
  if (value >= 65 && value <= 70) {  // Letters
    return 10 + value - (int)'A';
  }
  return -1;
}


long parseHEX(char value[]) {
  long result = 0;
  for (int i = 0; i < strlen(value); i++) {
    result *= 0x10;
    int c = char2HEX(value[i]);
    if (c < 0) {
      return -1;
    }
    result += c;
  }
  return result;
}

// Set the address we need to read from or write to
void setAddress(long address) {
  address = address & 0x0007ffff;
  long newAddress = 0;
  for (int i=0; i<MAX_NUMBER_OF_ADDRESS_BITS; i++) {
    long value = address & ADDRESS_MASK_ARRAY[i];
    int pin = EEPROM_ADDRESS_PINS[eepromType-1][i];
    if (pin != 255) {
      if (pin > 100) {
        pin = pin - 100;
        if (value == 0) {
          digitalWrite(pin, LOW);
        } else {
          digitalWrite(pin, HIGH);
        }
      } else {
        if (value != 0) {
          newAddress = newAddress | ADDRESS_MASK_ARRAY[pin];
        }
      }
    } 
  }

  if (powerEnable == PWR_ADDRESS_LINE_11) {
    newAddress = newAddress | ADDRESS_MASK_ARRAY[11]; // Keeps on power for 28C256 AND 28C64.
  }

  newAddress = newAddress & 0x0000ffff;
  byte low = newAddress % 0x0100;
  byte high = (newAddress - low) / 0x0100;
  digitalWrite(SHIFT_ENABLE, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, high);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, low);
  digitalWrite(SHIFT_ENABLE, HIGH);
  return;
}

// Set the direction of the data pins
void setDataDirection(int direction) {
  if (dataDirection != direction) {
    for (int i = 0; i < MAX_NUMBER_OF_DATA_BITS; i++) {
      pinMode(DATA_PIN_ARRAY[i], direction);
    }
    dataDirection = direction;
  }
  return;
}

void setDataByte(int value) {
  value = value & 0x000000ff;
  for (int i = 0; i < MAX_NUMBER_OF_DATA_BITS; i++) {
    if ((value & DATA_MASK_ARRAY[i]) == 0) {
      digitalWrite(DATA_PIN_ARRAY[i], LOW);
    } else {
      digitalWrite(DATA_PIN_ARRAY[i], HIGH);
    }
  }
  return;
}

int getDataByte() {
  int value = 0;
  for (int i = 0; i < MAX_NUMBER_OF_DATA_BITS; i++) {
    if (digitalRead(DATA_PIN_ARRAY[i]) == HIGH) {
      value += DATA_MASK_ARRAY[i];
    }
  }
  return value;
}

int readByte(long address) {
  setAddress(address);                // Set the addresss we are going to read from
  setDataDirection(INPUT);            // Set the data to input
  digitalWrite(writeEnable, DISABLE); // Disable write
  digitalWrite(outputEnable, ENABLE); // Enable output from the chip
  digitalWrite(chipEnable, ENABLE);   // Enable the chip
  delay(1);                           // Wait for the chip to get the data
  int value = getDataByte();          // Read the data from the chip
  digitalWrite(chipEnable, DISABLE);  // Disable the chip
  return value;
}

void writeByte(long address, int value, bool writeOnly) {
  if (!writeOnly) {
    setDataDirection(OUTPUT);
    if (eepromType == EEPROM_SST39SF010 || eepromType ==  EEPROM_SST39SF020 || eepromType ==  EEPROM_SST39SF040) {
      writeByte(0x5555, 0xaa, true);
      writeByte(0x2aaa, 0x55, true);
      writeByte(0x5555, 0xa0, true);
    }
  }
  setAddress(address);                  // Set the address we are going to program
  setDataDirection(OUTPUT);             // Set the data to output
  setDataByte(value);                   // Set the value of the byte to program
  digitalWrite(outputEnable, DISABLE);  // Set Output Enable to high
  digitalWrite(chipEnable, ENABLE);     // Make sure Chip Enable is low
  digitalWrite(writeEnable, ENABLE);    // Set Write Enable to high
  delayMicroseconds(10);                // Wait for the chip to notice the data
  digitalWrite(writeEnable, DISABLE);   // Set Output Enable to high
  delay(10);                            // Wait for new value to be saved
  digitalWrite(chipEnable, DISABLE);    // Disable the chip
  return;
}

void displayError(int errorFrom, int errorNumber) {
  if (errorFrom > HELP_METHOD) {
    errorFrom = OTHER_METHOD;
  }
  Serial.print(F("E,"));
  Serial.print(COMMAND_NAMES[errorFrom]);
  Serial.print(F(": "));
  
  switch(errorNumber) {
    case 1:
      Serial.println(F("EPROM type not set."));
      break;
    case 2:
      Serial.println(F("The syntax of the command is incorrect"));
      break;
    case 3:
      Serial.println(F("Address is not a valid HEX number."));
      break;
    case 4:
      Serial.println(F("Data is not a valid HEX number."));
      break;
    case 5:
      Serial.println(F("Data written to EEPROM did not match that read."));
      break;
    case 6:
      Serial.println(F("EEPROM name is invalid."));
      break;
    case 7:
      Serial.println(F("EEPROM type is not valid."));
      break;
    case 8:
      Serial.println(F("Failed to find line termination."));
      break;
    case 9:
      Serial.println(F("Data in buffer present when not expected."));
      break;
    case 10:
      Serial.println(F("Unknown command or incorrect syntax."));
      break;
    case 11:
      Serial.println(F("Option is not available for this EEPROM."));
      break;
    case 12:
      Serial.println(F("Block Address is not a valid HEX number."));
      break;
    case 13:
      Serial.println(F("Address is greater than EEPROM size."));
      break;
    default:
      Serial.println(F("Unknown error."));
      break;
  }  
  return;
}

void setup() {
  // Set up the serial link
  clock_prescale_set(clock_div_2); // Uncomment when running at 32MHz
  Serial.begin(115200);
  Serial.setTimeout(20);

  // Set up the pins
  pinMode(SHIFT_ENABLE, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);

  // Set up the data control pins
  dataDirection = UNKNOWN_DIRECTION;
  setDataDirection(INPUT);
  eepromType = EEPROM_UNKNOWN;
  setAddress(0);
  Serial.println("G");
  delay(500);

  return;
}

void read(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  // Has EEPROM type been set
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(READ_METHOD, 1);
    return;
  }

  if (numberOfParts != 2) {
    displayError(READ_METHOD, 2);
    return;
  }

  // Convert address (in the second part) to a long
  long address = parseHEX(commandLineParts[1]);
  if (address < 0) {
    displayError(READ_METHOD, 3);
    return;
  }

  if (address > EEPROM_SIZES[eepromType-1]-1) {
    displayError(READ_METHOD, 13);
    return;
  }

  int data = readByte(address);
  Serial.print("G,");
  Serial.println(data, HEX);
  return;
}

void write(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {

  // Have we set the EEPROM type
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(WRITE_METHOD, 1);
    return;
  }

  if (numberOfParts != 3) {
    displayError(WRITE_METHOD, 2);
    return;
  }

  // Convert address to a long
  long address = parseHEX(commandLineParts[1]);
  if (address < 0) {
    displayError(WRITE_METHOD, 3);
    return;
  }

  if (address > EEPROM_SIZES[eepromType-1]-1) {
    displayError(WRITE_METHOD, 13);
    return;
  }

  // Convert data to a int
  int data = parseHEX(commandLineParts[2]);
  if (data < 0 || data > 256) {
    displayError(WRITE_METHOD, 4);
    return;
  }

  writeByte(address, data, false);
  Serial.println("G");
  return;
}

void update(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {

  // Have we set the EEPROM type
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(UPDATE_METHOD, 1);
    return;
  }

  if (numberOfParts != 3) {
    displayError(UPDATE_METHOD, 2);
    return;
  }

  // Convert address to a long
  long address = parseHEX(commandLineParts[1]);
  if (address < 0) {
    displayError(UPDATE_METHOD, 3);
    return;
  }

  if (address > EEPROM_SIZES[eepromType-1]-1) {
    displayError(WRITE_METHOD, 13);
    return;
  }

  // Convert data to a int
  int data = parseHEX(commandLineParts[2]);
  if (data < 0) {
    displayError(UPDATE_METHOD, 4);
    return;
  }

  // Write the data, then read the data and compare the two
  writeByte(address, data, false);
  int newData = readByte(address);
  if (data != newData) {
    displayError(UPDATE_METHOD, 5);
    return;
  }
  Serial.print("G,");
  Serial.println(newData, HEX);
  return;
}

void set(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  // There should be 2 parts
  if (numberOfParts != 2) {
    displayError(SET_METHOD, 2);
    return;
  }

  // The EEPROM name must be between 1 and 8 characters
  int l = strlen(commandLineParts[1]);
  if (l < 1 || l > 10) {
    displayError(SET_METHOD, 6);
    return;
  }

  // Upper case the characters
  for (int i = 0; i < l; i++) {
    commandLineParts[1][i] = toupper(commandLineParts[1][i]);
  }

  // Find the EEPROM from list
  eepromType = EEPROM_UNKNOWN;
  for (int i = 0; i < EEPROM_ARRAY_SIZE; i++) {
    if (strcmp(commandLineParts[1], EEPROM_NAMES[i]) == 0) {
      eepromType = i + 1;
      break;
    }
  }
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(SET_METHOD, 7);
    return;
  }

// Set up the control lines for the EEPROM
  outputEnable = EEPROM_CTRL_PINS[eepromType-1][0];
  chipEnable = EEPROM_CTRL_PINS[eepromType-1][1];
  writeEnable = EEPROM_CTRL_PINS[eepromType-1][2];
  powerEnable = EEPROM_CTRL_PINS[eepromType-1][3];

// Set the control pins
  pinMode(outputEnable, OUTPUT);
  pinMode(chipEnable, OUTPUT);
  pinMode(writeEnable, OUTPUT);
  digitalWrite(outputEnable, DISABLE);
  digitalWrite(chipEnable, DISABLE);
  digitalWrite(writeEnable, DISABLE);
  if (powerEnable != 255) {
    if (powerEnable > 100) {
      setAddress(0); 
    } else {
      pinMode(powerEnable, OUTPUT);
      digitalWrite(powerEnable, HIGH);
    }
    delay(1000);
  }

// Update the address pins, that are not on the 74HC595s
  for(int i=0; i < MAX_NUMBER_OF_ADDRESS_BITS; i++) {
    int pinNumber = EEPROM_ADDRESS_PINS[eepromType-1][i];
    if (pinNumber != 255 && pinNumber > 100) {
      pinNumber = pinNumber - 100;
      pinMode(pinNumber, OUTPUT);
      digitalWrite(pinNumber, LOW);
    }
  }

  Serial.println("G");
  return;
}

void get(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  // There should be only 1 part
  if (numberOfParts != 1) {
    displayError(GET_METHOD, 2);
    return;
  }

  // Are we using the UNKNOWN type?
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(GET_METHOD, 1);
    return;
  }

  // Display the EEPROM name
  Serial.print("G,");
  Serial.println(EEPROM_NAMES[eepromType-1]);
  return;
}

void protect(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  // There should be 2 parts
  if (numberOfParts != 2) {
    displayError(PROTECT_METHOD, 2);
    return;
  }

  // Are we using the UNKNOWN type?
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(PROTECT_METHOD, 1);
    return;
  }

  // Convert value to a int
  int value = parseHEX(commandLineParts[1]);
  if (value != 0 && value != 1) {
    displayError(PROTECT_METHOD, 4);
    return;
  }

// Are we unprotecting the EEPROM?
  bool error = false;
  if (value == 0) {
    switch (eepromType) {
      case EEPROM_28C256:
      case EEPROM_28C512:
        setDataDirection(OUTPUT);
        writeByte(0x5555, 0xaa, false);
        writeByte(0x2aaa, 0x55, false);
        writeByte(0x5555, 0x80, false);
        writeByte(0x5555, 0xaa, false);
        writeByte(0x2aaa, 0x55, false);
        writeByte(0x5555, 0x20, false);
      default:
        displayError(PROTECT_METHOD, 11);
        return;
    }

  // Are we protecting the EEPROM?
  } else {
    switch (eepromType) {
      case EEPROM_28C256:
      case EEPROM_28C512:
        setDataDirection(OUTPUT);
        writeByte(0x5555, 0xaa, true);
        writeByte(0x2aaa, 0x55, true);
        writeByte(0x5555, 0xa0, true);
      default:
        displayError(PROTECT_METHOD, 11);
        return;
    }
  }

  Serial.println("G");
  return;
}

void erase(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  // There should be 2 parts
  if (numberOfParts != 2) {
    displayError(ERASE_METHOD, 2);
    return;
  }

  // Are we using the UNKNOWN type?
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(ERASE_METHOD, 1);
    return;
  }
  if (eepromType != EEPROM_SST39SF010 && eepromType !=  EEPROM_SST39SF020 && eepromType !=  EEPROM_SST39SF040) {
    displayError(ERASE_METHOD, 11);
    return;
  }
  
  // Are we erasing the whole chip or a 4K sector?
  if (strcasecmp(commandLineParts[1], "ALL") == 0 && strlen(commandLineParts[1]) == 3) {
    setDataDirection(OUTPUT);
    writeByte(0x5555, 0xaa, true);
    writeByte(0x2aaa, 0x55, true);
    writeByte(0x5555, 0x80, true);
    writeByte(0x5555, 0xaa, true);
    writeByte(0x2aaa, 0x55, true);
    writeByte(0x5555, 0x10, true);
  } else {
    // Extract address
    long address = parseHEX(commandLineParts[1]);
    if (address < 0) {
      displayError(ERASE_METHOD, 12);
      return;
    }
    address = address & 0b1111111000000000000; // Use 4K sectors
    if (address > EEPROM_SIZES[eepromType-1]-1) {
      displayError(WRITE_METHOD, 13);
      return;
    }
    setDataDirection(OUTPUT);
    writeByte(0x5555, 0xaa, true);
    writeByte(0x2aaa, 0x55, true);
    writeByte(0x5555, 0x80, true);
    writeByte(0x5555, 0xaa, true);
    writeByte(0x2aaa, 0x55, true);
    writeByte(address, 0x30, true);
  }

  Serial.println("G");
  return;
}

void test(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  if (numberOfParts != 2) {
    displayError(TEST_METHOD, 2);
    return;
  }

 // Have we set the EEPROM type
  if (eepromType == EEPROM_UNKNOWN) {
    displayError(TEST_METHOD, 1);
    return;
  }

  // Convert option to a int
  int option = parseHEX(commandLineParts[1]);
  if (option < 0) {
    displayError(TEST_METHOD, 12);
    return;
  }

  switch(option) {
    case 2:
      {
        setDataDirection(OUTPUT);
        int value = 1;
        setDataByte(value);
        delay(500);
        for (int i=0; i<7; i++) {
          value = value * 2;
          setDataByte(value);
          delay(500);
        }
        for (int i=0; i<7; i++) {
          value = value / 2;
          setDataByte(value);
          delay(500);
        }
        setDataByte(0);
      }
      break;
    case 3:
      {
        long value = 1;
        setAddress(value);
        delay(500);
        for (int i=0; i<EEPROM_MAX_ADDRESS_LINE[eepromType-1]; i++) {
          value = value * 2;
          setAddress(value);
          delay(500);
        }
        for (int i=0; i<EEPROM_MAX_ADDRESS_LINE[eepromType-1]; i++) {
          value = value / 2;
          setAddress(value);
          delay(500);
        }
        setAddress(0);
      }
      break;
    case 4:
      for (int i=0; i<10; i++) {
        digitalWrite(outputEnable, ENABLE);
        delay(250);
        digitalWrite(outputEnable, DISABLE);
        delay(250);
      }
      break;
    case 5:
      for (int i=0; i<10; i++) {
        digitalWrite(chipEnable, ENABLE);
        delay(250);
        digitalWrite(chipEnable, DISABLE);
        delay(250);
      }
      break;
    case 6:
      for (int i=0; i<10; i++) {
        digitalWrite(writeEnable, ENABLE);
        delay(250);
        digitalWrite(writeEnable, DISABLE);
        delay(250);
      }
      break;
    case 7:
      if (powerEnable == 255) {
        displayError(TEST_METHOD, 11);
        return;
      }
      if (powerEnable == PWR_ADDRESS_LINE_11) {
        for (int i=0; i<10; i++) {
          powerEnable = 0;
          setAddress(0);
          delay(250);
          powerEnable = PWR_ADDRESS_LINE_11;
          setAddress(0);
          delay(250);
        }
      } else {
        for (int i=0; i<10; i++) {
          digitalWrite(powerEnable, ENABLE);
          delay(250);
          digitalWrite(powerEnable, DISABLE);
          delay(250);
        }
      }
      break;
    default:
      displayError(TEST_METHOD, 11);
      return;
  }
  return;
}

void help(int numberOfParts, char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1]) {
  Serial.println(F("The commands available are:"));
  Serial.println("");
  Serial.println(F("R - Read    - R,<address>         - returns G,<data>       - Reads data from EEPROM."));
  Serial.println(F("W - Write   - W,<address>,<data>  - returns G              - Writes data to EEPROM."));
  Serial.println(F("U - Update  - U,<address>,<data>  - returns G,<data>       - Writes data to EEPROM and then reads the data."));
  Serial.println(F("S - Set     - S,<EPROM type>      - returns G              - Sets the type of EEPROM to be used."));
  Serial.println(F("G - Get     - G                   - returns G,<EPROM type> - Gets the type of EEPROM being used."));
  Serial.println(F("P - Protect - P,<1 or 0>          - returns G              - Set or unsets write protection on the EEPROM."));
  Serial.println(F("E - Erase   - P,<ALL or address>  - returns G              - Erase the flash. \"ALL\" for whole chip or address of the sector."));
  Serial.println(F("B - Batch   - B,<option>{,<data>} - returns G{,<data>}     - Performs batch mode functions."));
  Serial.println(F("T - Test    - T,<option>          - returns G              - Performs hardware tests."));
  Serial.println(F("H - Help    - H                   - returns <Help Message> - Display this help message."));
  Serial.println("");
  Serial.println(F("All errors return E,<Error message>."));
  Serial.println("");
  Serial.println(F("The type of UVEPROM must be set before any read or write will work."));
  Serial.print(F("Supported EEPROMs are: "));
  for(int i=0;i<EEPROM_ARRAY_SIZE-1;i++) {
    if (i != 0) {
          Serial.print(", ");
    }
    Serial.print(EEPROM_NAMES[i]);
  }
  Serial.print(" and ");
  Serial.println(EEPROM_NAMES[EEPROM_ARRAY_SIZE-1]);
  return;
}

// Main loop
void loop() {
  if (Serial.available()) {
    int c = 0;
    char commandLine[MAX_COMMAND_LINE_LENGTH + 1];
    int pos = 0;
    int giveupCount = 0;
    char bytesRead[MAX_COMMAND_LINE_LENGTH + 1];
    do {
      int available = Serial.available();
      if (available > MAX_COMMAND_LINE_LENGTH) {
        available = MAX_COMMAND_LINE_LENGTH;
      }
      if (available > 0) {
        int numRead = Serial.readBytes(bytesRead, available);
        if (numRead > 0) {
          bytesRead[numRead] = 0;
          for (int j = 0; j < numRead; j++) {
            c = bytesRead[j];
            if (c != END_OF_LINE && pos < MAX_COMMAND_LINE_LENGTH - 1) {
              commandLine[pos] = (char)c;
              commandLine[pos + 1] = 0;
              pos++;
            } else {
              break;
            }
          }
        }
      } else {
        giveupCount++;
        delay(1);
      }
    } while (c != END_OF_LINE && pos < MAX_COMMAND_LINE_LENGTH && giveupCount < MAX_GIVE_UP);

    if (giveupCount >= MAX_GIVE_UP) {
      displayError(MAIN_METHOD, 8);
      return;
    }

    // Clear anything left in buffer
    bool bufferNotEmpty = false;
    while (Serial.available()) {
      Serial.read();
      bufferNotEmpty = true;
    }
    if (bufferNotEmpty) {
      displayError(MAIN_METHOD, 9);
      return;
    }

    // Split the command line
    char commandLineParts[MAX_NUMBER_OF_PARTS][MAX_SIZE_OF_A_PART + 1];
    int numberOfParts = split(commandLine, ',', commandLineParts);
    if (numberOfParts < 1 || strlen(commandLineParts[0]) < 1) {
      displayError(MAIN_METHOD, 2);
      return;
    }

    // Which command do we need to run?
    char command = toupper(commandLineParts[0][0]);
    switch (command) {
      case READ_COMMAND:
        read(numberOfParts, commandLineParts);
        break;
      case WRITE_COMMAND:
        write(numberOfParts, commandLineParts);
        break;
      case UPDATE_COMMAND:
        update(numberOfParts, commandLineParts);
        break;
      case SET_COMMAND:
        set(numberOfParts, commandLineParts);
        break;
      case GET_COMMAND:
        get(numberOfParts, commandLineParts);
        break;
      case PROTECT_COMMAND:
        protect(numberOfParts, commandLineParts);
        break;
      case ERASE_COMMAND:
        erase(numberOfParts, commandLineParts);
        break;
      case TEST_COMMAND:
        test(numberOfParts, commandLineParts);
        break;
      case HELP_COMMAND:
        help(numberOfParts, commandLineParts);
        break;
      default:
        displayError(MAIN_METHOD, 10);
        break;
    }
  }
  return;
}