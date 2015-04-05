#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

#include <math.h>
#include <algorithm>

const int kNumPrograms = 5;

enum EParams
{
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

	AttachGraphics(pGraphics);

	CreatePresets();
}

Synthesis::~Synthesis() {}

void Synthesis::CreatePresets() {
	
}

void Synthesis::ProcessDoubleReplacing(double** inputs,
                                       double** outputs,
                                       int nFrames) {
  // Mutex is already locked for us.
  
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];

  for (size_t i = 0; i < nFrames; i++)
  {
	  mMIDIReceiver.advance();
  }

  //DBGMSG("%f", mMIDIReceiver.getLastFrequency());
  // void Process(double* lbuffer, double* rbuffer, int lastvel, double lastfreq, double mPI, double twoPI, double mSampleRate, Envelope* mEnvelope, OscillatorMode mode);
  mCUDA.Process(leftOutput, rightOutput, mMIDIReceiver.getLastVelocity(), mMIDIReceiver.getLastFrequency(), 
	  mOscillator.getmPI(), mOscillator.gettwoPI(), mOscillator.getSampleRate(), &mEnvelope, mOscillator.getmOscillatorMode());

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
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
}

