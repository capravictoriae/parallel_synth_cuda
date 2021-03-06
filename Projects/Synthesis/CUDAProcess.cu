#include "CUDAProcess.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "curand.h"
#include "curand_kernel.h"

#include <cmath>

__device__ float cuda_pbs(float t, float twoPI, float mPhaseIncrement){
	double dt = mPhaseIncrement / twoPI;
	if (t < dt) {
		t /= dt;
		return t + t - t*t - 1.0;
	}
	else if (t > 1.0 - dt) {
		t = (t - 1.0) / dt;
		return t*t + t + t + 1.0;
	}
	else return 0.0;
}

__device__ float cuda_osc_next(float d_phaseInc, float phaseIncrement, float twoPI, float mPI, bool muted, OscillatorMode mode){

	float value = 0.0;
	if (muted){
		return value;
	}

	float t = d_phaseInc / twoPI;
	float t_aux = fmod(t + 0.5, 1.0);
	float lastOutput = 0.0;

	switch (mode)
	{
	case OSCILLATOR_MODE_SINE:
		value = sin(d_phaseInc);
		break;
	case OSCILLATOR_MODE_SAW:
		value = ((2.0 * d_phaseInc / twoPI) - 1.0) - cuda_pbs(t, twoPI, phaseIncrement);
		break;
	case OSCILLATOR_MODE_SQUARE:
		if (d_phaseInc <= mPI) {
			value = 1.0 + cuda_pbs(t, twoPI, phaseIncrement) - cuda_pbs(t, twoPI, phaseIncrement);
		}
		else {
			value = -1.0 + cuda_pbs(t, twoPI, phaseIncrement) - cuda_pbs(t, twoPI, phaseIncrement);
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		if (d_phaseInc <= mPI) {
			value = 1.0 + cuda_pbs(t, twoPI, phaseIncrement) - cuda_pbs(t, twoPI, phaseIncrement);
		}
		else {
			value = -1.0 + cuda_pbs(t, twoPI, phaseIncrement) - cuda_pbs(t, twoPI, phaseIncrement);
		}

		value = phaseIncrement * value + (1 - phaseIncrement) * lastOutput;
		// TODO precalculate?
		// REMOVE triangle calc if gets worse performance
		lastOutput = value;

		//value = -1.0 + (2.0 * d_phaseInc / twoPI);
		//value = 2.0 * (fabs(value) - 0.5);
		break;
	default:
		break;
	}

	return value;

}

__global__ void cuda_process(float* d_lbuffer, float* d_rbuffer, float* d_phaseIncArray, float phaseIncrement, int d_velocity, float d_lastfreq, float twoPI, float mPI, 
	OscillatorMode mode){

	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	// TODO shared
	/*
	int num_partials = (int)(22050.0 / d_lastfreq);
	float mPart = mPart * 0.1 + d_lastfreq / 10000.0;
	mPart = mPart * 0.1 + d_lastfreq*d_lastfreq / 50000000.0;
	mPart = mPart * 1.01 / (1.01 - (d_velocity / (1.0f + 2.0*10.0f)) / 5.0f);
	*/

	//float d_osc_freq = 0.0;
	bool d_osc_muted = false;

	if (d_velocity > 0){
		//d_osc_freq = d_lastfreq;
		d_osc_muted = false;
	}
	else {
		d_osc_muted = true;
	}

	

	d_lbuffer[idx] = d_rbuffer[idx] = cuda_osc_next(d_phaseIncArray[idx], phaseIncrement, twoPI, mPI, d_osc_muted, mode) * d_velocity / 127.0;

}

void CUDAProcess::Process(double* lbuffer, double* rbuffer, int lastvel, double lastfreq, double mPI, double twoPI, 
	double mSampleRate, Envelope* mEnvelope, Filter* mFilter, OscillatorMode mode){

	const int cuda_nFrames = 1024;

	// convert double buffer to float fbuffer
	//---------------------------------------------------------------
	//	TODO REMOVE this with compute capability graphics card >= 2.0
	//---------------------------------------------------------------
	float h_lbuffer[cuda_nFrames];
	float h_rbuffer[cuda_nFrames];
	for (size_t i = 0; i < cuda_nFrames; i++)
	{
		h_lbuffer[i] = (float)lbuffer[i];
		h_rbuffer[i] = (float)rbuffer[i];
	}

	// calculate al phaseIncrements
	double phaseIncArray[cuda_nFrames];
	double mPhaseIncrement_aux = lastfreq * 2 * mPI / mSampleRate;
	// get previous array last value
	double mPhase_aux = lastPhaseIncrementValue + mPhaseIncrement_aux;
	//double mPhase_aux = mOscillator.getmPhase();
	for (size_t i = 0; i < cuda_nFrames; i++)
	{
		phaseIncArray[i] = mPhase_aux;
		mPhase_aux += mPhaseIncrement_aux;
		while (mPhase_aux >= twoPI) {
			mPhase_aux -= twoPI;
		}
	}

	//---------------------------------------------------------------
	//	TODO REMOVE this with compute capability graphics card >= 2.0
	//---------------------------------------------------------------	
	float phaseIncArray_f[cuda_nFrames];
	for (size_t i = 0; i < cuda_nFrames; i++)
	{
		phaseIncArray_f[i] = (float)phaseIncArray[i];
	}

	// ---------------------------------------- CUDA ------------------------------------------------

	float* d_lbuffer;
	float* d_rbuffer;
	float* d_phaseIncArray;
	
	int d_size = cuda_nFrames*sizeof(float);
	cudaMalloc((void **)&d_lbuffer, d_size);
	cudaMalloc((void **)&d_rbuffer, d_size);
	cudaMalloc((void **)&d_phaseIncArray, d_size);

	cudaMemcpy(d_lbuffer, h_lbuffer, d_size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_rbuffer, h_rbuffer, d_size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_phaseIncArray, phaseIncArray_f, d_size, cudaMemcpyHostToDevice);

	// change depending on available CUDA cores
	int block_size = 16;
	int n_blocks = cuda_nFrames / block_size + (cuda_nFrames % block_size == 0 ? 0 : 1);

	cuda_process << < n_blocks, block_size >> >(d_lbuffer, d_rbuffer, d_phaseIncArray,
		(float)mPhaseIncrement_aux, lastvel, (float)lastfreq, (float)twoPI, (float)mPI, mode);

	cudaMemcpy(h_lbuffer, d_lbuffer, d_size, cudaMemcpyDeviceToHost);
	cudaMemcpy(h_rbuffer, d_rbuffer, d_size, cudaMemcpyDeviceToHost);

	cudaFree(d_lbuffer);
	cudaFree(d_rbuffer);
	cudaFree(d_phaseIncArray);

	// copy float fbuffer to double buffer
	//---------------------------------------------------------------
	//	TODO REMOVE this with compute capability graphics card >= 2.0
	//---------------------------------------------------------------
	for (size_t i = 0; i < cuda_nFrames; i++)
	{
		lbuffer[i] = (double)h_lbuffer[i];
		rbuffer[i] = (double)h_rbuffer[i];
	}

	// calculate envelop stages
	double envelopeStage[cuda_nFrames];
	for (size_t i = 0; i < cuda_nFrames; i++)
	{
		envelopeStage[i] = mEnvelope->nextSample();
	}

	// add envelope volume to buffers
	for (size_t i = 0; i < cuda_nFrames; i++)
	{
		lbuffer[i] = mFilter->process(lbuffer[i] * envelopeStage[i]);
		rbuffer[i] = mFilter->process(rbuffer[i] * envelopeStage[i]);
	}
	
	
	/*
	for (int i = 0; i < cuda_nFrames; ++i) {

		if (lastvel > 0) {
			mOscillator.setFrequency(lastfreq);
			mOscillator.setMuted(false);
		}
		else {
			mOscillator.setMuted(true);
		}

		//lbuffer[i] = rbuffer[i] = mFilter.process(mOscillator.nextSample() * mEnvelope->nextSample() * lastvel / 127.0);
		//lbuffer[i] = rbuffer[i] = mOscillator.nextSample(phaseIncArray_f[i]) * lastvel / 127.0;
		lbuffer[i] = rbuffer[i] = mOscillator.nextSample(phaseIncArray[i]) * lastvel / 127.0;
	}
	*/
	
	// save last increment value for next chunk
	lastPhaseIncrementValue = phaseIncArray[cuda_nFrames - 1];

}

