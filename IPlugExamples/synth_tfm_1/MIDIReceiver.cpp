#include "MIDIReceiver.h"

/**
*	Called when the plugin/app receives a midi message.
*	Turn midi note on/off and add to midi queue.
*	@param midiMessage midi message received
*/

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage) {
	IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
	// We're only interested in Note On/Off messages (not CC, pitch, etc.)
	if (status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff) {
		mr_midi_queue.Add(midiMessage);
	}
}

/**
*	advance() is called every sample while generating an audio buffer.
*	Every midi message is processed at the same time a sample is processed
*	using the offset that maintains synchronization
*/

void MIDIReceiver::advance(){

	while (!mr_midi_queue.Empty()) {
		IMidiMsg* midiMessage = mr_midi_queue.Peek();
		if (midiMessage->mOffset > mr_offset)
		{
			break;
		}

		IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
		int noteNumber = midiMessage->NoteNumber();
		int velocity = midiMessage->Velocity();
		// There are only note on/off messages in the queue, see ::OnMessageReceived
		if (status == IMidiMsg::kNoteOn && velocity) {
			if (mr_key_status[noteNumber] == false) {
				mr_key_status[noteNumber] = true;
				mr_num_keys += 1;
			}
			// A key pressed later overrides any previously pressed key:
			if (noteNumber != mr_last_note_number) {
				mr_last_note_number = noteNumber;
				mr_last_freq = mr_note_num_to_freq(mr_last_note_number);
				mr_last_vel = velocity;
			}
		}
		else {
			if (mr_key_status[noteNumber] == true) {
				mr_key_status[noteNumber] = false;
				mr_num_keys -= 1;
			}
			// If the last note was released, nothing should play:
			if (noteNumber == mr_last_note_number) {
				mr_last_note_number = -1;
				mr_last_freq = -1;
				mr_last_vel = 0;
			}
		}
		mr_midi_queue.Remove();
	}
	mr_offset++;

}