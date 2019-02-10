#include "ggWaveEngine.h"
#include "TimerOne.h"


// singleton: THE one and only instance!
ggWaveEngine ggWaveEngine::mInstance;


// wave data
int mValuesSize(0);
byte* mValues(0);
byte* mValuesBegin(0);
byte* mValuesEnd(0);
byte* mValuesIterator(0);


// wave sampling
int mIndexStep(1);
long mSampleMicroSeconds(10);
long mSampleMicroSecondsMin(10);
volatile unsigned char* mPortDataRegister(&PORTB);
volatile unsigned char* mPortDataDirectionRegister(&DDRB);
  
  
ggWaveEngine::ggWaveEngine()
{
}


ggWaveEngine::~ggWaveEngine()
{
}


ggWaveEngine& ggWaveEngine::GetInstance(volatile unsigned char* aPortDataRegister,
                                        volatile unsigned char* aPortDataDirectionRegister,
                                        const int aSize,
                                        long aSampleMicroSecondsMin)
{
  mPortDataRegister = aPortDataRegister;
  mPortDataDirectionRegister = aPortDataDirectionRegister;
  if (aSize != mValuesSize) {
    mValuesSize = aSize;
    mIndexStep = 1;
    mSampleMicroSeconds = aSampleMicroSecondsMin;
    mSampleMicroSecondsMin = aSampleMicroSecondsMin;
    delete[] mValues;
    mValues = new byte[mValuesSize];
    mValuesBegin = mValues;
    mValuesEnd = mValues + mValuesSize;
    mValuesIterator = mValuesBegin;
  }
  return mInstance;
}


void SampleWave()
{
  *mPortDataRegister = *mValuesIterator;
  mValuesIterator += mIndexStep;
  if (mValuesIterator >= mValuesEnd) mValuesIterator -= mValuesSize;
}


void ggWaveEngine::begin()
{
  *mPortDataDirectionRegister = B11111111;
  Timer1.initialize(mSampleMicroSeconds);
  Timer1.attachInterrupt(SampleWave);
}


void ggWaveEngine::SetupSquare()
{
  for (long vIndex = 0; vIndex < mValuesSize; vIndex++) {
    mValues[vIndex] = vIndex < mValuesSize / 2 ? 0 : 255;
  }
}


void ggWaveEngine::SetupSine()
{
  for (int vIndex = 0; vIndex < mValuesSize; vIndex++) {
    float vW = 2.0f * M_PI * vIndex / mValuesSize;
    mValues[vIndex] = 127.5f + 127.5f * sin(vW);
  }
}


void ggWaveEngine::SetupSawtoothUp()
{
  for (long vIndex = 0; vIndex < mValuesSize; vIndex++) {
    mValues[vIndex] = 256 * vIndex / mValuesSize;
  }
}


void ggWaveEngine::SetupSawtoothDown()
{
  for (long vIndex = 0; vIndex < mValuesSize; vIndex++) {
    mValues[vIndex] = 255 - 256 * vIndex / mValuesSize;
  }
}


void ggWaveEngine::SetupTriangle()
{
  for (long vIndex = 0; vIndex < mValuesSize; vIndex++) {
    mValues[vIndex] = abs(255 - 511 * vIndex / mValuesSize);
  }
}


void ggWaveEngine::SetupConst()
{
  for (long vIndex = 0; vIndex < mValuesSize; vIndex++) {
    mValues[vIndex] = 127;
  }
}


float CalculateFrequency(long aSampleMicros, int aIndexStep)
{
  return (1000000.0f * (float)aIndexStep) / ((float)aSampleMicros * (float)mValuesSize);
}


long CalculateSampleMicros(float aFrequency, int aIndexStep)
{
  return (1000000.0f * (float)aIndexStep) / ((float)aFrequency * (float)mValuesSize);
}


void ggWaveEngine::Start()
{
  Timer1.initialize(mSampleMicroSeconds);
  Timer1.start();
}


void ggWaveEngine::Stop()
{
  Timer1.stop();
}


void ggWaveEngine::SetFrequency(float aFrequency)
{
  int vIndexStepBest = 0;
  long vSampleMicrosBest = 0;
  float vFrequencyBest = 0.0f;
  float vErrorBest = 1.0f;

  for (int vIndexStep = 1; vIndexStep <= mValuesSize / 4; vIndexStep++) {
    long vSampleMicros = CalculateSampleMicros(aFrequency, vIndexStep);
    if (vSampleMicros >= mSampleMicroSecondsMin) {
      float vFrequency = CalculateFrequency(vSampleMicros, vIndexStep);
      float vErrorFrequency = ((float)aFrequency - vFrequency) / aFrequency;
      float vErrorSampling = 2.0f * (float)vIndexStep / (float)mValuesSize;
      float vError = vErrorFrequency*vErrorFrequency + vErrorSampling*vErrorSampling;
      if (vError < vErrorBest) {
        vErrorBest = vError;
        vSampleMicrosBest = vSampleMicros;
        vIndexStepBest = vIndexStep;
      }
    }
  }
  
  if (vSampleMicrosBest >= mSampleMicroSecondsMin) {
    Stop();
    mSampleMicroSeconds = vSampleMicrosBest;
    mIndexStep = vIndexStepBest;
    Start();
  }
}


float ggWaveEngine::GetFrequency() const
{
  return CalculateFrequency(mSampleMicroSeconds, mIndexStep);
}


