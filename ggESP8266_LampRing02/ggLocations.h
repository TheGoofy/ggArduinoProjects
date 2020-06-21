#pragma once

namespace ggLocations {

  enum tEnum {
    eNone = 0,
    eAL   = 0b00000001, // strip A, left  (or lower, or base, ...)
    eAR   = 0b00000010, // strip A, right (or upper, or top, ...)
    eBL   = 0b00000100, // strip B, left  (or lower, or base, ...)
    eBR   = 0b00001000, // strip B, right (or upper, or top, ...)
    eA    = eAL | eAR,  // strip A (L+R)
    eB    = eBL | eBR,  // strip B (L+R)
    eL    = eAL | eBL,  // left (strips A+B)
    eR    = eAR | eBR,  // right (strips A+B)
    eAll  = eA | eB
  };

  inline tEnum Intersect(tEnum aLocations1, tEnum aLocations2) {
    return (tEnum)((int)aLocations1 & (int)aLocations2);
  }

  inline tEnum FromIndex(int aIndex) {
    switch (aIndex) {
      case 0: return eAL;
      case 1: return eAR;
      case 2: return eBL;
      case 3: return eBR;
      default: return eAL;
    }
  }

  inline int ToIndex(tEnum aLocations) {
    switch (aLocations) {
      case eAL: return 0;
      case eAR: return 1;
      case eBL: return 2;
      case eBR: return 3;
      default: return -1;
    }
  }

  inline int ToIndex(tEnum aLocations, tEnum aLocationsToCheck) {
    return ToIndex(Intersect(aLocations, aLocationsToCheck));
  }

  typedef std::function<void(tEnum aLocation)> tLocationFunc;
  inline void ForEach(tLocationFunc aLocationFunc) {
    aLocationFunc(FromIndex(0));
    aLocationFunc(FromIndex(1));
    aLocationFunc(FromIndex(2));
    aLocationFunc(FromIndex(3));
  }

};
