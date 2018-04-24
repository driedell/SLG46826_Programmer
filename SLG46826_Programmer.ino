#include <Wire.h>
#include <stdlib.h>
#include <string.h>

#define NVM_CONFIG 0x02
#define EEPROM_CONFIG 0x03
#define VDD 2

int count = 0;
uint8_t slave_address = 0x00;
bool device_present[16] = {false};

uint8_t data_array[16][16] = {};
 
// Store nvmData in PROGMEM to save on RAM
const char nvmString0[]  PROGMEM = "25050000D04D607CF2E5CF777F153800";
const char nvmString1[]  PROGMEM = "005C02E48B1C3062BC5BC23F80D35C70";
const char nvmString2[]  PROGMEM = "7142C5FD3120C1C5EF023C090000E003";
const char nvmString3[]  PROGMEM = "00000000000000E0FE80EBFE00000000";
const char nvmString4[]  PROGMEM = "80FEE80F000000000000000000000000";
const char nvmString5[]  PROGMEM = "00000000000000000000000000000000";
const char nvmString6[]  PROGMEM = "00303000303030000000308404003030";
const char nvmString7[]  PROGMEM = "30040403000000000000000000000000";
const char nvmString8[]  PROGMEM = "19000048041422300C3C000000000000";
const char nvmString9[]  PROGMEM = "02040000F800000E000EC000002A0000";
const char nvmString10[] PROGMEM = "00F0C8200001005C00F3FF1100FE00FF";
const char nvmString11[] PROGMEM = "0E00B2013100B204010E0030081100FE";
const char nvmString12[] PROGMEM = "01FF1A50260128000000010100000000";
const char nvmString13[] PROGMEM = "00000000000000000000000000000000";
const char nvmString14[] PROGMEM = "00000000000000000000000000000000";
const char nvmString15[] PROGMEM = "000000000000000000000000000000A5";

// Store eepromData in PROGMEM to save on RAM
const char eepromString0[]  PROGMEM = "00000000000000000000000000000000";
const char eepromString1[]  PROGMEM = "00000000000000000000000000000000";
const char eepromString2[]  PROGMEM = "00000011000000000000000011000000";
const char eepromString3[]  PROGMEM = "00000011000000000000000011000000";
const char eepromString4[]  PROGMEM = "00000011000000000000000011000000";
const char eepromString5[]  PROGMEM = "00000011000000000000000011000000";
const char eepromString6[]  PROGMEM = "00000000000000000000000000000000";
const char eepromString7[]  PROGMEM = "00000000000000000000000000000000";
const char eepromString8[]  PROGMEM = "00000000000000000000000000000000";
const char eepromString9[]  PROGMEM = "00000011000000000000000011000000";
const char eepromString10[] PROGMEM = "00000000111100000000111100000000";
const char eepromString11[] PROGMEM = "00000000000011111111000000000000";
const char eepromString12[] PROGMEM = "00000000000000000000000000000000";
const char eepromString13[] PROGMEM = "00000000000000000000000000000000";
const char eepromString14[] PROGMEM = "00000000000000000000000000000000";
const char eepromString15[] PROGMEM = "00000000000000000000000000000000";

const char* const nvmString[16] PROGMEM = {
  nvmString0,
  nvmString1,
  nvmString2,
  nvmString3,
  nvmString4,
  nvmString5,
  nvmString6,
  nvmString7,
  nvmString8,
  nvmString9,
  nvmString10,
  nvmString11,
  nvmString12,
  nvmString13,
  nvmString14,
  nvmString15
};

const char* const eepromString[16] PROGMEM = {
  eepromString0,
  eepromString1,
  eepromString2,
  eepromString3,
  eepromString4,
  eepromString5,
  eepromString6,
  eepromString7,
  eepromString8,
  eepromString9,
  eepromString10,
  eepromString11,
  eepromString12,
  eepromString13,
  eepromString14,
  eepromString15
};


void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(115200);
  pinMode(VDD, OUTPUT);  // This will be the GreenPAK's VDD
  digitalWrite(VDD, HIGH);

  delay(100);
}

