#include "bmx2wav_wav.h"

// load & save Ogg file
namespace Bmx2Wav {
	class HQOgg {
	private:
		// raw data & some metadatas ...
		HQWav *wav;
		std::wstring title;
		std::wstring artist;
	public:
		HQOgg();
		HQOgg(HQWav *wav);
		~HQOgg();

		HQWav* GetPtr();
		void SetPtr(HQWav* wav);
		void SetMetadata(const std::wstring& title, const std::wstring& artist);
		void Release();
		void WriteToFile(const std::wstring& path);
	};
}