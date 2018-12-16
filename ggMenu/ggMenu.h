#ifndef __GG_MENU__
#define __GG_MENU__

#include "Arduino.h"
#include "ggVector.h"

const char mClassID[] = "ggMenu";

class ggMenu
{
  
public:
  
  struct cItem;
  
  typedef const char* tStringPtr;
  typedef const tStringPtr* tStringPtrs;
  typedef const cItem* tItemPtr;
  typedef const tItemPtr* tItemPtrs;
  
  enum tItemType {
    eItemMenu,
    eItemByte,
    eItemInt,
    eItemLong,
    eItemFloat,
    eItemEnum,
    eItemAction
  };
  
  struct cMenu {
    tItemPtrs mItems;
    byte mSize;
  };
  
  template <typename TNumber>
  struct cNumberT {
    TNumber* mValue;
    TNumber mStep;
    TNumber mMin;
    TNumber mMax;
    byte mDecimals;
    tStringPtr mUnit;
  };
  
  typedef cNumberT<byte> tNumberByte;
  typedef cNumberT<int> tNumberInt;
  typedef cNumberT<long> tNumberLong;
  typedef cNumberT<float> tNumberFloat;
  
  struct cEnum {
    byte* mIndex;
    tStringPtrs mNames;
    byte mSize;
  };
  
  typedef void (*tAction)();
  
  union cValue {
    cMenu mMenu;
    tNumberByte mByte;
    tNumberInt mInt;
    tNumberInt mLong;
    tNumberFloat mFloat;
    cEnum mEnum;
    tAction mAction;
  };
  
  struct cItem {
    tItemType mType;
    tStringPtr mName;
    cValue mValue;
  };
  
  ggMenu(tItemPtr aItemRoot,
         const int aVersion = 1)
  : mItemRoot(aItemRoot),
    mEditing(false),
    mModified(false),
    mVersion(aVersion)
  {
    cSelectedItem vSelectedItem;
    vSelectedItem.mItem = mItemRoot;
    vSelectedItem.mSiblingIndex = 0;
    mSelection.push_back(vSelectedItem);
  }
  
  int GetItemSelectedIndex() const
  {
    return mSelection.back().mSiblingIndex;
  }
  
  int GetItemSelectedLevel() const
  {
    return mSelection.size() - 1;
  }
  
  int GetNumberOfSiblings() const
  {
    tItemPtr vParentItemPgm = GetParent();
    if (vParentItemPgm != 0) {
      return GetNumberOfChildren(vParentItemPgm);
    }
    else {
      return 1;
    }
  }
  
  void SelectEnter()
  {
    if (mEditing) {
      // stop editing, accept changes
      mEditing = false;
      mModified = true;
    }
    else {
      tItemType vType = ReadType(GetItemSelected());
      if (vType == eItemMenu) {
        tItemPtr vChildItem = GetChild(GetItemSelected(), 0);
        if (vChildItem != 0) {
          cSelectedItem vSelectedItem;
          vSelectedItem.mItem = vChildItem;
          vSelectedItem.mSiblingIndex = 0;
          mSelection.push_back(vSelectedItem);
        }
      }
      else if (vType == eItemAction) {
        cItem vItem = ReadItem(GetItemSelected());
        vItem.mValue.mAction();
      }
      else if ((vType == eItemByte) ||
               (vType == eItemInt) ||
               (vType == eItemLong) ||
               (vType == eItemFloat) ||
               (vType == eItemEnum)) {
        NumberBackup();
        mEditing = true;
      }
    }
  }
  
  void SelectExit()
  {
    if (mEditing) {
      // stop editing, discard changes
      NumberRestore();
      mEditing = false;
    }
    else if (mSelection.size() > 1) {
      mSelection.pop_back();
    }
  }
  
  void SelectNext()
  {
    if (mEditing) {
      ValueInc();
    }
    else {
      SelectSibling(GetItemSelectedIndex() + 1);
    }
  }
  
  void SelectPrev()
  {
    if (mEditing) {
      ValueDec();
    }
    else {
      SelectSibling(GetItemSelectedIndex() - 1);
    }
  }
  
  boolean IsEditing() const
  {
    return mEditing;
  }
  
  boolean IsModified() const
  {
    return mModified;
  }
  
  void SetModified(boolean aModified)
  {
    mModified = aModified;
  }
  
  template <typename TDevice>
  void PrintSibling(TDevice& aDevice, int aSiblingIndex) const
  {
    PrintItem(aDevice, GetSibling(aSiblingIndex));
  }
  
