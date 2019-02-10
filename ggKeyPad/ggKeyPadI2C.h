#ifndef __GG_KEY_PAD_I2C__
#define __GG_KEY_PAD_I2C__

#include <Arduino.h>
#include <Wire.h>

class ggKeyPadI2C
{

private:

  #define COMMAND_BUTTON_A            0x10
  #define COMMAND_BUTTON_B            0x11
  #define COMMAND_BUTTON_C            0x12

  #define COMMAND_LED_A0_ON           0x20
  #define COMMAND_LED_A0_OFF          0x21
  #define COMMAND_LED_A1_ON           0x22
  #define COMMAND_LED_A1_OFF          0x23
  #define COMMAND_LED_C0_ON           0x24
  #define COMMAND_LED_C0_OFF          0x25
  #define COMMAND_LED_C1_ON           0x26
  #define COMMAND_LED_C1_OFF          0x27
  
public:

  typedef enum tButton {
    eButtonA,
    eButtonB,
    eButtonC
  };

  typedef enum tLED {
    eOrangeA,
    eBlueA,
    eOrangeC,
    eBlueC
  };
  
  ggKeyPadI2C(byte aAddressI2C = 0x67)
  : mAddressI2C(aAddressI2C),
    mButtonA_On(false),
    mButtonB_On(false),
    mButtonC_On(false) {
  }

  void begin() {
    Wire.begin();
  }

  bool GetOn(tButton aButton) const {
    switch (aButton) {
      case eButtonA: return mButtonA_On;
      case eButtonB: return mButtonB_On;
      case eButtonC: return mButtonC_On;
      default: break;
    }
  }

  bool GetOff(tButton aButton) const {
    return !GetOn(aButton);
  }

  bool IsOn(tButton aButton) const {
    switch (aButton) {
      case eButtonA: Receive(COMMAND_BUTTON_A, mButtonA_On); return mButtonA_On;
      case eButtonB: Receive(COMMAND_BUTTON_B, mButtonB_On); return mButtonB_On;
      case eButtonC: Receive(COMMAND_BUTTON_C, mButtonC_On); return mButtonC_On;
      default: break;
    }
  }
  
  bool IsOff(tButton aButton) const {
    return !IsOn(aButton);
  }

  bool Switching(tButton aButton) const {
    bool vOn = GetOn(aButton);
    return vOn != IsOn(aButton);
  }
  
  bool SwitchingOn(tButton aButton) const {
    return Switching(aButton) && GetOn(aButton);
  }
  
  bool SwitchingOff(tButton aButton) const {
    return Switching(aButton) && GetOff(aButton);
  }
  
  void SetLED(tLED aLED, bool aOn) {
    switch (aLED) {
      case eOrangeA: SetCommand(aOn ? COMMAND_LED_A0_ON : COMMAND_LED_A0_OFF); break;
      case eBlueA:   SetCommand(aOn ? COMMAND_LED_A1_ON : COMMAND_LED_A1_OFF); break;
      case eOrangeC: SetCommand(aOn ? COMMAND_LED_C0_ON : COMMAND_LED_C0_OFF); break;
      case eBlueC:   SetCommand(aOn ? COMMAND_LED_C1_ON : COMMAND_LED_C1_OFF); break;
      default: break;
    }
  }

private:
  
  void SetCommand(byte aCommand) const {
    Wire.beginTransmission(mAddressI2C);
    Wire.write(aCommand);
    Wire.endTransmission();
  }

  template <typename TValue>
  void Receive(byte aCommand, TValue& aValue) const {
    SetCommand(aCommand);
    byte* vValueByte = (byte*)&aValue;
    Wire.requestFrom(mAddressI2C, sizeof(aValue));
    int vNumberOfBytes = Wire.available();
    if (vNumberOfBytes == sizeof(aValue)) {
      while (vNumberOfBytes--) *vValueByte++ = Wire.read();
    }
  }

  template <typename TValue>
  void Send(byte aCommand, TValue aValue) const {
    Wire.beginTransmission(mAddressI2C);
    Wire.write(aCommand);
    Wire.write((byte*)(&aValue), sizeof(aValue));
    Wire.endTransmission();
  }

  byte mAddressI2C;

  mutable bool mButtonA_On;
  mutable bool mButtonB_On;
  mutable bool mButtonC_On;
  
};

#endif // __GG_KEY_PAD_I2C__
