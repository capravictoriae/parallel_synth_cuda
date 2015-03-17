#include "Envelope.h">

/**
*	When envelope is on Attack, decay or release keep track of
*	samples looking if has finished
*	@return the sample 
*/

double Envelope::nextSample(){
	if (currentStage != ENVELOPE_STAGE_OFF && currentStage != ENVELOPE_STAGE_SUSTAIN){
		if (currentSampleIndex == nextStageSampleIndex){
			Envelope_stage stage = static_cast<Envelope_stage>((currentStage + 1) % kNumEnvelope_stages);
			enterStage(stage);
		}
		currentLevel *= multiplier;
		currentSampleIndex++;
	}

	return currentLevel;
}

/**
*	Instead of calculate the logarithmic volumen with exp (<cmath>),
*	calculate it using this method.
*	based on: http://www.musicdsp.org/showone.php?id=189
*/

void Envelope::calculateMultiplier(double startLevel, double endLevel, unsigned long long length){
	multiplier = 1.0 + (log(endLevel) - log(startLevel)) / (length);
}

/**
*	Calculate the behaviour depending on the new stage
*/

void Envelope::enterStage(Envelope_stage newStage){
	currentStage = newStage;
	currentSampleIndex = 0;
	if (currentStage == ENVELOPE_STAGE_OFF ||
		currentStage == ENVELOPE_STAGE_SUSTAIN) {
		nextStageSampleIndex = 0;
	}
	else {
		// calculate how long (in samples) the stage will be
		nextStageSampleIndex = stageValue[currentStage] * sampleRate;
	}
	switch (newStage) {
	case ENVELOPE_STAGE_OFF:
		currentLevel = 0.0;
		multiplier = 1.0;	// not necessary
		break;
	case ENVELOPE_STAGE_ATTACK:
		currentLevel = minimumLevel;
		calculateMultiplier(currentLevel,
			1.0,
			nextStageSampleIndex);
		break;
	case ENVELOPE_STAGE_DECAY:
		currentLevel = 1.0;
		// fmax to make sure doesn't reach zero
		calculateMultiplier(currentLevel,
			fmax(stageValue[ENVELOPE_STAGE_SUSTAIN], minimumLevel),
			nextStageSampleIndex);
		break;
	case ENVELOPE_STAGE_SUSTAIN:
		currentLevel = stageValue[ENVELOPE_STAGE_SUSTAIN];
		multiplier = 1.0;
		break;
	case ENVELOPE_STAGE_RELEASE:
		// We could go from ATTACK/DECAY to RELEASE,
		// so we're not changing currentLevel here.
		calculateMultiplier(currentLevel,
			minimumLevel,
			nextStageSampleIndex);
		break;
	default:
		break;
	}
}

/**
*
*/

