#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ggLCD.h"
#include "ggSampler.h"
#include "ggButton.h"
#include "TimerOne.h"
#include "ggWaveEngine.h"
#include "ggBase.h"


ggWaveEngine& mWaveEngine = ggWaveEngine::GetInstance(&PORTD, &DDRD, 1000, 10);

ggLCD mLCD(0x26);

ggSampler mSamplerUI(20.0f);

ggButton mButtonMode(8, true, true);
ggButton mButtonDec(9, true, true);
ggButton mButtonInc(10, true, true);

enum tEditMode {
  eEditWaveType,
  eEditFrequency,
  eEditLine3,
  eEditLine4
};

enum tWaveType {
  eWaveDummyFirst,
  eWaveSine,
  eWaveSquare,
  eWaveTriangle,
  eWaveSawtoothUp,
  eWaveSawtoothDown,
  eWaveConst,
  eWaveDummyLast
};

tEditMode mEditMode = eEditWaveType;
tWaveType mWaveType = eWaveSine;

float mFrequency = 50;

void UpdateUI_WaveType()
{
  mLCD.setCursor(0, 0);
  mLCD.print("Wave : ");
  switch (mWaveType) {
    case eWaveSine:         mLCD.print("Sine");          break;
    case eWaveSquare:       mLCD.print("Square");        break;
    case eWaveTriangle:     mLCD.print("Triangle");      break;
    case eWaveSawtoothUp:   mLCD.print("Sawtooth Up");   break;
    case eWaveSawtoothDown: mLCD.print("Sawtooth Down"); break;
    case eWaveConst:        mLCD.print("Const");         break;
    default: break;
  }
  mLCD.clearLineEnd();
}

void UpdateUI_Frequency()
{
  mLCD.setCursor(0, 1);
  mLCD.print("Freq : ");
  if (mFrequency < 1000) {
    mLCD.print(mFrequency);
    mLCD.print(" Hz");
  }
  else if (mFrequency < 1000000) {
    mLCD.print(0.001f * mFrequency);
    mLCD.print(" kHz");
  }
  else {
    mLCD.print(0.000001f * mFrequency);
    mLCD.print(" MHz");
  }
  mLCD.clearLineEnd();
}

void UpdateUI_EditMode()
{
  for (int vLine = 0; vLine < 2; vLine++) {
    mLCD.setCursor(5, vLine);
    mLCD.print(vLine == mEditMode ? (char)7 : (char)6);
  }
}

void UpdateUI()
{
  UpdateUI_WaveType();
  UpdateUI_Frequency();
  UpdateUI_EditMode();
}

void SetupWave()
{
  switch (mWaveType) {
    case eWaveSquare:       mWaveEngine.SetupSquare();       break;
    case eWaveSine:         mWaveEngine.SetupSine();         break;
    case eWaveTriangle:     mWaveEngine.SetupTriangle();     break;
    case eWaveSawtoothUp:   mWaveEngine.SetupSawtoothUp();   break;
    case eWaveSawtoothDown: mWaveEngine.SetupSawtoothDown(); break;
    case eWaveConst:        mWaveEngine.SetupConst();        break;
    default: break;
  }
}

void WaveTypeInc()
{
  mWaveType = (tWaveType)(mWaveType + 1);
  if (mWaveType == eWaveDummyLast) mWaveType = tWaveType(eWaveDummyFirst + 1);
  SetupWave();
  UpdateUI();
}

void WaveTypeDec()
{
  mWaveType = (tWaveType)(mWaveType - 1);
  if (mWaveType == eWaveDummyFirst) mWaveType = tWaveType(eWaveDummyLast - 1);
  SetupWave();
  UpdateUI();
}

void ChangeEditMode()
{
  switch (mEditMode) {
    case eEditWaveType: mEditMode = eEditFrequency; break;
    case eEditFrequency: mEditMode = eEditLine3; break;
    case eEditLine3: mEditMode = eEditLine4; break;
    case eEditLine4: mEditMode = eEditWaveType; break;
    default: break;
  }
  UpdateUI_EditMode();
}

void ChangeFrequency(long aDelta)
{
  if (aDelta != 0) {

    float vFactor = 0.1f * ggRoundTo125((aDelta > 0 ? 1.001f : 0.999f) * mFrequency, true);
    mFrequency += vFactor * aDelta;
    mFrequency = vFactor * (long)(mFrequency / vFactor + 0.5f);
    if (mFrequency < 0.1f) mFrequency = 0.1f;
    if (mFrequency > 20000.0f) mFrequency = 20000.0f;

    mWaveEngine.SetFrequency(mFrequency);

    UpdateUI_Frequency();
    UpdateUI_EditMode();
  }
}

void SetupCursorChars()
{
  static const uint8_t vCharColon[8] = {B00000, B00000, B00100, B00000, B00000, B00100, B00000, B00000};
  static const uint8_t vCharArrow[8] = {B01000, B01100, B01110, B01111, B01110, B01100, B01000, B00000};
  mLCD.createChar(6, (uint8_t*)vCharColon);
  mLCD.createChar(7, (uint8_t*)vCharArrow);
}

void setup()
{
  SetupWave();
  mWaveEngine.begin();
  mWaveEngine.SetFrequency(mFrequency);
  mLCD.begin();
  SetupCursorChars();
  mButtonMode.begin();
  mButtonInc.begin();
  mButtonDec.begin();
  UpdateUI();
}

void loop()
{
  
  // check buttons click
  if (mButtonMode.SwitchingOn()) {
    ChangeEditMode();
  }
  if (mButtonInc.SwitchingOn()) {
    switch (mEditMode) {
      case eEditWaveType: WaveTypeInc(); break;
      case eEditFrequency: ChangeFrequency(1); break;
      default: break;
    }
  }
  if (mButtonDec.SwitchingOn()) {
    switch (mEditMode) {
      case eEditWaveType: WaveTypeDec(); break;
      case eEditFrequency: ChangeFrequency(-1); break;
      default: break;
    }
  }
  
  // check buttons pressed
  if (mSamplerUI.IsDue()) {    
    long vTimeDec = mButtonDec.GetTimeOn();
    if (mEditMode == eEditFrequency) {
      if (vTimeDec > 500) ChangeFrequency(-1);
    }
    long vTimeInc = mButtonInc.GetTimeOn();
    if (mEditMode == eEditFrequency) {
      if (vTimeInc > 500) ChangeFrequency(1);
    }
  }
}


