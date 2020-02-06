#include "ggAveragesT.h"


typedef ggAveragesT<float, float> tAverages;


void Print(const tAverages& aAverages)
{
  Serial.printf("count = %f\n", aAverages.GetNumberOfSamples());
  Serial.printf("min = %f\n", aAverages.GetMin());
  Serial.printf("max = %f\n", aAverages.GetMax());
  Serial.printf("sum = %f\n", aAverages.GetSum());
  Serial.printf("mean = %f\n", aAverages.GetMean());
  Serial.printf("stddev = %f\n", aAverages.GetStdDev());
}


void setup()
{
  Serial.begin(115200);
  Serial.println("");

  tAverages vAveragesA;
  tAverages vAveragesB;
  tAverages vAveragesC;

  vAveragesA.AddValue(1);
  vAveragesA.AddValue(2);
  vAveragesA.AddValue(2);
  vAveragesA.AddValue(3);

  vAveragesB.AddValue(7);
  vAveragesB.AddValue(8);
  vAveragesB.AddValue(9);

  vAveragesC.AddValues(vAveragesA);
  vAveragesC.AddValues(vAveragesB);  
/*  
  Serial.printf("vAveragesA:\n");
  Print(vAveragesA);
  Serial.printf("vAveragesB:\n");
  Print(vAveragesB);
  Serial.printf("vAveragesC:\n");
  Print(vAveragesC);
*/
  vAveragesA.AddValue(-5);
  vAveragesB.AddValue(3);
  vAveragesB.AddValue(2);
  vAveragesC.AddValue(-5);
  vAveragesC.AddValue(3);
  vAveragesC.AddValue(2);

  Serial.printf("vAveragesA:\n");
  Print(vAveragesA);
  Serial.printf("vAveragesB:\n");
  Print(vAveragesB);
  Serial.printf("vAveragesC:\n");
  Print(vAveragesC);
  
}


void loop()
{
}
