#include "ggEEPromValueT.h"
#include "cSettings.h"


ggEEPromValueT<int> mValueInt(5);
ggEEPromValueT<float> mValueFloat(3.1415f);
ggEEPromValueT<double> mValueDouble(42.0);
ggEEPromValueT<cSettings> mSettings;


void Print()
{
  Serial.printf("mValueInt = %d\n", mValueInt.Value());
  Serial.printf("mValueFloat = %f\n", mValueFloat.Value());
  Serial.printf("mValueDouble = %lf\n", mValueDouble.Value());
  Serial.printf("mSettings.mData = %d\n", mSettings.Value().mData);
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.println("Default values:");
  Print();
  
  ggEEPromValue::Begin();

  Serial.println("Stored values:");
  Print();

  mValueInt.Set(mValueInt.Value() + 1);
  mValueFloat.Set(mValueFloat.Value() + 1.0f);
  mValueDouble.Set(mValueDouble.Value() + 1.0);

  float vFloat = mValueFloat;// = 0.01f;
  Serial.printf("vFloat = %f\n", vFloat);
  
  mSettings.Value().mData += 1;
  mSettings.Write();

  Serial.println("New values:");
  Print();
}


void loop()
{
}
