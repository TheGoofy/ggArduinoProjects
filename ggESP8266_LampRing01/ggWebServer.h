#pragma once

#include <FS.h>
#include <set>

#include "ggWebServerHtmlData.h"
#include "ggStringStream.h"
#include "ggDebug.h"
#include "ggNumerics.h"

class ggWebServer {

public:

  ggWebServer(int aPort, FS* aFileSystem)
  : mServer(aPort),
    mAddAlarmFunc(nullptr),
    mDelAlarmFunc(nullptr),
    mGetAlarmsFunc(nullptr),
    mSetAlarmFunc(nullptr),
    mGetTimeFunc(nullptr),
    mDebugStreamFunc(nullptr),
    mResetAllFunc(nullptr),
    mRebootFunc(nullptr),
    mResetWifiFunc(nullptr),
    mFileSystem(aFileSystem) {
  }

  void Begin() {
    mServer.on("/", [&] () { OnHome(); });
    mServer.on("/home", [&] () { OnHome(); });
    mServer.on("/alarms", [&] () { OnAlarms(); });
    mServer.on("/AddAlarm", [&] () { OnAddAlarm(); });
    mServer.on("/DelAlarm", [&] () { OnDelAlarm(); });
    mServer.on("/GetAlarms", [&] () { OnGetAlarms(); });
    mServer.on("/SetAlarm", [&] () { OnSetAlarm(); });
    mServer.on("/time", [&] () { OnTime(); });
    mServer.on("/GetTime", [&] () { OnGetTime(); });
    mServer.on("/files", [&] () { OnFS(); });
    mServer.on("/debug", [&] () { OnDebug(); });
    mServer.on("/resetall", [&] () { OnResetAll(); });
    mServer.on("/resetwifi", [&] () { OnResetWifi(); });
    mServer.on("/reboot", [&] () { OnReboot(); });
    mServer.onNotFound([&] () { OnNotFound(); });
    mServer.begin();
  }

  void Run() {
    mServer.handleClient();
  }

  typedef std::function<void()> tFunc;
  typedef std::function<void(Stream&)> tStreamFunc;

  typedef std::function<bool()> tAddAlarmFunc;
  void OnAddAlarm(tAddAlarmFunc aAddAlarmFunc) {
    mAddAlarmFunc = aAddAlarmFunc;
  }

  typedef std::function<bool(int)> tDelAlarmFunc;
  void OnDelAlarm(tDelAlarmFunc aDelAlarmFunc) {
    mDelAlarmFunc = aDelAlarmFunc;
  }

  typedef std::function<String()> tGetAlarmsFunc;
  void OnGetAlarms(tGetAlarmsFunc aGetAlarmsFunc) {
    mGetAlarmsFunc = aGetAlarmsFunc;
  }

  typedef std::function<bool(const String&)> tSetAlarmFunc;
  void OnSetAlarm(tSetAlarmFunc aSetAlarmFunc) {
    mSetAlarmFunc = aSetAlarmFunc;
  }

  typedef std::function<String()> tGetTimeFunc;
  void OnGetTime(tGetTimeFunc aGetTimeFunc) {
    mGetTimeFunc = aGetTimeFunc;
  }

  void OnDebugStream(tStreamFunc aStreamFunc) {
    mDebugStreamFunc = aStreamFunc;
  }
  
  void OnResetAll(tFunc aResetAllFunc) {
    mResetAllFunc = aResetAllFunc;
  }

  void OnResetWifi(tFunc aResetWifiFunc) {
    mResetWifiFunc = aResetWifiFunc;
  }

  void OnReboot(tFunc aRebootFunc) {
    mRebootFunc = aRebootFunc;
  }

private:

  static const char* GetContentType(const String& aFileName) {
    if (aFileName.endsWith(".htm")) return "text/html";
    if (aFileName.endsWith(".html")) return "text/html";
    if (aFileName.endsWith(".css")) return "text/css";
    if (aFileName.endsWith(".js")) return "application/javascript";
    if (aFileName.endsWith(".png")) return "image/png";
    if (aFileName.endsWith(".gif")) return "image/gif";
    if (aFileName.endsWith(".jpg")) return "image/jpeg";
    if (aFileName.endsWith(".ico")) return "image/x-icon";
    if (aFileName.endsWith(".xml")) return "text/xml";
    if (aFileName.endsWith(".json")) return "application/json";
    if (aFileName.endsWith(".pdf")) return "application/x-pdf";
    if (aFileName.endsWith(".zip")) return "application/x-zip";
    if (aFileName.endsWith(".gz")) return "application/x-gzip";
    if (aFileName.endsWith(".dat")) return "application/x-binary";
    if (aFileName.endsWith(".bin")) return "application/x-binary";
    return "text/plain";
  }

