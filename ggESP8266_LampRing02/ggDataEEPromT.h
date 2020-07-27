#pragma once

#include "ggValueEEPromString.h"

template <uint16_t TStringLength,
          uint16_t TNumScenesMax,
          typename TBrightness,
          uint16_t TNumBrightnesses,
          typename TColor,
          uint16_t TNumColors,
          uint16_t TNumAlarmsMax>

class ggDataEEPromT {

  static_assert(TNumScenesMax > 0, "max number of scenes size is too small");

public:

  typedef ggValueEEPromString<TStringLength> tString;
  typedef ggValueEEPromT<TBrightness> tBrightness;
  typedef ggValueEEPromT<TColor> tColor;

  struct cScene {
    tString mName;
    std::array<tBrightness, TNumBrightnesses> mBrightnesses;
    std::array<tColor, TNumColors> mColors;
  };

  struct cAlarm {
    uint16_t mID;
    bool mActive;
    uint8_t mMin;
    uint8_t mHour;
    uint8_t mDays;
    int8_t mSceneIndex;
    float mDuration;
    bool operator == (const cAlarm& aOther) const {
      return mID == aOther.mID
          && mActive == aOther.mActive
          && mMin == aOther.mMin
          && mHour == aOther.mHour
          && mDays == aOther.mDays
          && mSceneIndex == aOther.mSceneIndex
          && mDuration == aOther.mDuration;
    }
    bool operator != (const cAlarm& aOther) const {
      return mID != aOther.mID
          || mActive != aOther.mActive
          || mMin != aOther.mMin
          || mHour != aOther.mHour
          || mDays != aOther.mDays
          || mSceneIndex != aOther.mSceneIndex
          || mDuration != aOther.mDuration;
    }
  };

  typedef ggValueEEPromT<cAlarm> tAlarm;

  ggDataEEPromT()
  : mName(),
    mOn(false),
    mScenes(),
    mNumScenes(1),
    mCurrentSceneIndex(0),
    mNumAlarms(0)
  {
  }

  inline tString& Name() {
    return mName;
  }

  inline const tString& Name() const {
    return mName;
  }

  inline ggValueEEPromT<bool>& On() {
    return mOn;
  }

  inline const ggValueEEPromT<bool>& On() const {
    return mOn;
  }

  inline ggValueEEPromT<float>& TransitionTime() {
    return mTransitionTime;
  }

  inline const ggValueEEPromT<float>& TransitionTime() const {
    return mTransitionTime;
  }

  inline cScene& CurrentScene() {
    return mScenes[mCurrentSceneIndex];
  }

  inline const cScene& CurrentScene() const {
    return mScenes[mCurrentSceneIndex];
  }

  inline cScene& Scene(uint16_t aIndex) {
    return aIndex < mNumScenes ? mScenes[aIndex] : mScenes[mCurrentSceneIndex];
  }

  inline const cScene& Scene(uint16_t aIndex) const {
    return aIndex < mNumScenes ? mScenes[aIndex] : mScenes[mCurrentSceneIndex];
  }

  inline uint16_t GetNumScenes() const {
    return mNumScenes;
  }

  void SetNumScenes(uint16_t aNumScenes) {
    if (aNumScenes > 1) {
      while (mNumScenes < aNumScenes) AddScene();
      while (mNumScenes > aNumScenes) RemoveScene(mNumScenes - 1);
    }
  }

  inline uint16_t GetNumScenesMax() const {
    return TNumScenesMax;
  }

  inline uint16_t GetCurrentSceneIndex() const {
    return mCurrentSceneIndex;
  }

  inline void SetCurrentSceneIndex(uint16_t aIndex) {
    if (aIndex < mNumScenes) mCurrentSceneIndex = aIndex;
  }

