#include "HaasWell.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

HaasWell::HaasWell(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  GetParam(kPhaseInvertChannel)->InitEnum("PhaseChannel", 0, {"Left", "Right"});

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));

    // Add phase invert button (Left or Right)
    pGraphics->AttachControl(new IVRadioButtonControl(b.GetCentredInside(100).GetVShifted(-100).GetHShifted(100), kPhaseInvertChannel, {"Left", "Right"}, "InvertChannel"));
  };
#endif
}

#if IPLUG_DSP
void HaasWell::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  const int phaseInvertChannel = GetParam(kPhaseInvertChannel)->Int();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      // If the channel is the one we want to invert, invert it
      if (c == phaseInvertChannel) {
        outputs[c][s] = inputs[c][s] * -gain;
        continue;
      }
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif
