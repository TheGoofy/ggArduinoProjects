/*

Arduino UNO
'uint8_t' (1 bytes) FLOPS: 294521
'uint16_t' (2 bytes) FLOPS: 173111
'uint32_t' (4 bytes) FLOPS: 72092
'uint64_t' (8 bytes) FLOPS: 34134
'float' (4 bytes) FLOPS: 64652
'double' (4 bytes) FLOPS: 64652

Arduino Mega 2560
'uint8_t' (1 bytes) FLOPS: 286509
'uint16_t' (2 bytes) FLOPS: 170308
'uint32_t' (4 bytes) FLOPS: 71116
'uint64_t' (8 bytes) FLOPS: 33553
'float' (4 bytes) FLOPS: 62485
'double' (4 bytes) FLOPS: 62485

LGT8F328
'uint8_t' (1 bytes) FLOPS:  674199
'uint16_t' (2 bytes) FLOPS: 390286
'uint32_t' (4 bytes) FLOPS: 160394
'uint64_t' (8 bytes) FLOPS: 77899
'float' (4 bytes) FLOPS: 148514
'double' (4 bytes) FLOPS: 148515

ESP8266
'uint8_t' (1 bytes) FLOPS: 3118640
'uint16_t' (2 bytes) FLOPS: 1670090
'uint32_t' (4 bytes) FLOPS: 1488676
'uint64_t' (8 bytes) FLOPS: 580138
'float' (4 bytes) FLOPS: 663896
'double' (8 bytes) FLOPS: 281933

Raspberry Pi Pico
'uint8_t' (1 bytes) FLOPS: 4852661
'uint16_t' (2 bytes) FLOPS: 3250341
'uint32_t' (4 bytes) FLOPS: 2904190
'uint64_t' (8 bytes) FLOPS: 756229
'float' (4 bytes) FLOPS: 577014
'double' (8 bytes) FLOPS: 329893

ESP32-C3-xiao
'uint8_t' (1 bytes) FLOPS: 10260882
'uint16_t' (2 bytes) FLOPS: 9638089
'uint32_t' (4 bytes) FLOPS: 11358150
'uint64_t' (8 bytes) FLOPS: 3457575
'float' (4 bytes) FLOPS: 1221131
'double' (8 bytes) FLOPS: 727954

ESP32-c3-mini
'uint8_t' (1 bytes) FLOPS: 10261146
'uint16_t' (2 bytes) FLOPS: 9640180
'uint32_t' (4 bytes) FLOPS: 11361054
'uint64_t' (8 bytes) FLOPS: 3457605
'float' (4 bytes) FLOPS: 1221127
'double' (8 bytes) FLOPS: 727938

ESP32-wroom-da
'uint8_t' (1 bytes) FLOPS: 14681592
'uint16_t' (2 bytes) FLOPS: 14020820
'uint32_t' (4 bytes) FLOPS: 15167026
'uint64_t' (8 bytes) FLOPS: 5124132
'float' (4 bytes) FLOPS: 9434407
'double' (8 bytes) FLOPS: 1076559

NodeMCU-32S (ESP32)
'uint8_t' (1 bytes) FLOPS: 14681592
'uint16_t' (2 bytes) FLOPS: 14020329
'uint32_t' (4 bytes) FLOPS: 15167602
'uint64_t' (8 bytes) FLOPS: 5179736
'float' (4 bytes) FLOPS: 9527892
'double' (8 bytes) FLOPS: 1081440

Teensy 3.2
'uint8_t' (1 bytes) FLOPS: 11318939
'uint16_t' (2 bytes) FLOPS: 9616771
'uint32_t' (4 bytes) FLOPS: 11352026
'uint64_t' (8 bytes) FLOPS: 1955387
'float' (4 bytes) FLOPS: 798396
'double' (8 bytes) FLOPS: 330360

Teensy 4.0
'uint8_t' (1 bytes) FLOPS: 62637018
'uint16_t' (2 bytes) FLOPS: 54156512
'uint32_t' (4 bytes) FLOPS: 61785603
'uint64_t' (8 bytes) FLOPS: 22223456
'float' (4 bytes) FLOPS: 55803571
'double' (8 bytes) FLOPS: 33795201

*/


template <typename T>
int64_t CalculateFlopsT()
{
  volatile T vResult = 12.345;
  volatile T vCoeff1 = 3.1415;
  volatile T vCoeff2 = vCoeff1 * (vResult + vCoeff1) / (vResult + 1.001);
  int64_t vCountTotal = 100000;
  int64_t vCount = vCountTotal;
  int64_t vMicrosTotal = 0;
  int64_t vMicrosStart = 0;
  int64_t vMicrosEnd = 0;
  vMicrosStart = micros();
  while (vCount--) {
    vResult += vCoeff1;
    vResult *= vCoeff1;
    vResult -= vCoeff2;
    vResult /= vCoeff2;
  }
  vMicrosEnd = micros();
  vMicrosTotal = vMicrosEnd - vMicrosStart;
  return (4 * vCountTotal * 1000000) / vMicrosTotal;
}


template <typename T>
void SerialPrintFlopsT(const char* aTypeName)
{
  int64_t vFlops = CalculateFlopsT<T>();
  char vStringBuffer[256];
  sprintf(vStringBuffer, "'%s' (%d bytes) FLOPS: %lld", aTypeName, sizeof(T), vFlops);
  Serial.println(vStringBuffer);
   Serial.print((unsigned long)vFlops);
  // Serial.printf("'%s' (%d bytes) FLOPS: %lld\n", aTypeName, sizeof(T), vFlops);
}


void setup()
{
  Serial.begin(115200);
  Serial.print("\nHi FLOPS!\n");
}


void loop()
{
  Serial.println();
  SerialPrintFlopsT<uint8_t>("uint8_t");
  SerialPrintFlopsT<uint16_t>("uint16_t");
  SerialPrintFlopsT<uint32_t>("uint32_t");
  SerialPrintFlopsT<uint64_t>("uint64_t");
  SerialPrintFlopsT<float>("float");
  SerialPrintFlopsT<double>("double");
  delay(5000);
}
