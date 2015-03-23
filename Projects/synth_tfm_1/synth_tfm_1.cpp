#include "synth_tfm_1.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"

#include <algorithm>
#include <math.h>
#include <iostream>
#include <chrono>

#include <omp.h>


const int kNumPrograms = 5;

enum EParams
{
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kKeybX = 1,
  kKeybY = 0
};

/**
*	Main constructor with all graphic elements.
*/

synth_tfm_1::synth_tfm_1(IPlugInstanceInfo instanceInfo)
	: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1)
{
	TRACE;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Init Kernel
	FILE *fp;
	char filename[] = "./first_kernel.cl";
	char *source_str;
	size_t source_size;

	// Load the source code containing the kernel 
	fp = fopen(filename, "r");
	if (!fp){
		std::cerr << "Failed to load kernel" << std::endl;
		exit(1);
	}
	source_str = (char*)malloc(0x100000);
	source_size = fread(source_str, 1, 0x100000, fp);
	fclose(fp);

	//Get an OpenCL platform
	cl_platform_id cpPlatform;
	clGetPlatformIDs(1, &cpPlatform, NULL);

	// Get a GPU device
	cl_device_id cdDevice;
	clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
	char cBuffer[1024];
	clGetDeviceInfo(cdDevice, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
	//printf("CL_DEVICE_NAME: %s\n", cBuffer);
	clGetDeviceInfo(cdDevice, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
	//printf("CL_DRIVER_VERSION: %s\n\n", cBuffer);

	// Create a context to run OpenCL enabled GPU
	GPUContext = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);

	// Create a command-queue on the GPU device
	cqCommandQueue = clCreateCommandQueue(GPUContext, cdDevice, 0, NULL);

	///////////////////////////////////////////////////////////////////////////////////////////

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	//pGraphics->AttachPanelBackground(&COLOR_BLACK);
	pGraphics->AttachBackground(KEYS_BG_ID, KEYS_BG_FN);

	IBitmap whiteKeyBitmap = pGraphics->LoadIBitmap(KEYS_WHITE_ID, KEYS_WHITE_FN, 6);
	IBitmap blackKeyBitmap = pGraphics->LoadIBitmap(KEYS_BLACK_ID, KEYS_BLACK_FN);

	//                            C#     D#          F#      G#      A#
	int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
	// No destructor needed. Memory ownership of IKeyboardControl goes to graphics system and it manage it.

	mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyBitmap, &blackKeyBitmap, keyCoordinates);

	pGraphics->AttachControl(mVirtualKeyboard);

	AttachGraphics(pGraphics);
	CreatePresets();

	// connect midi receiver with envelopes 

	mMIDIReceiver.noteOn.Connect(this, &synth_tfm_1::onNoteOn);
	mMIDIReceiver.noteOff.Connect(this, &synth_tfm_1::onNoteOff);
}

synth_tfm_1::~synth_tfm_1() {}

void synth_tfm_1::CreatePresets() {
	
}

/**
*	This is where the actual processing is done.
*	@param	inputs input buffer for processing (not used for a synth, no incoming audio)
*	@param	outputs output buffer for processing
*	@param	nFrames number of frames to process
*/

void synth_tfm_1::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
	// Mutex is already locked for us.

	// Time measure
	auto begin = omp_get_wtime();

	double *left_output = outputs[0];
	double *right_output = outputs[1];

	processVirtualKeyboard();

	for (size_t i = 0; i < nFrames; i++)
	{
		mMIDIReceiver.advance();
		int velocity = mMIDIReceiver.getLastVelocity();
		if (velocity > 0){
			mOscillator.set_freq(mMIDIReceiver.getLastFrequency());
			mOscillator.set_muted(false);
		}
		else {
			mOscillator.set_muted(true);
		}
		left_output[i] = right_output[i] = mOscillator.nextSample() * mEnvelope.nextSample() * velocity / 127.0;
	}

	mMIDIReceiver.Flush(nFrames);

	// time measure
	auto final_var = omp_get_wtime() - begin;
	if (final_var > 0){
		DBGMSG("%f", final_var);
	}

	
}

/**
*	Called when the sample rate changes.
*/

void synth_tfm_1::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.set_samp_rate(GetSampleRate()); // GetSampleRate inherited from IPlugBase class
  mEnvelope.setSampleRate(GetSampleRate());
}

/**
*	Called whenever an element of the GUI / MIDI changes
*	@param	paramIdx ID of the element that changed.
*/

void synth_tfm_1::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);

}

/**
*	Called whenever a MIDI message is received.
*	@param	pMsg MIDI message received
*/

void synth_tfm_1::ProcessMidiMsg(IMidiMsg* pMsg){
	mMIDIReceiver.onMessageReceived(pMsg);
	// Mark as dirty for performance reasons
	// Repaints the keyboard on every cicle
	mVirtualKeyboard->SetDirty();
}

/**
*	Process the clicks on the virtualKeyboard
*/

void synth_tfm_1::processVirtualKeyboard(){
	IKeyboardControl* vKeyboard = (IKeyboardControl*)mVirtualKeyboard;
	int vkNoteNumber = vKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;

	if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && vkNoteNumber != lastVirtualKeyboardNoteNumber) {
		// The note number has changed from a valid key to something else (valid key or nothing). Release the valid key:
		IMidiMsg midiMessage;
		midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
		mMIDIReceiver.onMessageReceived(&midiMessage);
	}

	if (vkNoteNumber >= virtualKeyboardMinimumNoteNumber && vkNoteNumber != lastVirtualKeyboardNoteNumber) {
		// A valid key is pressed that wasn't pressed the previous call. Send a "note on" message to the MIDI receiver:
		IMidiMsg midiMessage;
		midiMessage.MakeNoteOnMsg(vkNoteNumber, vKeyboard->GetVelocity(), 0);
		mMIDIReceiver.onMessageReceived(&midiMessage);
	}

	lastVirtualKeyboardNoteNumber = vkNoteNumber;
}