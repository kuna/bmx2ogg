#include "audio.h"

bool Audio::LoadFile(const std::string& path) {
	Release();
	// COMMENT: check unicode validation
	SndfileHandle file(path.c_str(), 0, 0, 2, FREQUENCY);

	if (file.error() != 0)
		return false;

	sample = file.channels() * file.frames();
	size = sample * sizeof(int);
	buf = (int*)malloc(size);
	file.read((int*)buf, sample);

	return true;
}

bool Audio::SaveFile(const std::string& path) {
	SndfileHandle file(path.c_str(), 0, 0, 2, FREQUENCY);

	if (!title.empty()) sf_set_string(file.rawHandle(), SF_STR_TITLE, title.c_str());
	if (!artist.empty()) sf_set_string(file.rawHandle(), SF_STR_ARTIST, artist.c_str());

	file.write(buf, sample);

	return true;
}

void Audio::Release() {
	if (buf) {
		free(buf);
		buf = 0;
		sample = 0;
		size = 0;
		pos = 0;
	}
}

int Audio::Get(int pos) {
	if (!buf) return 0;
	if (pos > sample) return 0;
	return buf[pos];
}

int Audio::Length() {
	return sample * 1000 / FREQUENCY / 2;
}

void Audio::Add(int sample) {
	if (buf) {
		if (pos >= size) {
			size += CHUNKSIZE;
			buf = (int*)realloc(buf, size);
		}
		buf[pos++] = sample;
	}
}

void Audio::Normalize() {
	// TODO
}




void Mixer::Release() {
	for (int i = 0; i < MIXERSIZE; i++) {
		channel[i].audio.Release();
	}
}

Audio* Mixer::GetAudio(int c) {
	return &channel[c].audio;
}

bool Mixer::LoadFile(int c, const std::string& path) {
	return channel[c].audio.LoadFile(path);
}

void Mixer::Start(int c) {
	channel[c].mixing = true;
	channel[c].tick = tick;
}

void Mixer::Stop(int c) {
	channel[c].mixing = false;
}

void Mixer::StartMixing() {
	// initalize
	tick = 0;
	for (int i = 0; i < MIXERSIZE; i++) {
		channel[i].mixing = false;
		channel[i].tick = 0;
	}
}

int Mixer::GetTick() {
	return tick;
}

int Mixer::Tick() {
	int r = 0;
	for (int i = 0; i < MIXERSIZE; i++) {
		if (channel[i].mixing) {
			r += channel[i].audio.Get(tick - channel[i].tick);
		}
	}

	// TODO: do normalize ?

	// update tick and return
	tick++;
	return r;
}