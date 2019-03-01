#include "ggEEPromValueT.h"


ggEEPromValueT<int> mValueInt(10);
ggEEPromValueT<float> mValueFloat(3.1415f);
ggEEPromValueT<double> mValueDouble(42.0);


void Print()
{
  Serial.printf("mValueInt = %d\n", mValueInt.Get());
  Serial.printf("mValueFloat = %f\n", mValueFloat.Get());
  Serial.printf("mValueDouble = %f\n", mValueDouble.Get());
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  ggEEPromValue::Begin();

  Serial.println("Default or stored values:");
  Print();

  mValueInt.Set(mValueInt.Get() + 1);
  mValueFloat.Set(mValueFloat.Get() + 1.0f);
  mValueDouble.Set(mValueDouble.Get() + 1.0);

  Serial.println("New values:");
  Print();
}


void loop()
{
}
