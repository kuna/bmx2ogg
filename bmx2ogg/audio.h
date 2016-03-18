#pragma once

// for unicode filename support
#ifdef _WIN32
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
typedef wchar_t* LPCWSTR;
#endif
#include "sndfile.hh"

#include <string>

#define FREQUENCY 44100
#define MIXERSIZE 1296
#define CHUNKSIZE 10240000	// 10mb

class Audio {
private:
	int* buf;
	size_t sample;
	double quality = 0.95;

	// for recording (file related)
	size_t size;
	size_t pos;

	// tag
	std::string title;
	std::string artist;
public:
	Audio();
	~Audio();
	void Release();

	// file is loaded/saved in 44100Hz / 32bit sound.
	bool LoadFile(const std::string& path);
	bool SaveFile(const std::string& path, int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16);

	int Length();		// in microsecond
	int Get(int pos);

	void Create(int size = CHUNKSIZE);
	void Normalize();
	void Reset() { pos = 0; }
	void Add(int sample);
	void SetQuality(double v) { quality = v; }
	void SetTitle(const std::string& s) { title = s; };
	void SetArtist(const std::string& s) { artist = s; };
};




class Mixer {
private:
	struct Channel {
		Audio audio;
		bool mixing;	// is currently mixing?
		int tick;		// mixing started tick
	};
	Channel channel[MIXERSIZE];
	int tick;			// current tick
public:
	Mixer() {};
	~Mixer() { Release(); };
	void Release();
	bool LoadFile(int c, const std::string& path);
	Audio* GetAudio(int c);

	void Start(int c);
	void Stop(int c);
	void StartMixing();
	int GetTick();
	int Tick();			// tick to next sample, and returns current sample
};