  template <typename TDevice>
  void print(TDevice& aDevice)
  {
    print(aDevice, mItemRoot);
  }
  
  template <typename TDevice>
  void put(TDevice& aDevice, int aAddress)
  {
    aDevice.put(aAddress, mClassID);
    aAddress += sizeof(mClassID);
    aDevice.put(aAddress, mVersion);
    aAddress += sizeof(mVersion);
    put(aDevice, mItemRoot, aAddress);
  }
  
  template <typename TDevice>
  boolean get(TDevice& aDevice, int aAddress)
  {
    // check data integrity - class name
    char vClassID[7] = "";
    aDevice.get(aAddress, vClassID);
    aAddress += sizeof(mClassID);
    if (strcmp(vClassID, mClassID) != 0) return false;
    // check data integrity - version
    int vVersion = 0;
    aDevice.get(aAddress, vVersion);
    aAddress += sizeof(mVersion);
    if (vVersion != mVersion) return false;
    // now read all the data
    get(aDevice, mItemRoot, aAddress);
    return true;
  }
  
private:
  
  tItemPtr GetItemSelected() const
  {
    return mSelection.back().mItem;
  }
  
  tItemPtr GetSibling(int aSiblingIndex) const
  {
    tItemPtr vParentItem = GetParent();
    if (vParentItem != 0) {
      return GetChild(vParentItem, aSiblingIndex);
    }
    else if (aSiblingIndex == 0) {
      return GetItemSelected();
    }
    else {
      return 0;
    }
  }
  
  void SelectSibling(int aSiblingIndex)
  {
    tItemPtr vParentItem = GetParent();
    if (vParentItem != 0) {
      tItemPtr vItem = GetChild(vParentItem, aSiblingIndex);
      if (vItem != 0) {
        mSelection.back().mItem = vItem;
        mSelection.back().mSiblingIndex = aSiblingIndex;
      }
    }
  }
  
  template <typename TNumber>
  void NumberInc(cNumberT<TNumber>& aNumber)
  {
    *aNumber.mValue += aNumber.mStep;
    if (*aNumber.mValue > aNumber.mMax) *aNumber.mValue = aNumber.mMax;
  }
  
  template <typename TNumber>
  void NumberDec(cNumberT<TNumber>& aNumber)
  {
    *aNumber.mValue -= aNumber.mStep;
    if (*aNumber.mValue < aNumber.mMin) *aNumber.mValue = aNumber.mMin;
  }
  
  void EnumInc(cEnum& aEnum)
  {
    if (*aEnum.mIndex + 1 < aEnum.mSize) ++(*aEnum.mIndex);
  }
  
  void EnumDec(cEnum& aEnum)
  {
    if (*aEnum.mIndex > 0) --(*aEnum.mIndex);
  }
  
  void ValueInc()
  {
    cItem vItem = ReadItem(GetItemSelected());
    switch (vItem.mType) {
      case eItemByte: NumberInc(vItem.mValue.mByte); break;
      case eItemInt: NumberInc(vItem.mValue.mInt); break;
      case eItemLong: NumberInc(vItem.mValue.mLong); break;
      case eItemFloat: NumberInc(vItem.mValue.mFloat); break;
      case eItemEnum: EnumInc(vItem.mValue.mEnum); break;
      default: break;
    }
  }
  
  void ValueDec()
  {
    cItem vItem = ReadItem(GetItemSelected());
    switch (vItem.mType) {
      case eItemByte: NumberDec(vItem.mValue.mByte); break;
      case eItemInt: NumberDec(vItem.mValue.mInt); break;
      case eItemLong: NumberDec(vItem.mValue.mLong); break;
      case eItemFloat: NumberDec(vItem.mValue.mFloat); break;
      case eItemEnum: EnumDec(vItem.mValue.mEnum); break;
      default: break;
    }
  }
  
  void NumberBackup()
  {
    cItem vItem = ReadItem(GetItemSelected());
    switch (vItem.mType) {
      case eItemByte: mBackupNumber.mByte = *vItem.mValue.mByte.mValue; break;
      case eItemInt: mBackupNumber.mInt = *vItem.mValue.mInt.mValue; break;
      case eItemLong: mBackupNumber.mLong = *vItem.mValue.mLong.mValue; break;
      case eItemFloat: mBackupNumber.mFloat = *vItem.mValue.mFloat.mValue; break;
      case eItemEnum: mBackupNumber.mEnum = *vItem.mValue.mEnum.mIndex; break;
      default: break;
    }
  }
  
