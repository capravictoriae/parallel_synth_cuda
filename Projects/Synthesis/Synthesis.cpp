#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

#include <math.h>
#include <algorithm>

#include <omp.h>

const int kNumPrograms = 5;

enum EParams
{
	mWaveform = 0,
	mAttack,
	mDecay,
	mSustain,
	mRelease,
	mFilterMode,
	mCutoff,
	mResonance,
	kNumParams
};

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo) {
	TRACE;

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

	// Waveform switch
	GetParam(mWaveform)->InitEnum("Waveform", OSCILLATOR_MODE_SINE, kNumOscModes);
	GetParam(mWaveform)->SetDisplayText(0, "Sine"); // Needed for VST3, thanks plunntic
	IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
	pGraphics->AttachControl(new ISwitchControl(this, 24, 53, mWaveform, &waveformBitmap));

	// Knob bitmap for ADSR
	IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 64);
	// Attack knob:
	GetParam(mAttack)->InitDouble("Attack", 0.01, 0.01, 10.0, 0.001);
	GetParam(mAttack)->SetShape(3);
	pGraphics->AttachControl(new IKnobMultiControl(this, 95, 34, mAttack, &knobBitmap));
	// Decay knob:
	GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
	GetParam(mDecay)->SetShape(3);
	pGraphics->AttachControl(new IKnobMultiControl(this, 177, 34, mDecay, &knobBitmap));
	// Sustain knob:
	GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
	GetParam(mSustain)->SetShape(2);
	pGraphics->AttachControl(new IKnobMultiControl(this, 259, 34, mSustain, &knobBitmap));
	// Release knob:
	GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
	GetParam(mRelease)->SetShape(3);
	pGraphics->AttachControl(new IKnobMultiControl(this, 341, 34, mRelease, &knobBitmap));

	// Filter Switch
	GetParam(mFilterMode)->InitEnum("Filter Mode", Filter::FILTER_MODE_LOWPASS, Filter::kNumFilterModes);
	IBitmap filtermodeBitmap = pGraphics->LoadIBitmap(FILTERMODE_ID, FILTERMODE_FN, 3);
	pGraphics->AttachControl(new ISwitchControl(this, 24, 123, mFilterMode, &filtermodeBitmap));

	// Knobs for filter cutoff and resonance
	IBitmap smallKnobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 64);
	// Cutoff knob:
	GetParam(mCutoff)->InitDouble("Cutoff", 0.99, 0.01, 0.99, 0.001);
	GetParam(mCutoff)->SetShape(2);
	pGraphics->AttachControl(new IKnobMultiControl(this, 5, 177, mCutoff, &smallKnobBitmap));
	// Resonance knob:
	GetParam(mResonance)->InitDouble("Resonance", 0.01, 0.01, 1.0, 0.001);
	pGraphics->AttachControl(new IKnobMultiControl(this, 61, 177, mResonance, &smallKnobBitmap));

	AttachGraphics(pGraphics);

	CreatePresets();
}

Synthesis::~Synthesis() {}

void Synthesis::CreatePresets() {
	
}

void Synthesis::ProcessDoubleReplacing(double** inputs,
                                       double** outputs,
                                       int nFrames) {

	double t1 = omp_get_wtime();

  // Mutex is already locked for us.
  
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];

  for (size_t i = 0; i < nFrames; i++)
  {
	  mMIDIReceiver.advance();
  }

  mCUDA.Process(leftOutput, rightOutput, mMIDIReceiver.getLastVelocity(), mMIDIReceiver.getLastFrequency(), 
	  mOscillator.getmPI(), mOscillator.gettwoPI(), mOscillator.getSampleRate(), &mEnvelope, &mFilter, mOscillator.getmOscillatorMode());

  /*
  for (int i = 0; i < nFrames; ++i) {
	  
	  int velocity = mMIDIReceiver.getLastVelocity();
	  if (velocity > 0) {
		  mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
		  mOscillator.setMuted(false);
	  }
	  else {
		  mOscillator.setMuted(true);
	  }

	  leftOutput[i] = rightOutput[i] = mFilter.process(mOscillator.nextSample() * mEnvelope.nextSample() * velocity / 127.0);
  }
  */
  
  mMIDIReceiver.Flush(nFrames);

  mMIDIReceiver.noteOn.Connect(this, &Synthesis::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &Synthesis::onNoteOff);

  double t2 = omp_get_wtime() - t1;
  
  timer_list.push_back(t2);

  if (timer_list.size() >= 100){

	  double sum_value = 0.0;

	  for (size_t i = 0; i < timer_list.size(); i++)
	  {
		  sum_value += timer_list.at(i);
	  }

	  DBGMSG("%.9f\n", sum_value/timer_list.size());

	  timer_list.empty();
  }


}

void Synthesis::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mEnvelope.setSampleRate(GetSampleRate());
}

void Synthesis::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);
	switch (paramIdx) {
	case mWaveform:
		mOscillator.setMode(static_cast<OscillatorMode>(GetParam(mWaveform)->Int()));
		break;
	case mAttack:
	case mDecay:
	case mSustain:
	case mRelease:
		mEnvelope.setStageValue(static_cast<Envelope::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
		break;
	case mCutoff:
		mFilter.setCutoff(GetParam(paramIdx)->Value());
		break;
	case mResonance:
		mFilter.setResonance(GetParam(paramIdx)->Value());
		break;
	case mFilterMode:
		mFilter.setFilterMode(static_cast<Filter::FilterMode>(GetParam(paramIdx)->Int()));
		break;
  }
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
}

