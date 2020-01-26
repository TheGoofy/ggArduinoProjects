#pragma once

#include <time.h>
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
    mTime(0) {
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
  time_t GetTime() const {
    return mTime;
  }

  // see: http://www.cplusplus.com/reference/ctime/strftime/
  String GetTime(const char* aFormat) {
    if (mTimeValid) {
      #define M_BUFFER_SIZE 48
      static char vBuffer[M_BUFFER_SIZE];
      strftime(vBuffer, M_BUFFER_SIZE, aFormat, localtime(&mTime));
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
      time(&mTime);
      localtime_r(&mTime, &mTimeInfo);
      mTimeValid = mTimeInfo.tm_year >= 2020 - 1900;
      CheckTimers();
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
        aTimer.mDueTime = mTime + aTimer.mPeriod;
      }
    }
  }

  void CheckTimers() {
    if (mTimeValid) {
      std::for_each(mTimers.begin(), mTimers.end(), [&] (cTimer& aTimer) {
        if (aTimer.mDueTime == 0) {
          CalculateNextDueTime(aTimer);
        }
        else if (mTime >= aTimer.mDueTime) {
          aTimer.mFunc(aTimer.mPeriod);
          CalculateNextDueTime(aTimer);
        }
      });
    }
  }

  const String mServerNTP;
  const String mTimeZoneInfo;

  unsigned long mMillisLast;
  bool mTimeValid;
  time_t mTime;
  tm mTimeInfo;

  std::vector<cTimer> mTimers;

};
