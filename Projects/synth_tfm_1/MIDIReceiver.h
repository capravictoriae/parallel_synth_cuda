#ifndef __SYNTH_TFM_1__MIDIRECEIVER__
#define __SYNTH_TFM_1__MIDIRECEIVER__

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"

/**
*	MIDIReceiver class used for handling incoming midi messages.
*/

class MIDIReceiver {
private:
	IMidiQueue mr_midi_queue;
	static const int keys_count = 128;
	int mr_num_keys;
	bool mr_key_status[keys_count];
	int mr_last_note_number;
	double mr_last_freq;
	int mr_last_vel;
	int mr_offset;

	/**
	*	Needed because Oscillator handles frequencies not note numbers
	*	Inline used to be faster (cost in space - more code)
	*	@param	note_num Indicates the note number to convert
	*	@return Returns the frequency in Hz corresponding to a given note number
	*/
	inline double mr_note_num_to_freq(int note_num){
		return 440.0 * pow(2.0, (note_num - 69.0) / 12.0);
	}
public:
	MIDIReceiver() :
		mr_num_keys(0),
		mr_last_note_number(-1),
		mr_last_freq(-1.0),
		mr_last_vel(0),
		mr_offset(0){
			for (size_t i = 0; i < keys_count; i++)
			{
				mr_key_status[i] = false;
			}
	};

	/**
	*	Getter. 
	*	@param	keyIndex Indicates the key to look.
	*	@return Returns true if the key with a given index is currently pressed
	*/
	inline bool getKeyStatus(int keyIndex) const { return mr_key_status[keyIndex]; }

	/**
	*	Getter. 
	*	@return Returns the number of keys currently pressed
	*/
	inline int getNumKeys() const { return mr_num_keys; }

	/**
	*	Getter.
	*	@return Returns the last pressed note number
	*/
	inline int getLastNoteNumber() const { return mr_last_note_number; }

	/**
	*	Getter.
	*	@return Returns the last frequency note 
	*/
	inline double getLastFrequency() const { return mr_last_freq; }

	/**
	*	Getter.
	*	@return Returns the last velocity pressed note
	*/
	inline int getLastVelocity() const { return mr_last_vel; }

	void advance();
	void onMessageReceived(IMidiMsg* midiMessage);

	/**
	*	Discard a number of frames from the midi queue
	*	@param nFrames number of frames to discard
	*/
	inline void Flush(int nFrames) { mr_midi_queue.Flush(nFrames); mr_offset = 0; }

	/**
	*	Resizes the midi queue with the given block size
	*	@param blockSize is the size to resize.
	*/
	inline void Resize(int blockSize) { mr_midi_queue.Resize(blockSize); }

};

#endif