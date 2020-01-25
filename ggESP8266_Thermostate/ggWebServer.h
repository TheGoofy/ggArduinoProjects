#pragma once

#include <FS.h>

#include "ggWebServerHtmlData.h"
#include "ggStringStream.h"
#include "ggDebug.h"

class ggWebServer {

public:

  ggWebServer(int aPort, FS* aFileSystem)
  : mServer(aPort),
    mDebugStreamFunc(nullptr),
    mResetFunc(nullptr),
    mRebootFunc(nullptr),
    mFileSystem(aFileSystem) {
  }

  void Begin() {
    mServer.on("/", [&] () { OnHome(); });
    mServer.on("/home", [&] () { OnHome(); });
    mServer.on("/logger", [&] () { HandleFile("/ggLogger.html"); });
    mServer.on("/files", [&] () { OnFS(); });
    mServer.on("/debug", [&] () { OnDebug(); });
    mServer.on("/goofy", [&] () { HandleFile("/ggGoofy.html"); });
    mServer.on("/reset", [&] () { OnReset(); });
    mServer.on("/reboot", [&] () { OnReboot(); });
    mServer.onNotFound([&] () { OnNotFound(); });
    mServer.begin();
    FileSystem().begin();
  }

  void Run() {
    mServer.handleClient();
  }

  typedef std::function<void()> tFunc;
  typedef std::function<void(Stream&)> tStreamFunc;

  void OnDebugStream(tStreamFunc aStreamFunc) {
    mDebugStreamFunc = aStreamFunc;
  }
  
  void OnReset(tFunc aResetFunc) {
    mResetFunc = aResetFunc;
  }

  void OnReboot(tFunc aRebootFunc) {
    mRebootFunc = aRebootFunc;
  }

private:

  static String GetContentType(const String& aFileName) {
    GG_DEBUG();
    if (aFileName.endsWith(".htm")) return "text/html";
    if (aFileName.endsWith(".html")) return "text/html";
    if (aFileName.endsWith(".css")) return "text/css";
    if (aFileName.endsWith(".js")) return "application/javascript";
    if (aFileName.endsWith(".png")) return "image/png";
    if (aFileName.endsWith(".gif")) return "image/gif";
    if (aFileName.endsWith(".jpg")) return "image/jpeg";
    if (aFileName.endsWith(".ico")) return "image/x-icon";
    if (aFileName.endsWith(".xml")) return "text/xml";
    if (aFileName.endsWith(".pdf")) return "application/x-pdf";
    if (aFileName.endsWith(".zip")) return "application/x-zip";
    if (aFileName.endsWith(".gz")) return "application/x-gzip";
    if (aFileName.endsWith(".dat")) return "application/x-binary";
    if (aFileName.endsWith(".bin")) return "application/x-binary";
    return "text/plain";
  }

  bool HandleFile(const String& aFileName) {
    GG_DEBUG();
    vDebug.PrintF("aFileName = %s\n", aFileName.c_str());
    if (FileSystem().exists(aFileName)) {
      vDebug.PrintF("file exists\n");
      File vFile = FileSystem().open(aFileName, "r");
      if (vFile) {
        vDebug.PrintF("file opened\n");
        const String vContentType = GetContentType(aFileName);
        size_t sent = mServer.streamFile(vFile, vContentType);
        vFile.close();
        return true;
      }
    }
    return false;
  }

  void OnNotFound() {
    if (HandleFile(mServer.uri())) return;
    mServer.send(404, "text/plain", "url not found");
  }

  void OnHome() {
    GG_DEBUG();
    vDebug.PrintF("local IP = %s\n", mServer.client().localIP().toString().c_str());
    vDebug.PrintF("remote IP = %s\n", mServer.client().remoteIP().toString().c_str());
    HandleFile("/ggIndex.html");
  }

  void OnFS() {
    size_t vNumberOfFiles = 0;
    size_t vTotalFileSize = 0;
    String vContent = "<script>document.title = 'ESP8266 Thermostate SPIFFS';</script>\n";
    vContent += "<b style='font-size:larger'>SPIFFS</b><br>\n<hr noshade>\n";
    Dir vDir = SPIFFS.openDir("");
    while (vDir.next()) {
      vNumberOfFiles++;
      vTotalFileSize += vDir.fileSize();
      vContent += "<a href='" + vDir.fileName() + "'>" + vDir.fileName()+ "</a> - " + vDir.fileSize() + " bytes<br>\n";
    }
    vContent += String("Total ") + vNumberOfFiles + " files - " + vTotalFileSize + " bytes<br>\n";
    vContent += "<hr noshade>\n";
    FSInfo vFSInfo;
    FileSystem().info(vFSInfo);
    vContent += String("FSInfo.totalBytes = ") + vFSInfo.totalBytes + " bytes<br>\n";
    vContent += String("FSInfo.usedBytes = ") + vFSInfo.usedBytes + " bytes<br>\n";
    vContent += String("FSInfo.blockSize = ") + vFSInfo.blockSize + " bytes<br>\n";
    vContent += String("FSInfo.pageSize = ") + vFSInfo.pageSize + " bytes<br>\n";
    vContent += String("FSInfo.maxOpenFiles = ") + vFSInfo.maxOpenFiles + "<br>\n";
    vContent += String("FSInfo.maxPathLength = ") + vFSInfo.maxPathLength + "<br>\n";
    float vPercentUsed = (float)vFSInfo.usedBytes / (float)vFSInfo.totalBytes;
    vContent += String("<progress style='width:100%' value='") + vPercentUsed + "' max='1'></progress><br>\n";
    SendContent(vContent);
  }

  void OnDebug() {
    String vContent = "<script>document.title = 'ESP8266 Thermostate Debug';</script>\n";
    vContent += "<b style='font-size:larger'>Debug</b><br>\n<hr noshade>\n";
    vContent += "<a href='reset'>[reset]</a>\n";
    vContent += "<a href='reboot'>[reboot]</a>\n";
    if (mDebugStreamFunc != nullptr) {
      vContent += "<hr noshade>\n";
      vContent += "<textarea readonly style='width:100%;min-height:450px;'>\n";
      ggStringStream vStream(vContent);
      mDebugStreamFunc(vStream);
      vContent += "</textarea>\n";
    }
    SendContent(vContent);
  }

  void OnReset() {
    if (mResetFunc != nullptr) mResetFunc();
    mServer.sendHeader("Location", String("/"), true);
    mServer.send(302, "text/plain", "");
  }

  void OnReboot() {
    mServer.sendHeader("Location", String("/"), true);
    mServer.send(302, "text/plain", "reboot...");
    delay(1000);
    if (mRebootFunc != nullptr) mRebootFunc();
  }

  void SendContent(const String& aHtmlContent) {
    unsigned long vContentLength = 0;
    vContentLength += strlen_P(mWebServerHtmlFront);
    vContentLength += aHtmlContent.length();
    vContentLength += strlen_P(mWebServerHtmlBack);
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

  tStreamFunc mDebugStreamFunc;
  tFunc mResetFunc;
  tFunc mRebootFunc;

  FS* mFileSystem;
};
