#ifndef __SYNTH_TFM_1__ENVELOPE__
#define __SYNTH_TFM_1__ENVELOPE__

#include <cmath>

class Envelope {
public:
	enum Envelope_stage {
		ENVELOPE_STAGE_OFF = 0,
		ENVELOPE_STAGE_ATTACK,
		ENVELOPE_STAGE_DECAY,
		ENVELOPE_STAGE_SUSTAIN,
		ENVELOPE_STAGE_RELEASE,
		kNumEnvelope_stages
	};
	void enterStage(Envelope_stage newStage);
	double nextSample();
	void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; };
	inline Envelope_stage getCurrentStage() const { return currentStage; };
	const double minimumLevel;

	Envelope() :
		minimumLevel(0.0001),
		currentStage(ENVELOPE_STAGE_OFF),
		currentLevel(minimumLevel),
		multiplier(1.0),
		sampleRate(44100.0),
		currentSampleIndex(0),
		nextStageSampleIndex(0) {
		stageValue[ENVELOPE_STAGE_OFF] = 0.0;
		stageValue[ENVELOPE_STAGE_ATTACK] = 0.01;
		stageValue[ENVELOPE_STAGE_DECAY] = 0.5;
		stageValue[ENVELOPE_STAGE_SUSTAIN] = 0.1;
		stageValue[ENVELOPE_STAGE_RELEASE] = 1.0;
	};
private:
	Envelope_stage currentStage;
	double currentLevel;
	double multiplier;
	double sampleRate;
	double stageValue[kNumEnvelope_stages];
	void calculateMultiplier(double startLevel, double endLevel, unsigned long long lengthInSamples);
	unsigned long long currentSampleIndex;
	unsigned long long nextStageSampleIndex;
};

#endif