  void NumberRestore()
  {
    cItem vItem = ReadItem(GetItemSelected());
    switch (vItem.mType) {
      case eItemByte: *vItem.mValue.mByte.mValue = mBackupNumber.mByte; break;
      case eItemInt: *vItem.mValue.mInt.mValue = mBackupNumber.mInt; break;
      case eItemLong: *vItem.mValue.mLong.mValue = mBackupNumber.mLong; break;
      case eItemFloat: *vItem.mValue.mFloat.mValue = mBackupNumber.mFloat; break;
      case eItemEnum: *vItem.mValue.mEnum.mIndex = mBackupNumber.mEnum; break;
      default: break;
    }
  }
  
  tItemPtr GetParent() const
  {
    if (mSelection.size() >= 2) {
      tSelection::const_iterator vItemIterator = mSelection.end();
      return (*(vItemIterator - 2)).mItem;
    }
    return 0;
  }
  
  static tItemPtr GetChild(tItemPtr aItem, int aChildIndex)
  {
    if ((aChildIndex >= 0) && IsMenu(aItem)) {
      cItem vItem = ReadItem(aItem);
      if (aChildIndex < vItem.mValue.mMenu.mSize) {
        return (tItemPtr)pgm_read_ptr_near(vItem.mValue.mMenu.mItems + aChildIndex);
      }
    }
    return 0;
  }
  
  static boolean HasChildren(tItemPtr aItem)
  {
    return GetNumberOfChildren(aItem) > 0;
  }
  
  static int GetNumberOfChildren(tItemPtr aItem)
  {
    if (IsMenu(aItem)) {
      cItem vItem = ReadItem(aItem);
      return vItem.mValue.mMenu.mSize;
    }
    else {
      return 0;
    }
  }
  
  template <typename TDevice>
  static void PrintMenu(TDevice& aDevice, const cItem& aItem)
  {
    PrintString(aDevice, aItem.mName);
    aDevice.print(">");
  }
  
  template <typename TDevice>
  static void PrintByte(TDevice& aDevice, const cItem& aItem)
  {
    PrintString(aDevice, aItem.mName);
    aDevice.print(": ");
    aDevice.print(*aItem.mValue.mByte.mValue);
    PrintString(aDevice, aItem.mValue.mByte.mUnit);
  }
  
  template <typename TDevice>
  static void PrintInt(TDevice& aDevice, const cItem& aItem)
  {
    PrintString(aDevice, aItem.mName);
    aDevice.print(": ");
    aDevice.print(*aItem.mValue.mInt.mValue);
    PrintString(aDevice, aItem.mValue.mInt.mUnit);
  }
  
  template <typename TDevice>
  static void PrintLong(TDevice& aDevice, const cItem& aItem)
  {
    PrintString(aDevice, aItem.mName);
    aDevice.print(": ");
    aDevice.print(*aItem.mValue.mLong.mValue);
    PrintString(aDevice, aItem.mValue.mLong.mUnit);
  }
  
  template <typename TDevice>
  static void PrintFloat(TDevice& aDevice, const cItem& aItem)
  {
    PrintString(aDevice, aItem.mName);
    aDevice.print(": ");
    aDevice.print(*aItem.mValue.mFloat.mValue, aItem.mValue.mFloat.mDecimals);
    PrintString(aDevice, aItem.mValue.mFloat.mUnit);
  }
  
  template <typename TDevice>
  static void PrintEnum(TDevice& aDevice, const cItem& aItem)
  {
    PrintString(aDevice, aItem.mName);
    aDevice.print(": ");
    PrintString(aDevice, (tStringPtr)pgm_read_ptr_near(aItem.mValue.mEnum.mNames + *aItem.mValue.mEnum.mIndex));
  }
  
  template <typename TDevice>
  static void PrintAction(TDevice& aDevice, const cItem& aItem)
  {
    aDevice.print("<");
    PrintString(aDevice, aItem.mName);
    aDevice.print(">");
  }
  
  template <typename TDevice>
  static void PrintItem(TDevice& aDevice, tItemPtr aItem)
  {
    cItem vItem = ReadItem(aItem);
    switch (vItem.mType) {
      case eItemMenu: PrintMenu(aDevice, vItem); break;
      case eItemByte: PrintByte(aDevice, vItem); break;
      case eItemInt: PrintInt(aDevice, vItem); break;
      case eItemLong: PrintLong(aDevice, vItem); break;
      case eItemFloat: PrintFloat(aDevice, vItem); break;
      case eItemEnum: PrintEnum(aDevice, vItem); break;
      case eItemAction: PrintAction(aDevice, vItem); break;
      default: break;
    }
  }
  
