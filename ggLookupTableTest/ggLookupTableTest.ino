#include "ggHelpers.h"
#include "ggLookupTableT.h"


template <typename TFunc>
unsigned long ExecutionDuration(TFunc aFunc)
{
  unsigned long vMicrosStart = micros();
  aFunc();
  unsigned long vMicrosEnd = micros();
  return vMicrosEnd - vMicrosStart;
}


template <typename TFunc>
unsigned long ExecutionDuration(uint32_t aLoops, TFunc aFunc)
{
  return ExecutionDuration([&] () {
    for (uint32_t vLoop = 0; vLoop < aLoops; vLoop++) {
      aFunc();
    }
  });
}


template <typename TValue, typename TFunc>
void Sweep(const TValue& aMin,
           const TValue& aMax,
           uint32_t aSteps,
           TFunc aFunc)
{
  TValue vDelta = (aMax - aMin) / (aSteps - 1);
  for (uint32_t vStep = 0; vStep < aSteps; vStep++) {
    aFunc(aMin + vStep * vDelta);
  }
}


template <typename TLookupTable>
void TestLookupTable(const TLookupTable& aLookupTable,
                     uint32_t aSweepSteps = 101,
                     uint32_t aLoops = 50)
{
  Serial.printf("Table size = %d (%d bytes)\n", aLookupTable.GetSize(), sizeof(TLookupTable));
  Serial.printf("%d loops with %d steps from %f to %f\n",
                aLoops, aSweepSteps, aLookupTable.GetInputMin(), aLookupTable.GetInputMax());
  {
    float vOutputSum = 0.0f;
    unsigned long vDuration = ExecutionDuration(aLoops, [&] () {
      Sweep(aLookupTable.GetInputMin(), aLookupTable.GetInputMax(), aSweepSteps, [&] (float aInput) {
        vOutputSum += aLookupTable.GetExact(aInput);
      });
    });
    Serial.printf("\"GetExact\" Duraton = %d us, vOutputSum = %f\n", vDuration, vOutputSum);
    yield();
  }
  {
    float vOutputSum = 0.0f;
    unsigned long vDuration = ExecutionDuration(aLoops, [&] () {
      Sweep(aLookupTable.GetInputMin(), aLookupTable.GetInputMax(), aSweepSteps, [&] (float aInput) {
        vOutputSum += aLookupTable(aInput);
      });
    });
    Serial.printf("\"Get\" Duraton = %d us, vOutputSum = %f\n", vDuration, vOutputSum);
    yield();
  }
  {
    float vOutputSum = 0.0f;
    unsigned long vDuration = ExecutionDuration(aLoops, [&] () {
      Sweep(aLookupTable.GetInputMin(), aLookupTable.GetInputMax(), aSweepSteps, [&] (float aInput) {
        vOutputSum += aLookupTable.GetNN(aInput);
      });
    });
    Serial.printf("\"GetNN\" Duraton = %d us, vOutputSum = %f\n", vDuration, vOutputSum);
    yield();
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  {
    Serial.printf("\nTesting 1:1 Int Transfer Function\n");
    ggLookupTableT<int, 4> vLookupTable(-10, 10, [] (int aInput) { return aInput; });
    Sweep(-15, 15, 31, [&] (int aInput) {
      int vOutputA = vLookupTable.GetExact(aInput);
      int vOutputB = vLookupTable.Get(aInput);
      int vOutputC = vLookupTable.GetNN(aInput);
      Serial.printf("%d\t%d\t%d\t%d\n", aInput, vOutputA, vOutputB, vOutputC);
    });
  }

  auto vLinearFunc = [] (const float& aInput) {
    return aInput;
  };

  auto vSinFunc = [] (const float& aInput) {
    return sin(aInput);
  };

  auto vPowFunc = [] (const float& aInput) {
    return (pow(150.0f, aInput) - 1.0f) / (150.0f - 1.0f);
  };

  auto vLogFunc = [] (const float& aInput) {
    return (log((150.0f - 1.0f) * aInput + 1.0f) / log(150.0f));
  };

  {
    Serial.printf("\nTesting 1:1 Float Transfer Function\n");
    ggLookupTableT<float, 6> vLookupTable(0.0f, 1.0f, vLinearFunc);
    Sweep(-0.25f, 1.25f, 31, [&] (float aInput) {
      float vOutputA = vLookupTable.GetExact(aInput);
      float vOutputB = vLookupTable.Get(aInput);
      float vOutputC = vLookupTable.GetNN(aInput);
      Serial.printf("%f\t%f\t%f\t%f\n", aInput, vOutputA, vOutputB, vOutputC);
    });
  }
  {
    Serial.printf("\nTesting Performance \"linear\":\n");
    ggLookupTableT<float, 101> vLookupTable(0.0f, 1.0f, vLinearFunc);
    TestLookupTable(vLookupTable);
  }
  {
    Serial.printf("\nTesting Performance \"sin\":\n");
    ggLookupTableT<float, 501> vLookupTable(0.0f, 2.0f*M_PI, vSinFunc);
    TestLookupTable(vLookupTable);
  }
  {
    Serial.printf("\nTesting Performance \"pow\":\n");
    ggLookupTableT<float, 101> vLookupTable(0.0f, 1.0f, vPowFunc);
    TestLookupTable(vLookupTable);
  }
  {
    Serial.printf("\nTesting Performance \"log\":\n");
    ggLookupTableT<float, 101> vLookupTable(0.0f, 1.0f, vLogFunc);
    TestLookupTable(vLookupTable);
  }
}


void loop()
{
}