void loop() {
  String myString = "";
  int incomingByte = 0;
  slave_address = 0x00;

  Serial.println(F("\nMENU: r = read, e = erase, w = write, p = ping\n"));
  while (1) {
    if (Serial.available() > 0) {
      myString = Serial.readString(); // read the incoming byte:

      if (myString == "r") {
        Serial.println(F("Reading chip!"));
        requestSlaveAddress();
        requestNVMorEeprom();
        readChip();
        Serial.println(F("Done Reading!"));
      }

      if (myString == "e") {
        Serial.println(F("Erasing Chip!"));
        requestSlaveAddress();
        requestNVMorEeprom();
        
        if (eraseChip() == 0) {
          Serial.println(F("Done erasing!"));
        } else {
          Serial.println(F("Erasing did not complete correctly!"));
        }
        
        delay(100);
        ping();
      }

      if (myString == "w") {
        Serial.println(F("Writing Chip!"));
        requestSlaveAddress();
        String NVMorEEPROM = requestNVMorEeprom();
        
        if (eraseChip() == 0) {
          Serial.println(F("Done erasing!"));
        } else {
          Serial.println(F("Erasing did not complete correctly!"));
        }
        
        delay(100);
        ping();
        delay(100);

        if (writeChip(NVMorEEPROM) == 0) {
          Serial.println(F("Done writing!"));
        } else {
          Serial.println(F("Writing did not complete correctly!"));
        }

        delay(100);
        ping();
      }

      if (myString == "p") {
        Serial.println(F("Pinging!"));
        ping();
        Serial.println(F("Done Pinging!"));
      }

      break;
    }
  }
}

void requestSlaveAddress() {
  ping();

  Serial.println(F("Submit slave address, 0-F"));
  while (1) {
    if (Serial.available() > 0) {
      String myString = Serial.readString(); // read the incoming byte:

      slave_address = strtol(&myString[0], NULL, 16);

      //Check for a valid slave address
      if (device_present[slave_address] == false)
      {
        Serial.println(F("You entered an incorrect slave address, please try again."));
        Serial.println(F("Submit slave address, 0-F"));
        continue;
      }
      else {
        Serial.print(F("Slave address is: 0x"));
        PrintHex8(slave_address);
        Serial.println();
        return;
      }
    }
  }
}

String requestNVMorEeprom() {
  Serial.println(F("MENU: n = NVM, e = EEPROM"));
  while (1)
  {
    if(Serial.available() > 0) {
      String myString = Serial.readString(); // read the incoming byte:

      if (myString == "n")
      {
        Serial.println(F("NVM Selected!"));
        myString = "NVM";
      }
      else if (myString == "e")
      {
        Serial.println(F("EEPROM Selected!"));
        myString = "EEPROM";
      }
      else {
        Serial.println(F("Invalid command. You did not enter \"n\" or \"e\"."));
        continue;
      }

      Serial.println();
      return myString;
    }
  }
}

void PrintHex8(uint8_t data) {
  if (data < 0x10) {
    Serial.print("0");
  }
  Serial.print(data, HEX);
  // Serial.print(" ");
}

int readChip() {
  digitalWrite(VDD, HIGH);
  delay(500);

  Serial.print(F("Reading chip 0x"));
  PrintHex8(slave_address);
  Serial.println();

  int control_code = slave_address << 3;

  int pageData[16];
  uint8_t i = 0;

  for (int j = 0; j < 16; j++) {
    Wire.beginTransmission(control_code);
    Wire.write(j << 4);
    Wire.endTransmission(false);
    delay(20);
    Wire.requestFrom(control_code, 16);

    while (Wire.available()) {
      pageData[i] = Wire.read();
      PrintHex8(pageData[i]);
      i++;
    }
    Serial.println();
  }
}

int eraseChip() {
  digitalWrite(VDD, HIGH);
  delay(500);

  Serial.print(F("Erasing chip 0x"));
  PrintHex8(slave_address);
  Serial.println();

  for (uint8_t i = 0; i < 16; i++) {
    if (erasePage(i) == -1)
    {
      return -1;
    }
    delay(100);
  }

  Serial.println(F("Power Cycling!"));
  digitalWrite(VDD, LOW);
  delay(500);
  digitalWrite(VDD, HIGH);
  Serial.println(F("Done Power Cycling!"));
}

