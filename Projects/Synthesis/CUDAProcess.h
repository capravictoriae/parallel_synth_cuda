#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "MIDIReceiver.h"

enum EnvelopeStage {
	ENVELOPE_STAGE_OFF = 0,
	ENVELOPE_STAGE_ATTACK,
	ENVELOPE_STAGE_DECAY,
	ENVELOPE_STAGE_SUSTAIN,
	ENVELOPE_STAGE_RELEASE,
	kNumEnvelopeStages
};

class CUDAProcess
{
private:
	Oscillator mOscillator;
public:
	double lastPhaseIncrementValue;
	void Process(double* lbuffer, double* rbuffer, int lastvel, double lastfreq, double mPI, double twoPI, double mSampleRate, Envelope* mEnvelope, OscillatorMode mode);
	CUDAProcess() :
		lastPhaseIncrementValue(0.0){
	};
};

