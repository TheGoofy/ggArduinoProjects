#pragma once

#include <vector>
#include <algorithm>

/**
 * parses nun-nested function expression
 */
class ggFunctionParser {

public:

  ggFunctionParser(const String& aText) {
    Parse(aText);
  }

  const String& GetName() const {
    return mName;
  }

  int NumArgs() const {
    return mArgs.size();
  }

  const String& Arg(int aIndex) const {
    return mArgs[aIndex];
  }

  long ArgInt(int aIndex) const {
    return Arg(aIndex).toInt();
  }

  float ArgFloat(int aIndex) const {
    return Arg(aIndex).toFloat();
  }

  void Print(Stream& aStream) const {
    int vIndex = 0;
    aStream.printf("mName = %s\n", mName.c_str());
    std::for_each(mArgs.begin(), mArgs.end(), [&] (const String& aArg) {
      aStream.printf("mArgs[%d] = %s\n", vIndex++, aArg.c_str());
    });
  }

private:

  void Parse(const String& aText) {
    int vPosA = 0;
    int vPosB = aText.indexOf("(", vPosA);
    if (vPosB != -1) {
      mName = aText.substring(vPosA, vPosB);
      mName.trim();
      vPosA = vPosB + 1; // go to position right after "("
      vPosB = aText.indexOf(",", vPosA);
      while (vPosB != -1) {
        String vArg = aText.substring(vPosA, vPosB);
        vArg.trim();
        mArgs.push_back(vArg);
        vPosA = vPosB + 1; // go to position right after ","
        vPosB = aText.indexOf(",", vPosA);
      }
      vPosB = aText.indexOf(")", vPosA);
      if (vPosB != -1) {
        String vArg = aText.substring(vPosA, vPosB);
        vArg.trim();
        mArgs.push_back(vArg);
      }
    }
  }

  String mName;
  std::vector<String> mArgs;

};

