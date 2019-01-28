#pragma once

namespace ggConfig {

  String GetChipId() {
    return String(ESP.getChipId(), HEX);
  }

  void Print(Stream& aStream) {
    // basic chip info
    aStream.printf("ESP.getChipId() = 0x%08X\n", ESP.getChipId());
    aStream.printf("ESP.getCoreVersion() = %s\n", ESP.getCoreVersion().c_str());
    aStream.printf("ESP.getFreeHeap() = %d\n", ESP.getFreeHeap());
    // aStream.printf("ESP.getHeapFragmentation() = %d%\n", ESP.getHeapFragmentation());
    // aStream.printf("ESP.getMaxFreeBlockSize() = %d\n", ESP.getMaxFreeBlockSize());
    aStream.println();
    // flash info
    FlashMode_t ideMode = ESP.getFlashChipMode();
    aStream.printf("ESP.getFlashChipId() = 0x%08X\n", ESP.getFlashChipId());
    aStream.printf("ESP.getFlashChipRealSize() = %u bytes\n", ESP.getFlashChipRealSize());
    aStream.printf("ESP.getFlashChipSize() = %u bytes\n", ESP.getFlashChipSize());
    aStream.printf("ESP.getFlashChipSpeed() = %u Hz\n", ESP.getFlashChipSpeed());
    aStream.printf("ESP.getFlashChipMode() = %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    aStream.println();
  }

}

