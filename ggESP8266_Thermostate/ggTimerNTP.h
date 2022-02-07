#pragma once

#include <ctime>
#include <functional>

class ggTimerNTP {

public:

  /**
   * https://www.ntppool.org/
   * https://sites.google.com/a/usapiens.com/opnode/time-zones
   */
  ggTimerNTP(const char* aServerNTP,
             const char* aTimeZoneInfo)
  : mServerNTP(aServerNTP),
    mTimeZoneInfo(aTimeZoneInfo),
    mMillisLast(0),
    mTimeValid(false),
    mTimeT(0)
  {
    memset(&mTimeInfo, 0, sizeof(tm));
  }

  typedef std::function<void(uint32_t)> tFunc;

  void AddTimer(uint32_t aPeriod, tFunc aFunc) {
    if ((aPeriod > 0) && (aFunc != nullptr)) {
      cTimer vTimer(aPeriod, aFunc);
      CalculateNextDueTime(vTimer);
      mTimers.push_back(vTimer);
    }
  }

  bool IsTimeValid() const {
    return mTimeValid;
  }

  // unix time: http://www.cplusplus.com/reference/ctime/time_t/
  time_t GetTimeT() const {
    return mTimeT;
  }

  // see: http://www.cplusplus.com/reference/ctime/strftime/
  String GetTime(const char* aFormat) const {
    if (mTimeValid) {
      #define M_BUFFER_SIZE 48
      static char vBuffer[M_BUFFER_SIZE];
      strftime(vBuffer, M_BUFFER_SIZE, aFormat, localtime(&mTimeT));
      return String(vBuffer);
    }
    return "unknown";
  }

  void Begin() {
    configTime(0, 0, mServerNTP.c_str());
    setenv("TZ", mTimeZoneInfo.c_str(), 1);
    mMillisLast = millis();
  }

  void Run() {
    unsigned long vMillis = millis();
    unsigned long vMillisDelta = vMillis - mMillisLast;
    if (vMillisDelta > 1000) {
      mMillisLast = vMillis;
      mTimeT = time(nullptr);
      mTimeValid = (mTimeT != -1) && (mTimeT / (60*60*24*365) + 1970 >= 2020);
      if (mTimeValid) {
        localtime_r(&mTimeT, &mTimeInfo);
        mTimeValid = mTimeInfo.tm_year + 1900 >= 2020;
      }
      CheckTimers();
    }
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggTimerNTP", aName);
    vDebug.PrintF("mServerNTP = %s\n", mServerNTP.c_str());
    vDebug.PrintF("mTimeZoneInfo = %s\n", mTimeZoneInfo.c_str());
    vDebug.PrintF("mMillisLast = %d\n", mMillisLast);
    vDebug.PrintF("mTimeValid = %s\n", mTimeValid ? "true" : "false");
    vDebug.PrintF("mTimeT = %d\n", mTimeT);
    vDebug.PrintF("GetTime() => %s\n", GetTime("%d-%m-%Y %H:%M:%S").c_str());
    for (uint32_t vTimerIndex = 0; vTimerIndex < mTimers.size(); vTimerIndex++) {
      const cTimer& vTimer = mTimers[vTimerIndex];
      vDebug.PrintF("mTimer[%ld]: mPeriod = %ld, mDueTime = %lld\n",
        vTimerIndex, vTimer.mPeriod, vTimer.mDueTime);
    }
  }

private:

  struct cTimer {  
    cTimer(uint32_t aPeriod, tFunc aFunc)
    : mPeriod(aPeriod), mFunc(aFunc), mDueTime(0) {
    }    
    uint32_t mPeriod;
    tFunc mFunc;
    time_t mDueTime;
  };

  void CalculateNextDueTime(cTimer& aTimer) {
    if (mTimeValid) {
      // align with midnight if possible (86400 = 24*60*60)
      if (86400 % aTimer.mPeriod == 0) {
        tm vTimeInfo = mTimeInfo;
        uint32_t vSeconds = vTimeInfo.tm_sec + 60*vTimeInfo.tm_min + 3600*vTimeInfo.tm_hour;
        vSeconds = (vSeconds / aTimer.mPeriod + 1) * aTimer.mPeriod;
        vTimeInfo.tm_hour = vSeconds / 3600;
        vTimeInfo.tm_min = (vSeconds / 60) % 60;
        vTimeInfo.tm_sec = vSeconds % 60;
        aTimer.mDueTime = mktime(&vTimeInfo);
      }
      else {
        aTimer.mDueTime = mTimeT + aTimer.mPeriod;
      }
    }
  }

  void CheckTimers() {
    if (mTimeValid) {
      for (cTimer& vTimer : mTimers) {
        if (vTimer.mDueTime == 0) {
          CalculateNextDueTime(vTimer);
        }
        else if (mTimeT >= vTimer.mDueTime) {
          vTimer.mFunc(vTimer.mPeriod);
          CalculateNextDueTime(vTimer);
        }
      }
    }
  }

  const String mServerNTP;
  const String mTimeZoneInfo;

  unsigned long mMillisLast;
  bool mTimeValid;
  time_t mTimeT;
  tm mTimeInfo;

  std::vector<cTimer> mTimers;

};
