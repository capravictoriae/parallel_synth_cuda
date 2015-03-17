#ifndef __SYNTH_TFM_1__
#define __SYNTH_TFM_1__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"

/**
*	Main synth class. Defines the processing of the plugin/app
*	stores values for GUI and presets.
*	Also defines what to do when plugin is resetted (sample rate change),
*	and how the GUI interacts with the processing - onParamChange()
*/

class synth_tfm_1 : public IPlug
{
public:
  synth_tfm_1(IPlugInstanceInfo instanceInfo);
  ~synth_tfm_1();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);

  // things for the UI Keyboard
  inline int GetNumKeys() const { return mMIDIReceiver.getNumKeys(); };
  // return true if key is pressed
  inline bool GetKeyStatus(int key) const { return mMIDIReceiver.getKeyStatus(key); };
  static const int virtualKeyboardMinimumNoteNumber = 48;
  int lastVirtualKeyboardNoteNumber;

private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
  IControl* mVirtualKeyboard;
  void processVirtualKeyboard();
};

#endif