  bool HandleFile(const String& aFileName) {
    GG_DEBUG();
    if (FileSystem().exists(aFileName)) {
      File vFile = FileSystem().open(aFileName, "r+b");
      if (vFile) {
        unsigned long vMicrosStart = micros();
        mServer.sendHeader("Access-Control-Allow-Origin", "*");
        const char* vContentType = GetContentType(aFileName);
        size_t vBytesSent = mServer.streamFile(vFile, vContentType);
        unsigned long vMicrosEnd = micros();
        float vMilliSeconds = (vMicrosEnd - vMicrosStart) / 1000.0f;
        GG_DEBUG_PRINTF("file \"%s\" transferred in %lf ms\n", aFileName.c_str(), vMilliSeconds);
        vFile.close();
        return true;
      }
    }
    GG_DEBUG_PRINTF("file \"%s\" not found\n", aFileName.c_str());
    return false;
  }

  void OnNotFound() {
    if (HandleFile(mServer.uri())) return;
    mServer.send(404, "text/plain", "url not found");
  }

  void OnHome() {
    GG_DEBUG();
    GG_DEBUG_PRINTF("local IP = %s\n", mServer.client().localIP().toString().c_str());
    GG_DEBUG_PRINTF("remote IP = %s\n", mServer.client().remoteIP().toString().c_str());
    HandleFile("/ggIndex.html");
  }

  void OnAlarms() {
    GG_DEBUG();
    HandleFile("/ggAlarms.html");
  }

  void OnAddAlarm() {
    GG_DEBUG();
    String vResponseText = "FAIL";
    if (mServer.method() == HTTP_POST) {
      if (mAddAlarmFunc != nullptr) {
        if (mAddAlarmFunc()) {
          vResponseText = "OK";
        }
      }
    }
    mServer.sendHeader("Access-Control-Allow-Origin", "*");
    mServer.send(200, "text/plain", vResponseText);
  }

  void OnDelAlarm() {
    GG_DEBUG();
    String vResponseText = "FAIL";
    if (mServer.method() == HTTP_POST) {
      String vAlarmID = mServer.arg("aAlarmID");
      if (vAlarmID != "" && mDelAlarmFunc != nullptr) {
        if (mDelAlarmFunc(vAlarmID.toInt())) {
          vResponseText = "OK";
        }
      }
    }
    mServer.sendHeader("Access-Control-Allow-Origin", "*");
    mServer.send(200, "text/plain", vResponseText);
  }

  void OnGetAlarms() {
    GG_DEBUG();
    if (mGetAlarmsFunc != nullptr) {
      if (mServer.method() == HTTP_GET) {
        mServer.sendHeader("Access-Control-Allow-Origin", "*");
        mServer.send(200, GetContentType(".json"), mGetAlarmsFunc());
        return;
      }
    }
    OnNotFound();
  }

  void OnSetAlarm() {
    GG_DEBUG();
    String vResponseText = "FAIL";
    if (mServer.method() == HTTP_POST) {
      String vAlarmJson = mServer.arg("aAlarmJson");
      if (vAlarmJson != "" && mSetAlarmFunc != nullptr) {
        if (mSetAlarmFunc(vAlarmJson)) {
          vResponseText = "OK";
        }
      }
    }
    mServer.sendHeader("Access-Control-Allow-Origin", "*");
    mServer.send(200, "text/plain", vResponseText);
  }

  void OnTime() {
    GG_DEBUG();
    HandleFile("/ggTime.html");
  }

  void OnGetTime() {
    GG_DEBUG();
    if (mGetTimeFunc != nullptr) {
      if (mServer.method() == HTTP_GET) {
        mServer.sendHeader("Access-Control-Allow-Origin", "*");
        mServer.send(200, GetContentType(".json"), mGetTimeFunc());
        return;
      }
    }
    OnNotFound();
  }

  static String GetNiceByteSize(size_t aSize) {
    String vNumber(aSize);
    int vDigits = vNumber.length();
    String vNumberSep;
    for (int vIndex = 0; vIndex < vDigits; vIndex++) {
      vNumberSep += vNumber[vIndex];
      if ((vIndex + 1 < vDigits) && (vDigits - vIndex - 1) % 3 == 0) {
        vNumberSep += "'";
      }
    }
    return vNumberSep + " bytes";
  }

  struct cFile {
    cFile(const String& aName, size_t aSize)
    : mName(aName), mSize(aSize) {
    }
    String GetHTML() const {
      return "<a href='" + mName + "'>" + mName + "</a> - " + GetNiceByteSize(mSize) + "<br>\n";
    }
    inline bool operator < (const cFile& aOther) const {
      return mName < aOther.mName;
    }
    String mName;
    size_t mSize;
  };

