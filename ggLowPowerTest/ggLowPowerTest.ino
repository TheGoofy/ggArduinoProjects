

void SetClockPrescaler(uint8_t aClockPreScale)
{
  // prescale larger than 8 is not supported
  if (aClockPreScale <= 8) {
    // Disable interrupts.
    uint8_t vOldSREG = SREG;
    cli();
    // Enable change.
    CLKPR = _BV(CLKPCE); // write the CLKPCE bit to one and all the other to zero
    // Change clock division.
    CLKPR = aClockPreScale; // write the CLKPS0..3 bits while writing the CLKPE bit to zero
    // Recopy interrupt register.
    SREG = vOldSREG;
  }
}

void LowPowerDelay(unsigned long aMillis)
{
  int vClockPreScale = 9;
  while (vClockPreScale > 0) {
    --vClockPreScale;
    int vClockDivider = 1 << vClockPreScale;    
    unsigned long vTicks = aMillis / vClockDivider;
    aMillis %= vClockDivider;
    if (vTicks > 0) {
      SetClockPrescaler(vClockPreScale);
      delay(vTicks);
      SetClockPrescaler(0);
    }
    if (aMillis == 0) break;
  }
}

void setup()
{
  pinMode(13, OUTPUT);
}

void MyDelay(unsigned long aMillis)
{
  // delay(aMillis);
  LowPowerDelay(aMillis);
}

void loop()
{
  digitalWrite(13, HIGH);
  MyDelay(5);  
  digitalWrite(13, LOW);
  MyDelay(200);  
  digitalWrite(13, HIGH);
  MyDelay(5);  
  digitalWrite(13, LOW);
  MyDelay(1790);  
}
