#include "synth_tfm_1.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

#include <algorithm>
#include <math.h>

const int kNumPrograms = 5;

enum EParams
{
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
};

synth_tfm_1::synth_tfm_1(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
	TRACE;

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	pGraphics->AttachPanelBackground(&COLOR_BLACK);
	AttachGraphics(pGraphics);
	CreatePresets();
}

synth_tfm_1::~synth_tfm_1() {}

void synth_tfm_1::CreatePresets() {
	MakeDefaultPreset((char *)  "-", kNumParams);
}

/**
*	This is where the actual processing is done.
*	@param	inputs input buffer for processing (not used for a synth, no incoming audio)
*	@param	outputs output buffer for processing
*	@param	nFrames number of frames to process
*/

void synth_tfm_1::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

	double *left_output = outputs[0];
	double *right_output = outputs[1];

	for (size_t i = 0; i < nFrames; i++)
	{
		mMIDIReceiver.advance();
		int velocity = mMIDIReceiver.getLastVelocity();
		if (velocity > 0){
			mOscillator.set_freq(mMIDIReceiver.getLastFrequency());
			mOscillator.set_muted(false);
		}
		else {
			mOscillator.set_muted(true);
		}
		left_output[i] = right_output[i] = mOscillator.nextSample() * velocity / 127.0;
	}

	mMIDIReceiver.Flush(nFrames);
	
}

/**
*	Called when the sample rate changes.
*/

void synth_tfm_1::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.set_samp_rate(GetSampleRate()); // GetSampleRate inherited from IPlugBase class
}

/**
*	Called whenever an element of the GUI / MIDI changes
*	@param	paramIdx ID of the element that changed.
*/

void synth_tfm_1::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

}

/**
*	Called whenever a MIDI message is received.
*	@param	pMsg MIDI message received
*/

void synth_tfm_1::ProcessMidiMsg(IMidiMsg* pMsg){
	mMIDIReceiver.onMessageReceived(pMsg);
}

