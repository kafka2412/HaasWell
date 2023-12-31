#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Smoothers2.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kPhaseInvertChannel,

  kNumParams,
};

using namespace iplug;
using namespace igraphics;

class HaasWell final : public Plugin
{
public:
  HaasWell(const InstanceInfo& info);
  kafka::SmoothedGain<sample> mGainSmoother;

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
