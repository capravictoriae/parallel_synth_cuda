#include <math.h>

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
	double osc_freq;
	double osc_phase;
	double osc_samp_rate;
	double osc_phase_increment;
	void update_increment();
public:
	void set_mode(Oscillator_mode mode);
	void set_freq(double freq);
	void set_samp_rate(double samp_rate);
	void generate(double* buffer, int osc_frames);
	Oscillator() :
		osc_mode(OSCILLATOR_MODE_SAW),
		osc_num_pi(2 * acos(0.0)),
		osc_freq(440.0),
		osc_phase(0.0),
		osc_samp_rate(44100.0) {
		update_increment();
	}
};