#include "sndfile.hh"

#define FREQUENCY 44100
#define MIXERSIZE 1296
#define CHUNKSIZE 102400	// 100kb

class Audio {
private:
	int* buf;
	int sample;

	// for recording (file related)
	int size;
	int pos;

	// tag
	std::string title;
	std::string artist;
public:
	Audio() : buf(0), sample(0), pos(0), size(0) {};
	~Audio() { Release(); };
	void Release();

	// file is loaded/saved in 44100Hz / 32bit sound.
	bool LoadFile(const std::string& path);
	bool SaveFile(const std::string& path);

	int Length();		// in microsecond
	int Get(int pos);

	void Normalize();
	void Reset() { pos = 0; }
	void Add(int sample);
	void SetTitle(const std::string& s);
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