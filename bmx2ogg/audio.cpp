#include "audio.h"
#include "bmx2wav_common.h"
#include <algorithm>



bool Audio::LoadFile(const std::string& path) {
	Release();
#ifdef _WIN32
	wchar_t wpath[1024];
	ENCODING::utf8_to_wchar(path.c_str(), wpath, 1024);
	SndfileHandle file(wpath, SFM_READ, 0, 2, FREQUENCY);
#else
	SndfileHandle file(path.c_str(), SFM_READ, 0, 2, FREQUENCY);
#endif

	if (file.error() != 0) {
		printf("%s\n", sf_strerror(file.rawHandle()));
		return false;
	}

	sample = file.channels() * file.frames();
	size = sample * sizeof(Sample);
	buf = (Sample*)malloc(size);
	file.read(buf, sample);

	return true;
}

#define SAMPLECHUNKSIZE 1024
bool Audio::SaveFile(const std::string& path, int format) {
#ifdef _WIN32
	wchar_t wpath[1024];
	ENCODING::utf8_to_wchar(path.c_str(), wpath, 1024);
	SndfileHandle file(wpath, SFM_WRITE, format, 2, FREQUENCY);
#else
	SndfileHandle file(path.c_str(), SFM_WRITE, format, 2, FREQUENCY);
#endif

	if (!title.empty()) sf_set_string(file.rawHandle(), SF_STR_TITLE, title.c_str());
	if (!artist.empty()) sf_set_string(file.rawHandle(), SF_STR_ARTIST, artist.c_str());

	// set quality
	sf_command(file.rawHandle(), SFC_SET_VBR_ENCODING_QUALITY, &quality, sizeof(double));

	// write file
	// (cut into pieces)
	for (int i = 0; i < sample;) {
		size_t nchunk = sample - i;
		if (nchunk > SAMPLECHUNKSIZE) nchunk = SAMPLECHUNKSIZE;
		size_t r = file.write(buf + i, nchunk);
		if (r != nchunk) {
			printf("%s\n", sf_strerror(file.rawHandle()));
			return false;
		}
		i += nchunk;
	}

	return true;
}

Audio::Audio()
	: buf(0), sample(0), pos(0), size(0)
{}

Audio::~Audio() {
	Release();
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

Sample Audio::Get(int pos) {
	if (!buf) return 0;
	if (pos >= sample) return 0;
	return buf[pos];
}

int Audio::SampleLength() {
	return sample;
}

int Audio::Length() {
	return sample * 1000 / FREQUENCY / 2;
}

void Audio::Add(Sample sample) {
	if (buf) {
		if (pos >= size / sizeof(int)) {
			size += CHUNKSIZE;
			buf = (Sample*)realloc(buf, size);
		}
		buf[pos++] = sample;
		if (pos > this->sample) this->sample = pos;
	}
}

void Audio::Create(int size) {
	Release();
	buf = (Sample*)malloc(size);
	this->size = size;
	sample = 0;
	pos = 0;
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
	ratio = 1.0;
	for (int i = 0; i < MIXERSIZE; i++) {
		channel[i].mixing = false;
		channel[i].tick = 0;
	}
}

int Mixer::GetTick() {
	return tick;
}

void Mixer::Mix(int t) {
	for (int c = 0; c < t; c++) {
		MSample r = 0;
		for (int i = 0; i < MIXERSIZE; i++) {
			if (channel[i].mixing) {
				r += channel[i].audio.Get(tick - channel[i].tick);
			}
		}

		// update tick and add
		tick++;
		samples.push_back(r);
	}
}

void Mixer::MixUntilEnd() {
	while (!IsAllAudioStopped()) {
		// 2 channels
		Mix(2);
	}
}

bool Mixer::IsStopped(int c) {
	if (channel[c].audio.SampleLength() <= tick - channel[c].tick)
		channel[c].mixing = false;
	return !channel[c].mixing;
}

int Mixer::Flush(Audio *out, bool normalize, double *r) {
	int c = samples.size();
	if (normalize) {
		double ratio = 1.0;
		for (int i = 0; i < c; i++) {
			if (samples[i] > MSAMPLE_MAX) ratio = std::max(ratio, MSAMPLE_MAX / (double)samples[i]);
			else if (samples[i] < MSAMPLE_MIN) ratio = std::max(ratio, MSAMPLE_MIN / (double)samples[i]);
		}
		for (int i = 0; i < c; i++) {
			samples[i] *= ratio;
		}
		if (r) *r = ratio;
	}

	// cut and store samples to out
	for (int i = 0; i < c; i++) {
		if (samples[i] > MSAMPLE_MAX) samples[i] = MSAMPLE_MAX;
		else if (samples[i] < MSAMPLE_MIN) samples[i] = MSAMPLE_MIN;
		out->Add(samples[i]);
	}

	samples.clear();
	return c;
}

bool Mixer::IsAllAudioStopped() {
	for (int i = 0; i < MIXERSIZE; i++) {
		if (!IsStopped(i)) return false;
	}
	return true;
}