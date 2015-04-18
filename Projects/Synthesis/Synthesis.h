#ifndef __SYNTHESIS__
#define __SYNTHESIS__

#include "IPlug_include_in_plug_hdr.h"

#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "Envelope.h"
#include "Filter.h"

#include "CUDAProcess.h"

#include <vector>

class Synthesis : public IPlug
{
public:
	Synthesis(IPlugInstanceInfo instanceInfo);
	~Synthesis();

	void Reset();
	void OnParamChange(int paramIdx);
	void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
	// to receive MIDI messages:
	void ProcessMidiMsg(IMidiMsg* pMsg);

private:
	std::vector<double> timer_list;
	CUDAProcess mCUDA;
	double mFrequency;
	void CreatePresets();
	Oscillator mOscillator;
	MIDIReceiver mMIDIReceiver;
	Envelope mEnvelope;
	inline void onNoteOn(const int noteNumber, const int velocity) { mEnvelope.enterStage(Envelope::ENVELOPE_STAGE_ATTACK); };
	inline void onNoteOff(const int noteNumber, const int velocity) { mEnvelope.enterStage(Envelope::ENVELOPE_STAGE_RELEASE); };
	Filter mFilter;
};

#endif
