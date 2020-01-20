
void getSensorData(int address, unsigned int dataBuffer[6]) {
  Wire.beginTransmission(address);
  // Send measurement command
  Wire.write(0x2C);
  Wire.write(0x06);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(200); // default 500, lets try lower
  Wire.requestFrom(address, 6);

  if (Wire.available() == 6)
  {
    dataBuffer[0] = Wire.read();
    dataBuffer[1] = Wire.read();
    dataBuffer[2] = Wire.read();
    dataBuffer[3] = Wire.read();
    dataBuffer[4] = Wire.read();
    dataBuffer[5] = Wire.read();
  }
}
