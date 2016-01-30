#include "bmx2wav_wav.h"
#include "bmx2wav_common.h"

// load & save Ogg file
namespace Bmx2Wav {
	class HQOgg {
	private:
		// raw data & some metadatas ...
		HQWav *wav;
		std::string title;
		std::string artist;
	public:
		HQOgg();
		HQOgg(HQWav *wav);
		~HQOgg();

		HQWav* GetPtr();
		void SetPtr(HQWav* wav);
		void SetMetadata(const std::string& title, const std::string& artist);
		void Release();
		void WriteToFile(const std::string& path);
	};
}
