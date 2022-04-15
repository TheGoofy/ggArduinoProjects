#include <Wire.h> // arduino / esp8266
// #include <i2c_t3.h> // teensy3

#define SDA_PIN 4
#define SCL_PIN 5

void setup()
{
  // initialize I2C interface
  // Wire.begin(); // A4: SDA, A5: SCL // arduino
  Wire.begin(SDA_PIN, SCL_PIN); // esp8266
  // Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_INT, 800000); // teensy3

  // initialize serial communication (for debugging output only)
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("\nI2C Scanner");
}

 
void loop()
{
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of the Write.endTransmisstion
    // to see if a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) Serial.print("0");
      Serial.print(address,HEX);
      Serial.println();
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  
  if (nDevices == 0) Serial.print("No I2C devices found\n\n");
  else Serial.printf("Found %d I2C devices\n\n", nDevices);

  // wait 5 seconds for next scan
  delay(5000);
}
