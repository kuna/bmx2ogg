#pragma once

#include <vector>

template <typename WAV>
class BmsWavResource {
public:
	class BmsWav {
	private:
		WAV *wav;
	public:
		BmsWav();
		~BmsWav();
		bool IsLoaded();
		WAV* GetWAV();
		void SetWAV(WAV* wav);
	};

public:
	BmsWavResource();
	bool IsLoaded(int channel);
	WAV* GetWAV(int channel);
	void SetWAV(int channel, WAV* wav);
private:
	BmsWav wav_table[1296];
};

template <typename WAV>
BmsWavResource<WAV>::BmsWav::BmsWav() { wav = 0; }

template <typename WAV>
BmsWavResource<WAV>::BmsWav::~BmsWav() { delete wav; }

template <typename WAV>
bool BmsWavResource<WAV>::BmsWav::IsLoaded() { return wav; }

template <typename WAV>
WAV* BmsWavResource<WAV>::BmsWav::GetWAV() { return wav; }

template <typename WAV>
void BmsWavResource<WAV>::BmsWav::SetWAV(WAV *wav) { this->wav = wav; }

// -------------------------------------------------

template <typename WAV>
BmsWavResource<WAV>::BmsWavResource() {  }

template <typename WAV>
WAV* BmsWavResource<WAV>::GetWAV(int channel) {
	if (wav_table[channel].IsLoaded())
		return wav_table[channel].GetWAV();
	else
		return 0;
}

template <typename WAV>
void BmsWavResource<WAV>::SetWAV(int channel, WAV* wav) {
	wav_table[channel].SetWAV(wav);
}

template <typename WAV>
bool BmsWavResource<WAV>::IsLoaded(int channel) {
	return wav_table[channel].IsLoaded();
}