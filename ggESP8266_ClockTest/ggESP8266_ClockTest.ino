#include <ESP8266WiFi.h>
#include "ggAlarmClockNTP.h"


ggAlarmClockNTP mAlarmClock("ch.pool.ntp.org", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("NTP Clock Test");
/*
  // WiFi.begin("ssid", "password"); 
  while (WiFi.status() != WL_CONNECTED) delay(200);
  Serial.printf("Connected to: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
*/
  mAlarmClock.Begin();

  mAlarmClock.OnTimeValid([&] (bool aTimeValid) {
    if (aTimeValid) Serial.printf("Time is now valid: %s\n", mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S").c_str());
    else Serial.printf("Time is invalid: %d\n", mAlarmClock.GetTimeT());
  });

  mAlarmClock.AddAlarm(15, 07, ggAlarmClockNTP::eNoDay, [&] (ggAlarmClockNTP::cAlarm& aAlarm) {
    Serial.print(mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S"));
    Serial.printf(" - Single alarm! - active: %d\n", aAlarm.mActive);
  });

  mAlarmClock.AddAlarm(16, 7, ggAlarmClockNTP::eWorkDays, [&] (ggAlarmClockNTP::cAlarm& aAlarm) {
    Serial.print(mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S"));
    Serial.printf(" - Work day alarm! - active: %d\n", aAlarm.mActive);
  });

  mAlarmClock.AddAlarm(0, 8, ggAlarmClockNTP::eWeekendDays, [&] (ggAlarmClockNTP::cAlarm& aAlarm) {
    Serial.print(mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S"));
    Serial.printf(" - Weekend day alarm! - active: %d\n", aAlarm.mActive);
  });

  mAlarmClock.AddAlarm(0, 0, ggAlarmClockNTP::eAllDays, [&] (ggAlarmClockNTP::cAlarm& aAlarm) {
    Serial.print(mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S"));
    Serial.printf(" - Midnight alarm! - active: %d\n", aAlarm.mActive);
  });

  mAlarmClock.AddAlarm(0, 12, ggAlarmClockNTP::eAllDays, [&] (ggAlarmClockNTP::cAlarm& aAlarm) {
    Serial.print(mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S"));
    Serial.printf(" - Lunchtime alarm! - active: %d\n", aAlarm.mActive);
  });

  Serial.println(mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S"));
}


int RSSItoQuality(int aRSSI)
{
  return std::min(std::max(2 * (aRSSI + 100), 0), 100);
}


void loop()
{
  /*
  delay(1000);
  Serial.println(mClock.GetTime("%d-%m-%Y %H:%M:%S"));
  Serial.println(mClock.GetTimeT());

  static tm vTimeInfo;
  memset(&vTimeInfo, 0, sizeof(tm));
  vTimeInfo.tm_sec = 0; // [0..59] (60,61)
  vTimeInfo.tm_min = 20; // [0..59]
  vTimeInfo.tm_hour = 21; // [0..23] hours sonce midnight
  vTimeInfo.tm_mday = 7; // [1..31] day of month
  vTimeInfo.tm_mon = 6; // [0..11] month
  vTimeInfo.tm_year = 2020 - 1900; // years sonce 1900
  vTimeInfo.tm_isdst = -1; // -1: unknown DST / 0: no DST / 1: DST
  time_t vTimeT = mktime(&vTimeInfo);
  Serial.println(vTimeT);
  Serial.println(vTimeT - mClock.GetTimeT());
  Serial.printf("days since sunday: %d\n", vTimeInfo.tm_wday);
  Serial.printf("days since january 1: %d\n", vTimeInfo.tm_yday);
  */

  static unsigned long vMillisLast = 0;
  unsigned long vMillis = millis();
  if (vMillis - vMillisLast > 10000) {
    vMillisLast = vMillis;
    Serial.printf("%s WiFiStatus=%d SSID=%s Quality=%d%% IP=%s\n", 
                  mAlarmClock.GetTime("%d-%m-%Y %H:%M:%S").c_str(),
                  WiFi.status(), WiFi.SSID().c_str(), RSSItoQuality(WiFi.RSSI()),
                  WiFi.localIP().toString().c_str());
  }

  mAlarmClock.Run();
  yield();
}