int erasePage(uint8_t pageNumber) {
  Serial.print("Erasing page: 0x");
  PrintHex8(pageNumber);
  Serial.print(" ");

  // Serial.print(F("Control Code: 0x"));
  // PrintHex8(control_code);
  // Serial.println();

  int control_code = slave_address << 3;
  Wire.beginTransmission(control_code);

  Wire.write(0xE3);
  Wire.write(0x80 | pageNumber);

  if (Wire.endTransmission() == 0) {
    Serial.println(F("ack"));
    return 0;
  } 
  else { 
    Serial.println(F("nack")); 
    return -1;
  }
}

int 1(String NVMorEEPROM) {
  digitalWrite(VDD, HIGH);
  delay(500);

  // Set the control code to 0x00 since the chip has just been erased
  int control_code = 0x00;
  bool NVM_selected = false;
  bool EEPROM_selected = false;

  if (NVMorEEPROM == "NVM")
  {
    control_code |= NVM_CONFIG;
    NVM_selected = true;
  }
  else if (NVMorEEPROM == "EEPROM")
  {
    control_code |= EEPROM_CONFIG;
    EEPROM_selected = true;
  }

  Serial.print(F("Control Code: 0x"));
  PrintHex8(control_code);
  Serial.println();
  
  // Assign each byte to data_array[][] array;
  // http://www.gammon.com.au/progmem

  // Serial.println(F("New NVM data:"));
  for (size_t i = 0; i < 16; i++)
  {
    // Pull current page NVM from PROGMEM and place into buffer
    char buffer [64];
    if (NVM_selected)
    {
      char * ptr = (char *) pgm_read_word (&nvmString[i]);
      strcpy_P (buffer, ptr);
    }
    else if (EEPROM_selected)
    {
      char * ptr = (char *) pgm_read_word (&eepromString[i]);
      strcpy_P (buffer, ptr);
    }

    // Serial.println(buffer);

    // Store each byte into data_array[][] array
    for (size_t j = 0; j < 16; j++)
    {
      String temp = (String)buffer[2 * j] + (String)buffer[(2 * j) + 1];
      long myNum = strtol(&temp[0], NULL, 16);
      data_array[i][j] = (uint8_t) myNum;
    }
  }
  Serial.println();

  if (NVM_selected)
  {
    // Request new slave address
    Serial.println(F("New slave address?"));
    while (1) {
      if (Serial.available() > 0) {
        String myString = Serial.readString(); // read the incoming byte:

        slave_address = strtol(&myString[0], NULL, 16);
        Serial.print(F("New slave address is: 0x"));
        PrintHex8(slave_address);
        Serial.println();
        break;
      }
    }

    data_array[0xC][0xA] = slave_address;
    Serial.print(F("slave_addr: "));
    PrintHex8(data_array[0xC][0xA]);
    Serial.println();

    Serial.print(F("Control Code: 0x"));
    PrintHex8(control_code);
    Serial.println();
  }


  // Write each byte of data_array[][] array to the chip
  for (int i = 0; i < 16; i++) {
    Wire.beginTransmission(control_code);
    Wire.write(i << 4);

    for (int j = 0; j < 16; j++) {
      Wire.write(data_array[i][j]);
      PrintHex8(data_array[i][j]);
    }
    
    if (Wire.endTransmission(true) == 0) {
      Serial.print(F(" "));
      PrintHex8(i);
      Serial.print(F(" ack "));
    } else {
      PrintHex8(i);
      Serial.print(F(" nack "));
      Serial.println(F("Oh No! Something went wrong while programming!"));
      return -1;
    }

    // ACK polling to check if the writing is complete
    int nack_count = 0;
    while (1) {
      // Slave address is still 0x00
      Wire.beginTransmission(0x00);
      if (Wire.endTransmission() == 0) {
        break;
      }
      if (nack_count >= 1000)
      {
        Serial.println(F("Geez! Something went wrong while programming!"));
        return -1;
      }
      nack_count++;
      delay(1);
    }
    Serial.println(F("ready"));
    delay(100);
  }

  Serial.println(F("Power Cycling!"));
  digitalWrite(VDD, LOW);
  delay(500);
  digitalWrite(VDD, HIGH);
  Serial.println(F("Done Power Cycling!"));

  return 0;
}

void ping() {
  for (int i = 0; i < 16; i++) {
    Wire.beginTransmission(i << 3);

    if (Wire.endTransmission() == 0) {
      Serial.print(F("device "));
      Serial.print(i);
      Serial.println(F(" is present"));
      device_present[i] = true;
    } else {
      device_present[i] = false; 
    }
  }
}