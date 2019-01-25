#ifndef __GG_VECTOR__
#define __GG_VECTOR__

#include "Arduino.h"

template <class TValue>
class ggVector
{
public:
  
  typedef TValue value_type;
  typedef TValue* iterator;
  typedef const TValue* const_iterator;
  
  ggVector()
  : mBegin(0),
    mEnd(0),
    mEndCapacity(0)
  {
  }
  
  ggVector(unsigned long aSize)
  {
    mBegin = new TValue[aSize];
    mEnd = mBegin + aSize;
    mEndCapacity = mEnd;
  }
  
  ggVector(unsigned long aSize, const TValue& aDefaultValue)
  {
    mBegin = new TValue[aSize];
    mEnd = mBegin + aSize;
    mEndCapacity = mEnd;
    TValue* vIterator = mBegin;
    while (vIterator != mEnd) {
      *vIterator = aDefaultValue;
      ++vIterator;
    }
  }
  
  virtual ~ggVector()
  {
    delete[] mBegin;
  }
  
  inline iterator begin()
  {
    return mBegin;
  }
  
  inline iterator end()
  {
    return mEnd;
  }
  
  inline const_iterator begin() const
  {
    return mBegin;
  }
  
  inline const_iterator end() const
  {
    return mEnd;
  }
  
  inline TValue& operator[] (unsigned long aIndex)
  {
    return mBegin[aIndex];
  }
  
  inline const TValue& operator[] (unsigned long aIndex) const
  {
    return mBegin[aIndex];
  }
  
  inline TValue& front()
  {
    return *mBegin;
  }

  inline const TValue& front() const
  {
    return *mBegin;
  }

  inline TValue& back()
  {
    return *(mEnd - 1);
  }

  inline const TValue& back() const
  {
    return *(mEnd - 1);
  }  
  
  inline unsigned long size() const
  {
    return mEnd - mBegin;
  }
  
  inline void resize(unsigned long aSize)
  {
    unsigned long vCapacity = mEndCapacity - mBegin;
    if (aSize != vCapacity) {
      TValue* vBegin = new TValue[aSize];
      memcpy(vBegin, mBegin, aSize * sizeof(TValue));
      delete[] mBegin;
      mBegin = vBegin;
      mEnd = mBegin + aSize;
      mEndCapacity = mEnd;
    }
  }
  
  inline void push_back(const TValue& aValue)
  {
    if (mEnd == mEndCapacity) {
      unsigned long vCapacity = mEndCapacity - mBegin;
      unsigned long vSize = mEnd - mBegin;
      vCapacity = vCapacity + 1 + vCapacity / 3;
      TValue* vBegin = new TValue[vCapacity];
      memcpy(vBegin, mBegin, vSize * sizeof(TValue));
      delete[] mBegin;
      mBegin = vBegin;
      mEnd = mBegin + vSize;
      mEndCapacity = mBegin + vCapacity;
    }
    *mEnd = aValue;
    ++mEnd;
  }
  
  inline void pop_back()
  {
    if (mEnd != mBegin) mEnd--;
  }
  
  inline void clear()
  {
    delete[] mBegin;
    mBegin = 0;
    mEnd = 0;
    mEndCapacity = 0;
  }
  
  inline boolean empty() const
  {
    return mBegin == mEnd;
  }

private:
  
  TValue* mBegin;
  TValue* mEnd;
  TValue* mEndCapacity;
  
};

#endif // __GG_VECTOR__