  static tItemType ReadType(tItemPtr aItem)
  {
    return (tItemType)pgm_read_byte_near(&aItem->mType);
  }
  
  static cItem ReadItem(tItemPtr aItem)
  {
    cItem vItem;
    memcpy_P(&vItem, aItem, sizeof(cItem));
    return vItem;
  }
  
  static boolean IsMenu(tItemPtr aItem)
  {
    return ReadType(aItem) == eItemMenu;
  }
  
  template <typename TDevice>
  static void PrintString(TDevice& aDevice, tStringPtr aStringPtr)
  {
    if (aStringPtr != 0) {
      char vChar = pgm_read_byte_near(aStringPtr);
      while (vChar != 0) {
        aDevice.print(vChar);
        vChar = pgm_read_byte_near(++aStringPtr);
      }
    }
  }
  
  template <typename TDevice>
  void print(TDevice& aDevice, tItemPtr aItem, int aIndentSize = 0)
  {
    if (aItem == 0) return;
    aDevice.print(aItem == GetItemSelected() ? (IsEditing() ? "*" : ">") : " ");
    for (int vIndentIndex = 0; vIndentIndex < aIndentSize; vIndentIndex++) aDevice.print(" ");
    PrintItem(aDevice, aItem);
    aDevice.println();
    int vNumberOfChildren = GetNumberOfChildren(aItem);
    for (int vChildIndex = 0; vChildIndex < vNumberOfChildren; vChildIndex++) {
      print(aDevice, GetChild(aItem, vChildIndex), aIndentSize + 2);
    }
  }
  
  template <typename TDevice>
  void put(TDevice& aDevice, tItemPtr aItem, int& aAddress)
  {
    int vNumberOfChildren = GetNumberOfChildren(aItem);
    for (int vChildIndex = 0; vChildIndex < vNumberOfChildren; vChildIndex++) {
      tItemPtr vChildItemPGM = GetChild(aItem, vChildIndex);
      cItem vChildItem = ReadItem(vChildItemPGM);
      switch (vChildItem.mType) {
        case eItemMenu: put(aDevice, vChildItemPGM, aAddress); break;
        case eItemByte: aDevice.put(aAddress, *vChildItem.mValue.mByte.mValue); aAddress += sizeof(byte); break;
        case eItemInt: aDevice.put(aAddress, *vChildItem.mValue.mInt.mValue); aAddress += sizeof(int); break;
        case eItemLong: aDevice.put(aAddress, *vChildItem.mValue.mLong.mValue); aAddress += sizeof(long); break;
        case eItemFloat: aDevice.put(aAddress, *vChildItem.mValue.mFloat.mValue); aAddress += sizeof(float); break;
        case eItemEnum: aDevice.put(aAddress, *vChildItem.mValue.mEnum.mIndex); aAddress += sizeof(byte); break;
        default: break;
      }
    }
  }
  
  template <typename TDevice>
  void get(TDevice& aDevice, tItemPtr aItem, int& aAddress)
  {
    int vNumberOfChildren = GetNumberOfChildren(aItem);
    for (int vChildIndex = 0; vChildIndex < vNumberOfChildren; vChildIndex++) {
      tItemPtr vChildItemPGM = GetChild(aItem, vChildIndex);
      cItem vChildItem = ReadItem(vChildItemPGM);
      switch (vChildItem.mType) {
        case eItemMenu: get(aDevice, vChildItemPGM, aAddress); break;
        case eItemByte: aDevice.get(aAddress, *vChildItem.mValue.mByte.mValue); aAddress += sizeof(byte); break;
        case eItemInt: aDevice.get(aAddress, *vChildItem.mValue.mInt.mValue); aAddress += sizeof(int); break;
        case eItemLong: aDevice.get(aAddress, *vChildItem.mValue.mLong.mValue); aAddress += sizeof(long); break;
        case eItemFloat: aDevice.get(aAddress, *vChildItem.mValue.mFloat.mValue); aAddress += sizeof(float); break;
        case eItemEnum: aDevice.get(aAddress, *vChildItem.mValue.mEnum.mIndex); aAddress += sizeof(byte); break;
        default: break;
      }
    }
  }
  
  tItemPtr mItemRoot;
  
  struct cSelectedItem {
    tItemPtr mItem;
    int mSiblingIndex;
  };
  
  typedef ggVector<cSelectedItem> tSelection;
  
  tSelection mSelection;
  boolean mEditing;
  boolean mModified;
  const int mVersion;
  
  union {
    byte mByte;
    int mInt;
    long mLong;
    float mFloat;
    byte mEnum;
  } mBackupNumber;
};

#endif // __GG_MENU__
