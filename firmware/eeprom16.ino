// For this project I use arudino nano every that can go up to 20MHz
// In this case each clock cycle should be around 50ns more than enough for all computations
// (considering that most oprations such as digitalWrite require well above 1 cycle!)
// So I'll comment delays but be careful with other boards!

// Data bus (D0–D7)
const int dataPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};

// Shift register pins (74HC164)
const int dataPin = 2;
const int clockPin = 3;

// EEPROM control pins
const int oePin = 13;
const int wePin = 12;
const int clearPin = A0;

// --------------------------------------------------
// Shift address into the 74HC164
// --------------------------------------------------
void setAddress(unsigned int address) {
  for (int i = 10; i >= 0; i--) {
    // Minimum pulse width is 20ns
    // This way we are quite safe with 1us
    // Data is shifted on the rising edge of the clock pulse

    // Data cycle
    digitalWrite(clockPin, LOW);
    // delayMicroseconds(1);
    // Insert current 1/0
    digitalWrite(dataPin, (address >> i) & 1);
    // Shift data
    digitalWrite(clockPin, HIGH);
    // delayMicroseconds(1);
  }
}

void clearShitRegister() {
  // Average time required is 50ns to clear
  digitalWrite(clearPin, LOW);
  // delayMicroseconds(1);
  digitalWrite(clearPin, HIGH);
}

// --------------------------------------------------
// Write one byte to EEPROM 
// --------------------------------------------------
void writeEEPROM(unsigned int address, byte data) {
  // Disable EEPROM
  digitalWrite(wePin, HIGH);
  digitalWrite(oePin, HIGH);

  // Clear shift register
  clearShitRegister();

  // Set address
  setAddress(address);

  // Put data on bus
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], OUTPUT);
    digitalWrite(dataPins[i], (data >> i) & 1);
  }

  // Write pulse
  // Between 100-1000ns
  digitalWrite(wePin, LOW);
  // delayMicroseconds(1);
  digitalWrite(wePin, HIGH);

  // Wait for internal write cycle
  // Datasheet says max 1ms
  delay(2);
}

// --------------------------------------------------
// Read one byte from EEPROM 
// --------------------------------------------------
byte readEEPROM(unsigned int address) {
  digitalWrite(wePin, HIGH);
  digitalWrite(oePin, HIGH);

  // Clear shift register before loading new address
  clearShitRegister();

  // Set address twice
  setAddress(address);

  // Data bus as input
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], INPUT);
  }

  // Enable EEPROM output
  // Should be around 10ns
  digitalWrite(oePin, LOW);
  // delayMicroseconds(1);

  byte data = 0;
  for (int i = 0; i < 8; i++) {
    data |= (digitalRead(dataPins[i]) << i);
  }

  digitalWrite(oePin, HIGH);

  return data;
}

// --------------------------------------------------
// Print full EEPROM contents
// --------------------------------------------------
void printContents() {
  for (unsigned int base = 0; base < 2048; base += 16) {
    byte data[16];

    for (int offset = 0; offset < 16; offset++) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[100];
    sprintf(buf,
            "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base,
            data[0], data[1], data[2], data[3],
            data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11],
            data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(oePin, OUTPUT);
  pinMode(wePin, OUTPUT);
  pinMode(clearPin, OUTPUT);

  digitalWrite(wePin, HIGH);
  digitalWrite(oePin, HIGH);
  digitalWrite(clearPin, HIGH);

  // Digit patterns
  byte digits[] = {
    0x7B, // 0
    0x12, // 1
    0xB9, // 2
    0xB3, // 3
    0xD2, // 4
    0xE3, // 5
    0xEB, // 6
    0x32, // 7
    0xFB, // 8
    0xF2  // 9
  };

  const byte minusSign = 0x80;

  // Unsigned 0..255
  Serial.println("Programming ones place");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value, digits[value % 10]);
  }

  Serial.println("Programming tens place");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value + 256, digits[(value / 10) % 10]);
  }

  Serial.println("Programming hundreds place");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value + 512, digits[(value / 100) % 10]);
  }

  Serial.println("Programming sign");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value + 768, 0x00);
  }

  // Signed -128..127 (two's complement)
  Serial.println("Programming ones place (twos complement)");
  for (int value = -128; value <= 127; value++) {
    writeEEPROM((byte)value + 1024, digits[abs(value) % 10]);
  }

  Serial.println("Programming tens place (twos complement)");
  for (int value = -128; value <= 127; value++) {
    writeEEPROM((byte)value + 1280, digits[abs(value / 10) % 10]);
  }

  Serial.println("Programming hundreds place (twos complement)");
  for (int value = -128; value <= 127; value++) {
    writeEEPROM((byte)value + 1536, digits[abs(value / 100) % 10]);
  }

  Serial.println("Programming sign (twos complement)");
  for (int value = -128; value <= 127; value++) {
    if (value < 0) {
      writeEEPROM((byte)value + 1792, minusSign);
    } else {
      writeEEPROM((byte)value + 1792, 0x00);
    }
  }

  Serial.println("Reading EEPROM");
  printContents();
}

void loop() {
}