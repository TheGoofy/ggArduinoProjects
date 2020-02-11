#pragma once

#include <Arduino.h>
#include <FS.h>

/**
 * Saves binary data blocks circular buffered in a file.
 */
template <class TTime, class TData>
class ggCircularFileT {

public:

  ggCircularFileT(const String& aFileName,
                  uint32_t aNumberOfDataBlocks,
                  FS* aFileSystem)
  : mFileSystem(aFileSystem),
    mFileName(aFileName),
    mNumberOfDataBlocks(aNumberOfDataBlocks),
    mIndex(aNumberOfDataBlocks) {
  }

  const String& GetFileName() const {
    return mFileName;
  }

  uint32_t GetNumberOfDataBlocks() const {
    return mNumberOfDataBlocks;
  }

  // "aTime" must have an incremental value
  bool Write(const TTime& aTime, const TData& aData) {
    bool vSuccess = false;
    if (OpenFile()) {
      vSuccess = true;
      vSuccess &= GetIndex();
      vSuccess &= WriteData(aTime, aData);
      CloseFile();
    }
    return vSuccess;
  }

  // reads the newest data block
  bool Read(TTime& aTime, TData& aData) const {
    bool vSuccess = false;
    if (OpenFile()) {
      vSuccess = true;
      vSuccess &= GetIndex();
      vSuccess &= ReadData(aTime, aData);
      CloseFile();
      return true;
    }
    return vSuccess;
  }

  // ultimately clean up
  void Reset() {
    mFile.close();
    FileSystem().remove(mFileName.c_str());
    mIndex = 0;
  }

private:

  FS& FileSystem() const {
    return *mFileSystem;
  }

  size_t GetFileSize() const {
    return mNumberOfDataBlocks * (sizeof(TTime) + sizeof(TData));
  }

  size_t GetPosition(uint32_t aIndex) const {
    return aIndex * (sizeof(TTime) + sizeof(TData));
  }

  // allocates a complete new file and opens it for update
  void ResetAndOpenFile() {
    // ultimately clean up
    Reset();
    // create a new empty file for writing
    mFile = FileSystem().open(mFileName.c_str(), "w");
    if (mFile) {
      // initialize all bytes with zero
      size_t vPosition = GetFileSize();;
      while (vPosition--) mFile.write(0);
      mFile.flush();
      mFile.close();
      // for later operations, the file must be opened for update
      mFile = FileSystem().open(mFileName.c_str(), "r+b");
      // the file is empty - no need to "search" index in file
      mIndex = 0;
    }
    else {
      GG_DEBUG();
      vDebug.PrintF("failed to reset file \"%s\"\n", mFileName.c_str());
    }
  }

  bool OpenFile() const {
    // open an existing file for update
    mFile = FileSystem().open(mFileName.c_str(), "r+b");
    // if file can't be opened (because it does not yet exists), or
    // if the filesize does not match, we're resetting the file
    if (!mFile || ((mFile.size() != GetFileSize()))) {
      const_cast<ggCircularFileT&>(*this).ResetAndOpenFile();
    }
    // hopefully there is now a file open for update
    if (!mFile) {
      GG_DEBUG();
      vDebug.PrintF("failed to open file \"%s\"\n", mFileName.c_str());
    }
    return mFile;
  }

  bool GetIndex() const {
    // index is valid, if smaller than number of blocks
    if (mIndex < mNumberOfDataBlocks) return true;
    // invalid index => need to search in file
    bool vSuccess = true;
    mIndex = 0;
    TTime vTime = 0;
    TTime vLastTime = 0;
    // search whole file when "time" jumps back
    while (vSuccess && (mIndex < mNumberOfDataBlocks)) {
      size_t vPosition = GetPosition(mIndex);
      if ((vSuccess = mFile.seek(vPosition)) == true) {
        if (Read(mFile, vTime)) {
          // found free (unused) block
          if (vTime == 0) return true;
          // found wrap-around block (ring closed)
          if (vTime < vLastTime) return true;
          // remember current time for next check
          vLastTime = vTime;
        }
      }
      // iterate next
      ++mIndex;
    }
    // last block was newest
    mIndex = 0;
    return vSuccess;
  }

  bool WriteData(const TTime& aTime, const TData& aData) {
    bool vSuccess = false;
    size_t vPosition = GetPosition(mIndex++);
    if (mIndex >= mNumberOfDataBlocks) mIndex = 0;
    if (mFile.seek(vPosition)) {
      vSuccess = true;
      vSuccess &= Write(mFile, aTime);
      vSuccess &= Write(mFile, aData);
    }
    return vSuccess;
  }

  bool ReadData(TTime& aTime, TData& aData) const {
    bool vSuccess = false;
    uint32_t vIndex = mIndex > 0 ? mIndex - 1 : mNumberOfDataBlocks - 1;
    size_t vPosition = GetPosition(vIndex);
    if (mFile.seek(vPosition)) {
      vSuccess = true;
      vSuccess &= Read(mFile, aTime);
      vSuccess &= Read(mFile, aData);
    }
    return vSuccess;
  }

  void CloseFile() const {
    mFile.flush();
    mFile.close();
  }

  template <typename T>
  static bool Read(File& aFile, T& aData) {
    return aFile.read(reinterpret_cast<uint8_t*>(&aData), sizeof(T)) == sizeof(T);
  }

  template <typename T>
  static bool Write(File& aFile, const T& aData) {
    return aFile.write(reinterpret_cast<uint8_t*>(const_cast<T*>(&aData)), sizeof(T)) == sizeof(T);
  }

  FS* mFileSystem;

  const String mFileName;
  const uint32_t mNumberOfDataBlocks;

  mutable File mFile;
  mutable uint32_t mIndex;

};
