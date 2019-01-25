#ifndef __GG_TOOLS__
#define __GG_TOOLS__

unsigned long ggGetLargestFreeMemoryBlockSize()
{
  unsigned long vSize = 0xFFFFFFFF;
  unsigned long vSizeDelta = vSize / 2;
  byte* vData = 0;
  while (vSizeDelta != 0) {
    vData = new byte[vSize];
    if (vData != 0) {
      delete[] vData;
      vData = 0;
      vSize += vSizeDelta;
    }
    else {
      vSize -= vSizeDelta;
    }
    vSizeDelta /= 2;
  }
  return vSize;
}

#endif // __GG_TOOLS__
