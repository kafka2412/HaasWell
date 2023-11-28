/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/
#pragma once

#include "IPlugConstants.h"
#include "denormal.h"

// Namespace has been changed from iplug to kafka to
//    distinguish the customized SmoothedGain class
namespace kafka
{

template <typename T, int NC = 1>
class LogParamSmooth
{
private:
  double mA, mB;
  T mOutM1[NC];

public:
  LogParamSmooth(double timeMs = 5., T initialValue = 0.)
  {
    for (auto i = 0; i < NC; i++)
    {
      mOutM1[i] = initialValue;
    }

    SetSmoothTime(timeMs, DEFAULT_SAMPLE_RATE);
  }

  // only works for NC = 1
  inline T Process(T input)
  {
    mOutM1[0] = (input * mB) + (mOutM1[0] * mA);
#ifndef OS_IOS
    denormal_fix(&mOutM1[0]);
#endif
    return mOutM1[0];
  }

  inline void SetValue(T value)
  {
    for (auto i = 0; i < NC; i++)
    {
      mOutM1[i] = value;
    }
  }

  inline void SetValues(T values[NC])
  {
    for (auto i = 0; i < NC; i++)
    {
      mOutM1[i] = values[i];
    }
  }

  void SetSmoothTime(double timeMs, double sampleRate)
  {
    static constexpr double TWO_PI = 6.283185307179586476925286766559;

    mA = exp(-TWO_PI / (timeMs * 0.001 * sampleRate));
    mB = 1.0 - mA;
  }

  void ProcessBlock(T inputs[NC], T** outputs, int nFrames, int channelOffset = 0)
  {
    const T b = mB;
    const T a = mA;

    for (auto s = 0; s < nFrames; ++s)
    {
      for (auto c = 0; c < NC; c++)
      {
        T output = (inputs[channelOffset + c] * b) + (mOutM1[c] * a);
#ifndef OS_IOS
        denormal_fix(&output);
#endif
        mOutM1[c] = output;
        outputs[channelOffset + c][s] = output;
      }
    }
  }

} WDL_FIXALIGN;

template <typename T>
class SmoothedGain
{
public:
  void ProcessBlock(T** inputs, T** outputs, int nChans, int nFrames, double gainValue)
  {
    for (auto s = 0; s < nFrames; ++s)
    {
      const double smoothedGain = mSmoother.Process(gainValue);

      for (auto c = 0; c < nChans; c++)
      {
        outputs[c][s] = inputs[c][s] * smoothedGain;
      }
    }
  }

  // Overload for phase inversion, (Left: 0, Right: 1)
  void ProcessBlock(T** inputs, T** outputs, int nChans, int nFrames, double gainValue, const int CHANNEL)
  {
    for (auto s = 0; s < nFrames; ++s)
    {
      const double smoothedGain = mSmoother.Process(gainValue);

      for (auto c = 0; c < nChans; c++)
      {
        // If the channel is the one we want to invert, invert it
        if (c == CHANNEL)
        {
          outputs[c][s] = inputs[c][s] * -smoothedGain;
          continue;
        }
        outputs[c][s] = inputs[c][s] * smoothedGain;
      }
    }
  }

private:
  LogParamSmooth<double, 1> mSmoother;
};

} // namespace kafka