  bool AddScene(bool aCopyCurrent = true) {
    GG_DEBUG();
    if (mNumScenes < TNumScenesMax) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      mNumScenes += 1;
      GG_DEBUG_PRINTF("mNumScenes = %d\n", mNumScenes.Get());
      if (aCopyCurrent) {
        GG_DEBUG_PRINTF("Copy current\n");
        mScenes[mNumScenes - 1] = CurrentScene();
      }
      return true;
    }
    return false;
  }

  bool RemoveScene(uint16_t aIndex) {
    GG_DEBUG();
    if ((0 < aIndex) && (aIndex < mNumScenes)) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      mNumScenes -= 1;
      if (mCurrentSceneIndex == aIndex) mCurrentSceneIndex -= 1;
      for (uint16_t vIndex = aIndex; vIndex < mNumScenes; vIndex++) {
        mScenes[vIndex] = mScenes[vIndex + 1];
      }
      return true;
    }
    return false;
  }

  String ScenesToJson() const {
    String vScenesJson = "\"mScenes\":[\n"; // scenes begin
    for (uint16_t vIndex = 0; vIndex < mNumScenes; vIndex++) {
      const cScene& vScene = mScenes[vIndex];
      vScenesJson += "  {\n"; // scene begin
      vScenesJson += String("    \"mName\":\"") + vScene.mName.Get() + "\",\n";
      vScenesJson += String("    \"mBrightnesses\":["); // brightnesses begin
      for (int vBrightnessIndex = 0; vBrightnessIndex < TNumBrightnesses; vBrightnessIndex++) {
        const TBrightness& vBrightness = vScene.mBrightnesses[vBrightnessIndex].Get();
        vScenesJson += ::ToJson(vBrightness);
        if (vBrightnessIndex + 1 < TNumBrightnesses) vScenesJson += ",";
      }
      vScenesJson += "],\n"; // brightnesses end
      vScenesJson += String("    \"mColors\":["); // colors begin
      for (int vColorIndex = 0; vColorIndex < TNumColors; vColorIndex++) {
        const TColor& vColor = vScene.mColors[vColorIndex].Get();
        vScenesJson += ::ToJson(vColor);
        if (vColorIndex + 1 < TNumColors) vScenesJson += ",";
      }
      vScenesJson += "]\n"; // colors end
      vScenesJson += vIndex + 1 < mNumScenes ? "  },\n" : "  }\n"; // scene end
    }
    vScenesJson += "]"; // scenes end
    return vScenesJson;
  }

  String SceneNamesToJson() const {
    String vSceneNamesJson = "\"mSceneNames\":[\n";
    vSceneNamesJson += "  {\"mIndex\":-1,\"mName\":\"Power Off\"},\n";
    for (uint16_t vIndex = 0; vIndex < mNumScenes; vIndex++) {
      vSceneNamesJson += "  {\"mIndex\":" + String(vIndex) + ",\"mName\":\"" + mScenes[vIndex].mName.Get() + "\"}";
      vSceneNamesJson += vIndex + 1 < mNumScenes ? ",\n" : "\n";
    }
    vSceneNamesJson += "]";
    return vSceneNamesJson;
  }

  inline uint16_t GetNumAlarms() const {
    return mNumAlarms;
  }

  inline uint16_t GetNumAlarmsMax() const {
    return TNumAlarmsMax;
  }

  bool AddAlarm() {
    GG_DEBUG();
    if (mNumAlarms < TNumAlarmsMax) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      uint16_t vID = GetNewAlarmID();
      cAlarm vAlarm;
      vAlarm.mID = vID;
      vAlarm.mActive = false;
      vAlarm.mMin = 59;
      vAlarm.mHour = 23;
      vAlarm.mDays = 0;
      vAlarm.mSceneIndex = 0;
      vAlarm.mDuration = 1.0f;
      mAlarms[mNumAlarms] = vAlarm;
      mAlarmIDs.insert(vID);
      mNumAlarms += 1;
      GG_DEBUG_PRINTF("vID = %d\n", vID);
      GG_DEBUG_PRINTF("mNumAlarms = %d\n", mNumAlarms.Get());
      return true;
    }
    return false;
  }

  bool RemoveAlarm(uint16_t aIndex) {
    GG_DEBUG();
    if ((0 <= aIndex) && (aIndex < mNumAlarms)) {
      ggValueEEProm::cLazyWriter vLazyWriter;
      mNumAlarms -= 1;
      mAlarmIDs.erase(mAlarms[aIndex].Get().mID);
      for (uint16_t vIndex = aIndex; vIndex < mNumAlarms; vIndex++) {
        mAlarms[vIndex] = mAlarms[vIndex + 1];
      }
      GG_DEBUG_PRINTF("aIndex = %d\n", aIndex);
      GG_DEBUG_PRINTF("mNumAlarms = %d\n", mNumAlarms.Get());
      return true;
    }
    return false;
  }

  uint16_t GetAlarmIndex(uint16_t aID) const {
    CheckAlarmIDs();
    if (AlarmExists(aID)) {
      for (uint16_t vIndex = 0; vIndex < mNumAlarms; vIndex++) {
        if (mAlarms[vIndex].Get().mID == aID) return vIndex;
      }
    }
    return -1;
  }

  bool RemoveAlarmID(uint16_t aID) {
    return RemoveAlarm(GetAlarmIndex(aID));
  }

  inline tAlarm& Alarm(uint16_t aIndex) {
    return aIndex < mNumAlarms ? mAlarms[aIndex] : mAlarms[0];
  }

  inline const tAlarm& Alarm(uint16_t aIndex) const {
    return aIndex < mNumAlarms ? mAlarms[aIndex] : mAlarms[0];
  }

  String AlarmsToJson() const {
    String vAlarmsJson = "\"mAlarms\":[\n";
    for (uint16_t vIndex = 0; vIndex < mNumAlarms; vIndex++) {
      const cAlarm& vAlarm = mAlarms[vIndex].Get();
      vAlarmsJson += "  {";
      vAlarmsJson += String("\"mID\":") + vAlarm.mID + ",";
      vAlarmsJson += String("\"mActive\":") + (vAlarm.mActive ? "true" : "false") + ",";
      vAlarmsJson += String("\"mMin\":") + vAlarm.mMin + ",";
      vAlarmsJson += String("\"mHour\":") + vAlarm.mHour + ",";
      vAlarmsJson += String("\"mDays\":") + vAlarm.mDays + ",";
      vAlarmsJson += String("\"mSceneIndex\":") + vAlarm.mSceneIndex + ",";
      vAlarmsJson += String("\"mDuration\":") + vAlarm.mDuration;
      vAlarmsJson += vIndex + 1 < mNumAlarms ? "},\n" : "}\n";
    }
    vAlarmsJson += "]";
    return vAlarmsJson;
  }

  String ToJson() const {
    String vJson = "{\n";
    vJson += String() + "\"mName\":\"" + mName.Get() + "\",\n";
    vJson += String() + "\"mOn\":" + (mOn.Get() ? "true" : "false") + ",\n";
    vJson += String() + "\"mTransitionTime\":" + mTransitionTime.Get() + ",\n";
    vJson += String() + "\"mCurrentSceneIndex\":" + mCurrentSceneIndex.Get() + ",\n";
    vJson += ScenesToJson() + ",\n";
    vJson += AlarmsToJson() + "\n";
    vJson += "}\n";
    return vJson;
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggDataEEPromT", aName);
    vDebug.PrintF("TStringLength = %d\n", TStringLength);
    vDebug.PrintF("TNumScenesMax = %d\n", TNumScenesMax);
    vDebug.PrintF("TNumBrightnesses = %d\n", TNumBrightnesses);
    vDebug.PrintF("TNumColors = %d\n", TNumColors);
    vDebug.PrintF("TNumAlarmsMax = %d\n", TNumAlarmsMax);
    vDebug.PrintF("mNumScenes = %d\n", mNumScenes.Get());
    vDebug.PrintF("mNumAlarms = %d\n", mNumAlarms.Get());
    vDebug.PrintF("ToJson() => %s\n", ToJson().c_str());
  }

private:

  void CheckAlarmIDs() const {
    if (mAlarmIDs.size() != mNumAlarms) {
      mAlarmIDs.clear();
      for (uint16_t vIndex = 0; vIndex < mNumAlarms; vIndex++) {
        mAlarmIDs.insert(mAlarms[vIndex].Get().mID);
      }
    }
  }

  bool AlarmExists(uint16_t aID) const {
    return mAlarmIDs.find(aID) != mAlarmIDs.end();
  }

  uint16_t GetNewAlarmID() const {
    uint16_t vID = 0;
    CheckAlarmIDs();
    while (AlarmExists(vID)) vID++;
    return vID;
  }

  tString mName;
  ggValueEEPromT<bool> mOn;
  ggValueEEPromT<float> mTransitionTime;

  ggValueEEPromT<uint16_t> mNumScenes;
  ggValueEEPromT<uint16_t> mCurrentSceneIndex;
  std::array<cScene, TNumScenesMax> mScenes;

  ggValueEEPromT<uint16_t> mNumAlarms;
  std::array<tAlarm, TNumAlarmsMax> mAlarms;
  mutable std::set<uint16_t> mAlarmIDs;

};