  void OnFS() {
    size_t vTotalFileSize = 0;
    Dir vDir = FileSystem().openDir("");
    std::set<cFile> vFiles;
    while (vDir.next()) {
      vFiles.insert(cFile(vDir.fileName(), vDir.fileSize()));
      vTotalFileSize += vDir.fileSize();
    }
    String vContent = "<script>document.title = 'ESP8266 Lamp Files';</script>\n";
    vContent += "<b style='font-size:larger'>Files</b><br>\n<hr noshade>\n";
    std::for_each(vFiles.begin(), vFiles.end(), [&] (const cFile& aFile) {
      vContent += aFile.GetHTML();
    });
    vContent += String("Total ") + vFiles.size() + " files - " + GetNiceByteSize(vTotalFileSize) + "<br>\n";
    vContent += "<hr noshade>\n";
    FSInfo vFSInfo;
    FileSystem().info(vFSInfo);
    vContent += String("FSInfo.totalBytes = ") + GetNiceByteSize(vFSInfo.totalBytes) + "<br>\n";
    vContent += String("FSInfo.usedBytes = ") + GetNiceByteSize(vFSInfo.usedBytes) + "<br>\n";
    vContent += String("FSInfo.blockSize = ") + GetNiceByteSize(vFSInfo.blockSize) + "<br>\n";
    vContent += String("FSInfo.pageSize = ") + GetNiceByteSize(vFSInfo.pageSize) + "<br>\n";
    vContent += String("FSInfo.maxOpenFiles = ") + vFSInfo.maxOpenFiles + "<br>\n";
    vContent += String("FSInfo.maxPathLength = ") + vFSInfo.maxPathLength + "<br>\n";
    float vPercentUsed = (float)vFSInfo.usedBytes / (float)vFSInfo.totalBytes;
    vContent += String("<progress style='width:100%' value='") + vPercentUsed + "' max='1'></progress><br>\n";
    SendContent(vContent);
  }

  void OnDebug() {
    String vContent = "<script>document.title = 'ESP8266 Lamp Debug';</script>\n";
    vContent += "<b style='font-size:larger'>Debug</b><br>\n<hr noshade>\n";
    vContent += "<a href='resetall' onclick='return confirm(\"Reset ALL settings and log data?\")'>[reset all]</a>\n";
    vContent += "<a href='resetwifi' onclick='return confirm(\"Reset WiFi settings?\")'>[reset wifi]</a>\n";
    vContent += "<a href='reboot' onclick='return confirm(\"Reboot?\")'>[reboot]</a>\n";
    if (mDebugStreamFunc != nullptr) {
      vContent += "<hr noshade>\n";
      vContent += "<textarea readonly style='width:100%;min-height:450px;'>\n";
      ggStringStream vStream(vContent);
      mDebugStreamFunc(vStream);
      vContent += "</textarea>\n";
    }
    SendContent(vContent);
  }

  void OnResetAll() {
    if (mResetAllFunc != nullptr) {
      mServer.sendHeader("Location", String("/home"), true);
      mServer.send(307, "text/plain", "reset all...");
      mServer.client().stop();
      mResetAllFunc();
    }
  }

  void OnResetWifi() {
    if (mResetWifiFunc != nullptr) {
      mServer.sendHeader("Location", String("/home"), true);
      mServer.send(307, "text/plain", "reset wifi...");
      mServer.client().stop();
      mResetWifiFunc();
    }
  }

  void OnReboot() {
    if (mRebootFunc != nullptr) {
      mServer.sendHeader("Location", String("/home"), true);
      mServer.send(307, "text/plain", "reboot...");
      mServer.client().stop();
      mRebootFunc();
    }
  }

  void SendContent(const String& aHtmlContent) {
    unsigned long vContentLength = 0;
    vContentLength += strlen_P(mWebServerHtmlFront);
    vContentLength += aHtmlContent.length();
    vContentLength += strlen_P(mWebServerHtmlBack);
    mServer.sendHeader("Access-Control-Allow-Origin", "*");
    mServer.setContentLength(vContentLength);
    mServer.send(200, "text/html", "");
    mServer.sendContent_P(mWebServerHtmlFront);
    mServer.sendContent(aHtmlContent);
    mServer.sendContent_P(mWebServerHtmlBack);
    mServer.client().stop();
  }

  FS& FileSystem() {
    return *mFileSystem;
  }

  ESP8266WebServer mServer;

  tAddAlarmFunc mAddAlarmFunc;
  tDelAlarmFunc mDelAlarmFunc;
  tGetAlarmsFunc mGetAlarmsFunc;
  tSetAlarmFunc mSetAlarmFunc;

  tGetTimeFunc mGetTimeFunc;
  
  tStreamFunc mDebugStreamFunc;
  
  tFunc mResetAllFunc;
  tFunc mRebootFunc;
  tFunc mResetWifiFunc;

  FS* mFileSystem;
};
