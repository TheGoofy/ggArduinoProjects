#pragma once

#include <ctime>
#include <functional>

class ggClockNTP
{

public:

  ggClockNTP(const char* aServerNTP,
             const char* aTimeZoneInfo)
  : mServerNTP(aServerNTP),
    mTimeZoneInfo(aTimeZoneInfo),
    mMillisLast(0),
    mTimeValidFunc(nullptr),
    mTimeValid(false),
    mTimeT(0)
  {
    memset(&mTimeInfo, 0, sizeof(tm));
  }

  bool IsTimeValid() const {
    return mTimeValid;
  }

  typedef std::function<void (bool aTimeValid)> tTimeValidFunc;
  void OnTimeValid(tTimeValidFunc aTimeValidFunc) {
    mTimeValidFunc = aTimeValidFunc;
  }

  // unix time: http://www.cplusplus.com/reference/ctime/time_t/
  time_t GetTimeT() const {
    return mTimeValid ? mTimeT : 0;
  }

  // https://en.cppreference.com/w/cpp/chrono/c/tm
  const tm& GetTimeTM() const {
    return mTimeInfo;
  }

  // formatted time: http://www.cplusplus.com/reference/ctime/strftime/
  String GetTime(const char* aFormat) const {
    if (mTimeValid) {
      #define M_BUFFER_SIZE 48
      static char vBuffer[M_BUFFER_SIZE];
      strftime(vBuffer, M_BUFFER_SIZE, aFormat, localtime(&mTimeT));
      return String(vBuffer);
    }
    return "unknown";
  }

  virtual void Begin() {
    configTime(0, 0, mServerNTP.c_str());
    setenv("TZ", mTimeZoneInfo.c_str(), 1);
  }

  virtual void Run() {
    unsigned long vMillis = millis();
    unsigned long vMillisDelta = vMillis - mMillisLast;
    if (vMillisDelta > 1000) {
      mMillisLast = vMillis;
      mTimeT = time(nullptr);
      bool vTimeValid = (mTimeT != -1) && (mTimeT / (60*60*24*365) + 1970 >= 2020);
      if (mTimeValid != vTimeValid) {
        mTimeValid = vTimeValid;
        if (mTimeValidFunc != nullptr) {
          mTimeValidFunc(vTimeValid);
        }
      }
      if (mTimeValid) {
        localtime_r(&mTimeT, &mTimeInfo);
      }
    }
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggClockNTP", aName);
    vDebug.PrintF("mServerNTP = %s\n", mServerNTP.c_str());
    vDebug.PrintF("mTimeZoneInfo = %s\n", mTimeZoneInfo.c_str());
    vDebug.PrintF("mMillisLast = %d\n", mMillisLast);
    vDebug.PrintF("mTimeValidFunc = 0x%08X\n", std::addressof(mTimeValidFunc));
    vDebug.PrintF("mTimeValid = %s\n", mTimeValid ? "true" : "false");
    vDebug.PrintF("mTimeT = %d\n", mTimeT);
    vDebug.PrintF("GetTime() => %s\n", GetTime("%d-%m-%Y %H:%M:%S").c_str());
  }

private:

  const String mServerNTP;
  const String mTimeZoneInfo;

  unsigned long mMillisLast;
  tTimeValidFunc mTimeValidFunc;
  bool mTimeValid;
  time_t mTimeT;
  tm mTimeInfo;

};
