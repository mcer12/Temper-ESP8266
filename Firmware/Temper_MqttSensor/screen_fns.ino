void refreshScreen() {

  for (int row = 0; row < sizeof(rows); row++) {
    for (int col = 0; col < sizeof(cols); col++) {
      shift.setNoUpdate(cols[col], content[row][col]);
    }
    shift.setNoUpdate(rows[row], 1); // set single pin HIGH
    shift.updateRegisters(); // update the pins to the set values
    shift.setNoUpdate(rows[row], 0); // set single pin HIGH
  }
  shift.set(rows[sizeof(rows) - 1], 0); // set single pin HIGH

}

void splitTo(uint8_t result[10][25]) {

}

void drawNumberL(int digit, int x, int y) {
  for (uint8_t row = 0; row < sizeof(digitsLarge[digit]) / sizeof(digitsLarge[digit][0]); row++) {
    for (uint8_t col = 0; col < sizeof(digitsLarge[digit][0]); col++) {
      content[row + y][col + x] = digitsLarge[digit][row][col];
    }
  }
}

void drawNumberS(int digit, int x, int y) {
  for (uint8_t row = 0; row < sizeof(digitsSmall[digit]) / sizeof(digitsSmall[digit][0]); row++) {
    for (uint8_t col = 0; col < sizeof(digitsSmall[digit][0]); col++) {
      content[row + y][col + x] = digitsSmall[digit][row][col];
    }
  }
}

void bufferNumberL(int digit, int x, int y) {
  for (uint8_t row = 0; row < sizeof(digitsLarge[digit]) / sizeof(digitsLarge[digit][0]); row++) {
    for (uint8_t col = 0; col < sizeof(digitsLarge[digit][0]); col++) {
      buffer[row + y][col + x] = digitsLarge[digit][row][col];
    }
  }
}

void drawPixel(uint8_t x, uint8_t y) {
  content[y][x] = 1;
}

void drawImage(uint8_t image[7][13]) {
  for (uint8_t row = 0; row < sizeof(rows); row++) {
    for (uint8_t col = 0; col < sizeof(cols); col++) {
      content[row][col] = image[row][col];
    }
  }
}

void drawDegreesSign() {
  drawPixel(10, 0);
  drawPixel(11, 0);
  drawPixel(12, 0);
  drawPixel(10, 1);
  drawPixel(12, 1);
  drawPixel(10, 2);
  drawPixel(11, 2);
  drawPixel(12, 2);
}

void blankPixel(uint8_t x, uint8_t y) {
  content[y][x] = 0;
}

void clearScreen() {
  for (uint8_t row = 0; row < sizeof(rows); row++) {
    for (uint8_t col = 0; col < sizeof(cols); col++) {
      content[row][col] = 0;
    }
  }
}
