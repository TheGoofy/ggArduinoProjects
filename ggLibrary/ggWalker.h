#ifndef __GG_WALKER__
#define __GG_WALKER__

#include "Arduino.h"

/**
 * This class allows a compact way to build a loop using
 * iterators. It also helps to not forget calling "++" of the
 * iterator, because this is done internally while the condition
 * for the next iteration end is checked. See the following
 * example:
 *
 * <code>
 * std::vector<bpSize> vVector;
 * bpWalker<std::vector<bpSize>::iterator> vWalker(vVector);
 * bpSize vSum = 0;
 * while (vWalker) {
 *   vSum += *vWalker;
 * }
 * </code>
 */
template <class TIterator, class TValue = typename TIterator::reference>
class ggWalker
{

public:

  /**
   * Creates a new walker object. Iterates over all elements of a
   * constant container (read only).
   */
  template <typename TContainer>
  ggWalker(const TContainer& aContainer)
    : mBegin(aContainer.begin()),
      mIterator(aContainer.begin()),
      mIteratorNext(aContainer.begin()),
      mEnd(aContainer.end()) {
  }

  /**
   * Creates a new walker object. Iterates over all elements of a
   * non constant container (read and write allowed).
   */
  template <typename TContainer>
  ggWalker(TContainer& aContainer)
    : mBegin(aContainer.begin()),
      mIterator(aContainer.begin()),
      mIteratorNext(aContainer.begin()),
      mEnd(aContainer.end()) {
  }

  /**
   * Creates a new walker object.
   */
  ggWalker(const TIterator& aBegin,
           const TIterator& aEnd)
    : mBegin(aBegin),
      mIterator(aBegin),
      mIteratorNext(aBegin),
      mEnd(aEnd) {
  }

  /**
   * Returns "true", if there is an element available for
   * iteration. It internally advances already one step, but the
   * element accessed by "Get()" remains at the previous position.
   * The position of the element accessed by "Get()" is
   * incremented with the next call of "Walk()".
   */
  inline bool Walk() {
    if (mIteratorNext != mEnd) {
      mIterator = mIteratorNext;
      ++mIteratorNext;
      return true;
    }
    else {
      mIterator = mIteratorNext;
      return false;
    }
  }

  /**
   * Does the same as "Walk()".
   */
  inline operator bool() {
    return Walk();
  }

  /**
   * Returns the element for the current iteration.
   */
  inline TValue& Get() const {
    return *mIterator;
  }

  /**
   * Does the same as "Get()" .
   */
  inline const TValue& operator * () const {
    return *mIterator;
  }

  /**
   * Does the same as "&Get()" .
   * TIterator::reference* should be TIterator::pointer, but it is not defined for some bpIterators
   */
//  inline typename TIterator::pointer operator -> () const {
//    return mIterator.operator -> ();
//  }

  /**
   * Resets the walker to the initial position (new game - go back
   * to the start).
   */
  inline void Reset() {
    mIterator = mBegin;
    mIteratorNext = mBegin;
  }

private:

  /// begin of the iteration
  TIterator mBegin;

  /// position of the current element
  TIterator mIterator;

  /// position of the element after "mIterator"
  TIterator mIteratorNext;

  /// end of the iteration
  TIterator mEnd;

};

#endif // __GG_WALKER__


