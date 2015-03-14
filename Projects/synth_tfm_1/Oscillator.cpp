#include "Oscillator.h"

/**
*	Setters
*	TODO
*/

void Oscillator::set_mode(Oscillator_mode mode) {
	osc_mode = mode;
}

void Oscillator::set_freq(double freq) {
	osc_freq = freq;
	update_increment();
}

void Oscillator::set_samp_rate(double samp_rate) {
	osc_samp_rate = samp_rate;
	update_increment();
}

/**
*	Update for the new phase increment introduced. Update every time
*	the frequency or sample rate is updated.
*/

void Oscillator::update_increment(){
	osc_phase_increment = osc_freq * 2 * osc_num_pi / osc_samp_rate;
}

/**
*	TODO
*/

double Oscillator::nextSample(){
	double value = 0.0;
	if (osc_is_muted){
		return value;
	}
	
	switch (osc_mode)
	{
	case OSCILLATOR_MODE_SINE:
		value = sin(osc_phase);
		break;
	case OSCILLATOR_MODE_SAW:
		value = 1.0 - (2.0 * osc_phase / osc_two_pi);
		break;
	case OSCILLATOR_MODE_SQUARE:
		if (osc_phase <= osc_num_pi) {
			value = 1.0;
		}
		else {
			value = -1.0;
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		value = -1.0 + (2.0 * osc_phase / osc_two_pi);
		value = 2.0 * (fabs(value) - 0.5);
		break;
	}

	osc_phase += osc_phase_increment;
	while (osc_phase >= osc_two_pi){
		osc_phase -= osc_two_pi;
	}
	return value;
}

/**
*	Called by ProcessDoubleReplacing. Fills the buffer with the
*	selected waveform. 
*	Waveforms are generated not table-based.
*/

void Oscillator::generate(double* buffer, int osc_frames){
	switch (osc_mode)
	{
	case OSCILLATOR_MODE_SINE:
		for (size_t i = 0; i < osc_frames; i++)
		{
			buffer[i] = sin(osc_phase);
			osc_phase += osc_phase_increment;
			// make sure phase stays between 0 and 2pi
			while (osc_phase >= osc_two_pi)
			{
				osc_phase -= osc_two_pi;
			}
		}
		break;
	case OSCILLATOR_MODE_SAW:
		for (size_t i = 0; i < osc_frames; i++)
		{
			buffer[i] = 1.0 - (2.0 * osc_phase / osc_two_pi);
			osc_phase += osc_phase_increment;
			// make sure phase stays between 0 and 2pi
			while (osc_phase >= osc_two_pi)
			{
				osc_phase -= osc_two_pi;
			}
		}
		break;
	case OSCILLATOR_MODE_SQUARE:
		for (size_t i = 0; i < osc_frames; i++)
		{
			if (osc_phase <= osc_num_pi)
			{
				buffer[i] = 1.0;
			}
			else
			{
				buffer[i] = -1.0;
			}
			// make sure phase stays between 0 and 2pi
			while (osc_phase >= osc_two_pi)
			{
				osc_phase -= osc_two_pi;
			}
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		for (size_t i = 0; i < osc_frames; i++)
		{
			double value = -1.0 + (2.0 * osc_phase / osc_two_pi);
			buffer[i] = 2.0 * (fabs(value) - 0.5);
			osc_phase += osc_phase_increment;
			// make sure phase stays between 0 and 2pi
			while (osc_phase >= osc_two_pi)
			{
				osc_phase -= osc_two_pi;
			}
		}
		break;
	default:
		break;
	}
}