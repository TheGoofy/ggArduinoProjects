#pragma once

// Define a type alias for the callback function.
typedef void (*ggTimerCallbackFunc)();

/**
 * @brief A high-resolution timer class for periodic execution of a callback function.
 *
 * This class allows you to execute a callback function at regular intervals
 * with microsecond precision. It calculates the timing internally and triggers
 * the callback function when the specified interval has elapsed.
 */
template<unsigned long TIntervalMicros, // Interval between executions in microseconds
         ggTimerCallbackFunc TCallback, // Callback function to be executed
         typename TMicrosType = unsigned long> // Type for timing (defaults to unsigned long)
class ggTimerT
{
public:

  // Constructor initializes the next execution time to zero
  ggTimerT() {
    Reset();
  }

  //
  void Reset() {
    mNextMicros = micros() + TIntervalMicros;
  }

  // Run method to be called periodically to check and execute the callback
  void Run() {
    // Get the current time in microseconds
    TMicrosType vCurrentMicros = micros();
    // Calculate the overdue time for the next deadline 
    TMicrosType vOverdueMicros = vCurrentMicros - mNextMicros;
    // Early return if overdue time is negative (deadline not yet reached)
    // Note: for unsigned types the MSB can be interpreted as sign bit (value rolls over)
    if (CheckMSB(vOverdueMicros)) return;
    // Calculate the elapsed time since the last execution
    TMicrosType vDeltaMicros = vOverdueMicros + TIntervalMicros;
    // Calculate time for next callback
    // Nnote: expected rollover handled by subtraction and MSB-check above
    TMicrosType vIntervalsElapsed = (vDeltaMicros / TIntervalMicros);
    mNextMicros += vIntervalsElapsed * TIntervalMicros;
    // Call the callback function with the elapsed time as an argument
    TCallback();
  }

private:

  // Check if the MSB is set (represents the sign of a number)
  inline bool CheckMSB(const TMicrosType& aValue) const {
    return aValue >> (8 * sizeof(TMicrosType) - 1);
  }

  // Timestamp when the next callback should be executed
  TMicrosType mNextMicros = 0;
};
