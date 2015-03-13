#include "synth_tfm_1.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

#include <algorithm>
#include <math.h>

const int kNumPrograms = 5;

enum EParams
{
  kFrequency = 0,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kFrequencyX = 96,
  kFrequencyY = 96,
  kKnobFrames = 128
};

synth_tfm_1::synth_tfm_1(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
	TRACE;

	//arguments are: name, defaultVal, minVal, maxVal, step, label
	GetParam(kFrequency)->InitDouble("Frequency", 440.0, 50.0, 20000.0, 0.01, "Hz");
	GetParam(kFrequency)->SetShape(2.0);

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	// pGraphics->AttachPanelBackground(&COLOR_RED);
	pGraphics->AttachPanelBackground(&COLOR_BLACK);

	IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);

	pGraphics->AttachControl(new IKnobMultiControl(this, kFrequencyX, kFrequencyY, kFrequency, &knob));

	AttachGraphics(pGraphics);

	//MakePreset("preset 1", ... );
	CreatePresets();
}

synth_tfm_1::~synth_tfm_1() {}

void synth_tfm_1::CreatePresets() {
	MakePreset("clean", 440.0);
}


void synth_tfm_1::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

	double *left_output = outputs[0];
	double *right_output = outputs[1];

	mOscillator.generate(left_output, nFrames);
	//mOscillator.generate(right_output, nFrames);

	
	// Copy left buffer into right buffer:
	// faster than generating all samples
	for (int s = 0; s < nFrames; ++s) {
		right_output[s] = left_output[s];
	}
	
}

void synth_tfm_1::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.set_samp_rate(GetSampleRate()); // GetSampleRate inherited from IPlugBase class
}

void synth_tfm_1::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

	switch (paramIdx)
	{
	case kFrequency:
		mOscillator.set_freq(GetParam(kFrequency)->Value());
		break;

	default:
		break;
	}
}

