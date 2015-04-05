#include "Oscillator.h"

#include "MIDIReceiver.h"

#include <vector>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

__global__ void generate_oscillator_sine(float *d_buffer, float* d_phaseIncArray, OscillatorMode mode, float mPI, float twoPI){

	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	switch (mode)
	{
	case OSCILLATOR_MODE_SINE:
		d_buffer[idx] = sin(d_phaseIncArray[idx]);
		break;
	case OSCILLATOR_MODE_SAW:
		d_buffer[idx] = 1.0 - (2.0 * d_phaseIncArray[idx] / twoPI);
		break;
	case OSCILLATOR_MODE_SQUARE:
		if (d_phaseIncArray[idx] <= mPI) {
			d_buffer[idx] = 1.0;
		}
		else {
			d_buffer[idx] = -1.0;
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		double value = -1.0 + (2.0 * d_phaseIncArray[idx] / twoPI);
		d_buffer[idx] = 2.0 * (fabs(value) - 0.5);
		break;
	default:
		break;
	}
}

void Oscillator::setMode(OscillatorMode mode) {
    mOscillatorMode = mode;
}

void Oscillator::setFrequency(double frequency) {	
    mFrequency = frequency;
    updateIncrement();
}

void Oscillator::setSampleRate(double sampleRate) {
    mSampleRate = sampleRate;
    updateIncrement();
}

void Oscillator::updateIncrement() {
    mPhaseIncrement = mFrequency * 2 * mPI / mSampleRate;
}

void Oscillator::generate(double* buffer, int nFrames) {
    const double twoPI = 2 * mPI;
	const int nFrames_size = 256;
	
	// convert double buffer to float fbuffer
	float fbuffer[nFrames_size];
	for (size_t i = 0; i < nFrames_size; i++)
	{
		fbuffer[i] = (float)buffer[i];
	}

	// calculate al phaseIncrements
	float phaseIncArray[nFrames_size];
	for (size_t i = 0; i < nFrames_size; i++)
	{
		phaseIncArray[i] = mPhase;
		mPhase += mPhaseIncrement;
		while (mPhase >= twoPI) {
			mPhase -= twoPI;
		}
	}

	
	// ----------------------- CUDA ----------------------------

	float* d_buffer;
	float* d_phaseIncArray;

	int d_size = nFrames*sizeof(float);
	cudaMalloc((void **) &d_buffer, d_size);
	cudaMalloc((void **) &d_phaseIncArray, d_size);

	cudaMemcpy(d_buffer, fbuffer, d_size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_phaseIncArray, phaseIncArray, d_size, cudaMemcpyHostToDevice);

	int block_size = 4;
	int n_blocks = nFrames / block_size + (nFrames % block_size == 0 ? 0 : 1);

	generate_oscillator_sine << < n_blocks, block_size >> >(d_buffer, d_phaseIncArray, mOscillatorMode, (float)mPI, (float)twoPI);

	cudaMemcpy(fbuffer, d_buffer, d_size, cudaMemcpyDeviceToHost);

	cudaFree(d_buffer);

	// copy float fbuffer to double buffer
	for (size_t i = 0; i < nFrames_size; i++)
	{
		buffer[i] = (double)fbuffer[i];
	}
	

	// ----------------------- CUDA ----------------------------

	/*
    switch (mOscillatorMode) {
        case OSCILLATOR_MODE_SINE:
            for (int i = 0; i < nFrames; i++) {
                buffer[i] = sin(phaseIncArray[i]);
            }
            break;
        case OSCILLATOR_MODE_SAW:
            for (int i = 0; i < nFrames; i++) {
				buffer[i] = 1.0 - (2.0 * phaseIncArray[i] / twoPI);
            }
            break;
        case OSCILLATOR_MODE_SQUARE:
            for (int i = 0; i < nFrames; i++) {
				if (phaseIncArray[i] <= mPI) {
                    buffer[i] = 1.0;
                } else {
                    buffer[i] = -1.0;
                }
            }
            break;
        case OSCILLATOR_MODE_TRIANGLE:
            for (int i = 0; i < nFrames; i++) {
				double value = -1.0 + (2.0 * phaseIncArray[i] / twoPI);
                buffer[i] = 2.0 * (fabs(value) - 0.5);
            }
            break;
    }
	*/
	

}

/*
double Oscillator::nextSample() {
	double value = 0.0;
	if (isMuted) return value;

	switch (mOscillatorMode) {
	case OSCILLATOR_MODE_SINE:
		value = sin(mPhase);
		break;
	case OSCILLATOR_MODE_SAW:
		value = 1.0 - (2.0 * mPhase / twoPI);
		break;
	case OSCILLATOR_MODE_SQUARE:
		if (mPhase <= mPI) {
			value = 1.0;
		}
		else {
			value = -1.0;
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		value = -1.0 + (2.0 * mPhase / twoPI);
		value = 2.0 * (fabs(value) - 0.5);
		break;
	}
	mPhase += mPhaseIncrement;
	while (mPhase >= twoPI) {
		mPhase -= twoPI;
	}

	return value;
}
*/

double Oscillator::nextSample(double mPhase_processed) {
	double value = 0.0;
	if (isMuted) return value;

	switch (mOscillatorMode) {
	case OSCILLATOR_MODE_SINE:
		value = sin(mPhase_processed);
		break;
	case OSCILLATOR_MODE_SAW:
		value = 1.0 - (2.0 * mPhase_processed / twoPI);
		break;
	case OSCILLATOR_MODE_SQUARE:
		if (mPhase_processed <= mPI) {
			value = 1.0;
		}
		else {
			value = -1.0;
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		value = -1.0 + (2.0 * mPhase_processed / twoPI);
		value = 2.0 * (fabs(value) - 0.5);
		break;
	}

	/*
	mPhase += mPhaseIncrement;
	while (mPhase >= twoPI) {
		mPhase -= twoPI;
	}
	*/

	return value;
}