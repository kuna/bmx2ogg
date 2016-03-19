#pragma once

// for unicode filename support
#ifdef _WIN32
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
typedef wchar_t* LPCWSTR;
#endif
#include <sndfile.hh>

#include "bmx2wav_common.h"
#include <limits.h>
#include <string>

#define FREQUENCY 44100
#define MIXERSIZE 1296
#define CHUNKSIZE 512000	// 0.5m samples


/*
 * INT PRECISION make more detailed encoding, but that needs more memory and time.
 * use it at your own risk.
 */
//#define USE_INTPRECISION
#ifdef USE_INTPRECISION
typedef int Sample;
typedef long long int MSample;
#define MSAMPLE_MAX INT_MAX
#define MSAMPLE_MIN INT_MIN
#else
typedef short Sample;
typedef int MSample;
#define MSAMPLE_MAX SHRT_MAX
#define MSAMPLE_MIN SHRT_MIN
#endif


template <typename T>
class AudioData {
	T* samples;
	int samplesize;
	int pos;
public:
	AudioData<T>() : samples(0) { Create(); }
	~AudioData<T>() { Release(); }
	void Release() {
		if (samples) { free(samples); }
		samples = 0; samplesize = 0; pos = 0;
	}
	void Create(int size = CHUNKSIZE) {
		Release();
		samplesize = size;
		samples = (T*)malloc(sizeof(T) * size);
	}
	void Clear() { Release(); Create(); }
	void Add(T v) {
		if (pos >= samplesize) {
			samplesize += CHUNKSIZE;
			samples = (T*)realloc(samples, sizeof(T) * samplesize);
		}
		samples[pos++] = v;
	}
	T* GetPtr() { return samples; }
	int GetSampleCount() { return pos; }
	void SetSampleCount(int v) { pos = v; }
	T& operator[](int p) { return samples[p]; }
};

class Audio {
private:
	AudioData<Sample> buf;

	// tag
	std::string title;
	std::string artist;
	std::string genre;
	double quality = 0.95;
	char* albumart;
	size_t albumart_size;
public:
	Audio();
	~Audio();
	void Release();

	// file is loaded/saved in 44100Hz / 32bit sound.
	bool LoadFile(const std::string& path);
	bool SaveFile(const std::string& path, int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16);

	int Length();		// in microsecond
	int SampleLength();
	Sample Get(int pos);

	void Create(int size = CHUNKSIZE);
	void Add(Sample sample);
	void SetQuality(double v) { quality = v; }
	void SetTitle(const std::string& s) { title = s; };
	void SetArtist(const std::string& s) { artist = s; };
	void SetGenre(const std::string& s) { genre = s; }
	void SetCoverArt(char *p, size_t s) { albumart = p; albumart_size = s; };

	// a little modification
	void ChangeRate(double d);
	void ChangeLength(double d);
	void ChangePitch(double d);
};



#include <vector>

class Mixer {
private:
	struct Channel {
		Audio audio;
		bool mixing;	// is currently mixing?
		int tick;		// mixing started tick
	};
	Channel channel[MIXERSIZE];
	int tick;			// current tick
	bool IsStopped(int c);
	int RemainingTick(int c);
	double ratio;

	// you may need this if you're going to normalize.
	AudioData<MSample> samples;
public:
	Mixer() {};
	~Mixer() { Release(); };
	void Release();
	bool LoadFile(int c, const std::string& path);
	Audio* GetAudio(int c);
	void SetRatio(double v) { ratio = v; }

	void Start(int c);
	void Stop(int c);
	void StartMixing();
	int GetTick();
	void Mix(int t = 1);			// tick to next sample, and returns current sample
	void MixUntilEnd();				// mix until all audio stops playing
	bool IsAllAudioStopped();
	int Flush(Audio *out, bool normalized = false, double *ratio = 0);		// returns flushed samples
};

// pitch shifter
namespace SoundUtil {
	template <typename T>
	void smbPitchShift(float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate, T *indata, T *outdata);
}
