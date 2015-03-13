#ifndef __SYNTH_TFM_1__
#define __SYNTH_TFM_1__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"

class synth_tfm_1 : public IPlug
{
public:
  synth_tfm_1(IPlugInstanceInfo instanceInfo);
  ~synth_tfm_1();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
};

#endif
