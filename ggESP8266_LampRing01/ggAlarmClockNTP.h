#pragma once

#include "ggClockNTP.h"
#include <memory>

class ggAlarmClockNTP : public ggClockNTP
{

public:

  ggAlarmClockNTP(const char* aServerNTP,
                  const char* aTimeZoneInfo)
  : ggClockNTP(aServerNTP, aTimeZoneInfo) {
  }

  enum tDays {
    eNoDay       = 0,
    eSunday      = 0b00000001,
    eMonday      = 0b00000010,
    eTuesday     = 0b00000100,
    eWednesday   = 0b00001000,
    eThursday    = 0b00010000,
    eFriday      = 0b00100000,
    eSaturday    = 0b01000000,
    eWorkDays    = eMonday | eTuesday | eWednesday | eThursday | eFriday,
    eWeekendDays = eSaturday | eSunday,
    eAllDays     = eWorkDays | eWeekendDays
  };

  class cAlarm;
  typedef std::function<void(cAlarm& aAlarm)> tAlarmFunc;

  class cAlarm {
  public:

    cAlarm() 
    : mActive(true),
      mMin(0),
      mHour(0),
      mDays(0),
      mAlarmFunc(nullptr),
      mTimeMatch(false) {
    }

    cAlarm(uint8_t aMin,
           uint8_t aHour,
           uint8_t aDays,
           tAlarmFunc aFunc)
    : mActive(true),
      mMin(aMin),
      mHour(aHour),
      mDays(aDays),
      mAlarmFunc(aFunc),
      mTimeMatch(false) {
    }

    bool mActive;

    uint8_t mMin;
    uint8_t mHour;
    uint8_t mDays;

    void OnAlarm(tAlarmFunc aFunc) {
      mAlarmFunc = aFunc;
    }

    virtual String ToJson() const {
      return String() +
        "\"mActive\":" + mActive + ","
        "\"mMin\":" + mMin + ","
        "\"mHour\":" + mHour + ","
        "\"mDays\":" + mDays;
    }

  private:

    friend class ggAlarmClockNTP;

    void Check(const tm& aTimeInfo) {
      // check if activated
      if (mActive && (mAlarmFunc != nullptr)) {
        // avoid to execute the alarm multiple times
        if (!mTimeMatch) {
          // check it time matches
          if ((mMin == aTimeInfo.tm_min) && (mHour == aTimeInfo.tm_hour)) {
            // single alarm?
            if (mDays == 0) {
              mActive = false;
              mAlarmFunc(*this);
            }
            else {
              // check weekdays          
              uint8_t vDay = 1 << (aTimeInfo.tm_wday % 7); // 0 is sunday
              if (vDay & mDays) {
                mTimeMatch = true;
                mAlarmFunc(*this);
              }
            }
          }
        }
        else {
          if ((mMin != aTimeInfo.tm_min) || (mHour != aTimeInfo.tm_hour)) {
            mTimeMatch = false;
          }
        }
      }
    }

    tAlarmFunc mAlarmFunc;
    mutable bool mTimeMatch;
  };

  typedef std::vector<std::shared_ptr<cAlarm>> tAlarms;

  tAlarms& Alarms() {
    return mAlarms;
  }

  const tAlarms& Alarms() const {
    return mAlarms;
  }

  void AddAlarm(uint8_t aMin,
                uint8_t aHour,
                uint8_t aDays,
                tAlarmFunc aFunc) {
    mAlarms.push_back(std::make_shared<cAlarm>(aMin, aHour, aDays, aFunc));
  }

  virtual void Run() override {
    ggClockNTP::Run();
    if (IsTimeValid()) {
      for (auto& vAlarm : mAlarms) {
        vAlarm->Check(GetTimeTM());
      }
    }
  }

  String AlarmsToJson() const {
    String vAlarmsJson("\"mAlarms\":[\n");
    for (auto vAlarmIt = mAlarms.begin(); vAlarmIt != mAlarms.end(); vAlarmIt++) {
      vAlarmsJson += "  {" + (*vAlarmIt)->ToJson() + "}";
      vAlarmsJson += (std::next(vAlarmIt) != mAlarms.end()) ? ",\n" : "\n";
    }
    vAlarmsJson += "]";
    return vAlarmsJson;
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggAlarmClockNTP", aName);
    ggClockNTP::PrintDebug();
    vDebug.PrintF("mAlarms = %s\n", AlarmsToJson().c_str());
  }

private:

  tAlarms mAlarms;

};
