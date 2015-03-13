#ifndef __SYNTH_TFM_1__OSCILLATOR__
#define __SYNTH_TFM_1__OSCILLATOR__

#include <math.h>

/**
*	Oscillator class used for defining and generating 
*	the differente waveforms
*/

enum Oscillator_mode {
	OSCILLATOR_MODE_SINE,
	OSCILLATOR_MODE_SAW,
	OSCILLATOR_MODE_SQUARE,
	OSCILLATOR_MODE_TRIANGLE
};

class Oscillator {
private:
	Oscillator_mode osc_mode;
	const double osc_num_pi;
	const double osc_two_pi;
	double osc_freq;
	double osc_phase;
	double osc_samp_rate;
	double osc_phase_increment;
	bool osc_is_muted;
	void update_increment();
public:
	void set_mode(Oscillator_mode mode) { osc_mode = mode; }
	void set_freq(double freq) { osc_freq = freq; }
	void set_samp_rate(double samp_rate) { osc_samp_rate = samp_rate; }
	inline void set_muted(bool muted) { osc_is_muted = muted; }
	double nextSample();
	void generate(double* buffer, int osc_frames);
	Oscillator() :
		osc_mode(OSCILLATOR_MODE_SAW),
		osc_num_pi(2 * acos(0.0)),
		osc_two_pi(2 * osc_num_pi),
		osc_is_muted(true),
		osc_freq(440.0),
		osc_phase(0.0),
		osc_samp_rate(44100.0) {
		update_increment();
	}
};

#endif 