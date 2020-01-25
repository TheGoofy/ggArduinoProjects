#pragma once

#include <Arduino.h>
#include <FS.h>

template <class TData>
class ggLoggerDataT {

public:

  ggLoggerDataT(const String& aFileName,
                unsigned long aMaxSizeBytes,
                FS* aFileSystem)
  : mFileName(aFileName),
    mIndexSize((aMaxSizeBytes - sizeof(uint32_t)) / sizeof(TData)),
    mFileSystem(aFileSystem) {
  }

  void Log(const TData& aData) const {
    GG_DEBUG();
    vDebug.PrintF("mFileName = %s\n", mFileName.c_str());
    vDebug.PrintF("mIndexSize = %d\n", mIndexSize);
    // open file for append/update "a+" and binary "b" 
    File vFile = FileSystem().open(mFileName.c_str(), "r+b");
    if (vFile) {
      Initialize(vFile);
      vDebug.PrintF("file opened\n");
      uint32_t vIndex = ReadIndex(vFile);
      vDebug.PrintF("vIndex = %d\n", vIndex);
      vIndex = (vIndex + 1) % mIndexSize;
      vDebug.PrintF("next vIndex = %d\n", vIndex);
      WriteIndex(vFile, vIndex);
      Write(vFile, vIndex, aData);
      vFile.flush();
      vFile.close();
    }
  }

private:

  FS& FileSystem() const {
    GG_DEBUG();
    vDebug.PrintF("mFileSystem = %d\n", mFileSystem);
    return *mFileSystem;
  }

  size_t GetFileSize() const {
    return sizeof(uint32_t) + mIndexSize * sizeof(TData);
  }

  void Initialize(File& aFile) const {
    GG_DEBUG();
    const size_t vFileSize = GetFileSize();
    if (aFile.size() != vFileSize) {
      vDebug.PrintF("resetting file\n");
      aFile.seek(0);
      size_t vIndex = vFileSize;
      while (vIndex--) aFile.write(0);
      aFile.truncate(vFileSize);
    }
  }

  static uint32_t ReadIndex(File& aFile) {
    GG_DEBUG();
    uint32_t vIndex = 0;
    if (aFile.seek(0)) {
      Read(aFile, vIndex);
    }
    return vIndex;
  }

  static void WriteIndex(File& aFile, uint32_t aIndex) {
    GG_DEBUG();
    vDebug.PrintF("aIndex = %d\n", aIndex);
    if (aFile.seek(0)) {
      Write(aFile, aIndex);
    }
  }

  template <typename T>
  static bool Read(File& aFile, T& aData) {
    GG_DEBUG();
    T vData = aData;
    if (aFile.read(reinterpret_cast<uint8_t*>(&vData), sizeof(T)) == sizeof(T)) {
      vDebug.PrintF("reading succeeded\n");
      aData = vData;
      return true;
    }
    vDebug.PrintF("reading failed\n");
    return false;
  }

  template <typename T>
  static void Write(File& aFile, const T& aData) {
    GG_DEBUG();
    size_t vNumWritten = aFile.write(reinterpret_cast<uint8_t*>(const_cast<T*>(&aData)), sizeof(T));
    vDebug.PrintF("sizeof(T) = %d\n", sizeof(T));
    vDebug.PrintF("vNumWritten = %d\n", vNumWritten);
  }

  static void Write(File& aFile, uint32_t aIndex, const TData& aData) {
    GG_DEBUG();
    size_t vPosition = sizeof(uint32_t) + aIndex * sizeof(TData);
    vDebug.PrintF("vPosition = %d\n", vPosition);
    if (aFile.seek(vPosition)) {
      Write(aFile, aData);
    }
  }

  const String mFileName;
  const unsigned long mIndexSize;
  FS* mFileSystem;

};
