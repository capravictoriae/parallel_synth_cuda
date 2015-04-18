#ifndef __Synthesis__Oscillator__
#define __Synthesis__Oscillator__

#include <math.h>

enum OscillatorMode {
    OSCILLATOR_MODE_SINE,
    OSCILLATOR_MODE_SAW,
    OSCILLATOR_MODE_SQUARE,
    OSCILLATOR_MODE_TRIANGLE,
	kNumOscModes
};

class Oscillator {
private:
    OscillatorMode mOscillatorMode;
    const double mPI;
	const double twoPI;
	bool isMuted;
    double mFrequency;
    double mPhase;
    double mSampleRate;
    double mPhaseIncrement;
	void updateIncrement();
public:
    void setMode(OscillatorMode mode);
    void setFrequency(double frequency);
    void setSampleRate(double sampleRate);
    void generate(double* buffer, int nFrames);
	inline void setMuted(bool muted) { isMuted = muted; }
	OscillatorMode getmOscillatorMode(){ return mOscillatorMode; };
	double getmPI() { return mPI; };
	double gettwoPI() { return twoPI; };
	double getSampleRate() { return mSampleRate; };
	double getmPhase(){ return mPhase; };
	virtual double nextSample();
	double pbs(double t);
	double lastOutput;
	
    Oscillator() :
		mOscillatorMode(OSCILLATOR_MODE_SAW),
		mPI(2*acos(0.0)),
		twoPI(2 * mPI), 
		isMuted(true),  
		mFrequency(440.0),
		mPhase(0.0),
		lastOutput(0.0),
		mSampleRate(44100.0) { 
			updateIncrement(); 
		};
};

#endif /* defined(__Synthesis__Oscillator__) */
