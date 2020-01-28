#pragma once

#include <Arduino.h>
#include <FS.h>

/**
 * Saves binary data blocks circular buffered in a file.
 */
template <class TData>
class ggCircularFileT {

public:

  ggCircularFileT(const String& aFileName,
                  unsigned long aNumberOfDataBlocks,
                  FS* aFileSystem)
  : mFileName(aFileName),
    mNumberOfDataBlocks(aNumberOfDataBlocks),
    mFileSystem(aFileSystem) {
  }

  void Write(const TData& aData) const {
    File vFile;
    Initialize(vFile);
    if (vFile) {
      uint32_t vIndex = ReadIndex(vFile);
      WriteIndex(vFile, (vIndex + 1) % mNumberOfDataBlocks);
      Write(vFile, vIndex, aData);
      vFile.flush();
      vFile.close();
    }
  }

private:

  FS& FileSystem() const {
    return *mFileSystem;
  }

  size_t GetFileSize() const {
    return sizeof(uint32_t) + mNumberOfDataBlocks * sizeof(TData);
  }

  void Reset(File& aFile) const {
    aFile.seek(0);
    const size_t vFileSize = GetFileSize();
    size_t vIndex = vFileSize;
    while (vIndex--) aFile.write(0);
    aFile.truncate(vFileSize);
  }

  void Initialize(File& aFile) const {
    // if file is not open, it needs to be opened or created    
    if (!aFile) {
      // open an existing file for update
      aFile = FileSystem().open(mFileName.c_str(), "r+b");
      // if file can't be opened, it probably doesn't exist
      if (!aFile) {
        // create an empty file for output operations
        aFile = FileSystem().open(mFileName.c_str(), "w");
        // close it and re-open in update mode
        if (aFile) {
          Reset(aFile);
          aFile.flush();
          aFile.close();
          // for later operations, the file must be opened for update
          aFile = FileSystem().open(mFileName.c_str(), "r+b");
        }
      }
    }
    // hopefully there is now a file open for update
    if (aFile) {
      // if the filesize does not match, we're resetting the file
      const size_t vFileSize = GetFileSize();
      if (aFile.size() != vFileSize) {
        Reset(aFile);
      }
    }
  }

  static uint32_t ReadIndex(File& aFile) {
    uint32_t vIndex = 0;
    if (aFile.seek(0)) {
      Read(aFile, vIndex);
    }
    return vIndex;
  }

  static void WriteIndex(File& aFile, uint32_t aIndex) {
    if (aFile.seek(0)) {
      Write(aFile, aIndex);
    }
  }

  template <typename T>
  static bool Read(File& aFile, T& aData) {
    T vData = aData;
    if (aFile.read(reinterpret_cast<uint8_t*>(&vData), sizeof(T)) == sizeof(T)) {
      aData = vData;
      return true;
    }
    return false;
  }

  template <typename T>
  static void Write(File& aFile, const T& aData) {
    size_t vNumWritten = aFile.write(reinterpret_cast<uint8_t*>(const_cast<T*>(&aData)), sizeof(T));
  }

  static void Write(File& aFile, uint32_t aIndex, const TData& aData) {
    size_t vPosition = sizeof(uint32_t) + aIndex * sizeof(TData);
    if (aFile.seek(vPosition)) {
      Write(aFile, aData);
    }
  }

  const String mFileName;
  const unsigned long mNumberOfDataBlocks;
  FS* mFileSystem;

};
