#include <ESP8266WiFi.h>
#include "ggTimerNTP.h"


ggTimerNTP mTimer("ch.pool.ntp.org", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("NTP Timer Test");

  // WiFi.begin("ssid", "password"); 
  while (WiFi.status() != WL_CONNECTED) delay(200);
  Serial.println("WiFi connected");
  
  mTimer.AddTimer(5, [] (uint32_t aPeriod) {
    Serial.println(mTimer.GetTime("%d-%m-%Y %H:%M:%S") + " period = " + aPeriod);
  });

  mTimer.AddTimer(6, [] (uint32_t aPeriod) {
    Serial.println(mTimer.GetTime("%d-%m-%Y %H:%M:%S") + " period = " + aPeriod);
  });

  mTimer.AddTimer(60, [] (uint32_t aPeriod) {
    Serial.println(mTimer.GetTime("%d-%m-%Y %H:%M:%S") + " period = " + aPeriod);
  });

  mTimer.Begin();
  
  Serial.println(mTimer.GetTime("%d-%m-%Y %H:%M:%S"));
}


void loop()
{
  mTimer.Run();
}
