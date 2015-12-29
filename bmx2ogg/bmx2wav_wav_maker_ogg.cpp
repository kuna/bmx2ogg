#include <stdio.h>
#include <errno.h>

#pragma warn -8080
#pragma warn -8071
#include "vorbis/vorbisfile.h"
// #pragma warn .8080
#pragma warn .8071

#include "exception.h"
#include "bmx2wav_wav_maker.h"
#include "bmx2wav_common.h"

using namespace Bmx2Wav;
#pragma warning(disable:4996)

namespace {
	class FileHolder{
	public:
		explicit FileHolder(const std::wstring& filename) :
			filename_(filename),
			file_(_wfopen(filename.c_str(), L"rb")) {
			if (file_ == NULL) {
				throw Bmx2WavInvalidFile(filename, errno);
			}
		}

		~FileHolder() {
			if (file_ != NULL) {
				fclose(file_);
			}
		}

		const std::wstring& GetFileName(void) {
			return filename_;
		}

		FILE* GetPtr(void) {
			return file_;
		}

	private:
		std::wstring filename_;
		FILE*       file_;
	};

	class OggVorbisFileHolder{
	public:
		explicit OggVorbisFileHolder(FileHolder& file_holder) :
			file_holder_(file_holder) {
			if (ov_open(file_holder.GetPtr(), &vf_, NULL, 0) < 0) {
				throw Bmx2WavCannotReadFile(file_holder.GetFileName());
			}
		}

		~OggVorbisFileHolder() {
			ov_clear(&vf_);
		}

		FileHolder& GetFileHolder(void) {
			return file_holder_;
		}

		OggVorbis_File* GetPtr(void) {
			return &vf_;
		}

	private:
		FileHolder& file_holder_;
		OggVorbis_File vf_;
	};
}

namespace Bmx2Wav {
	namespace DataReader {
		// -- DataReader::SixteenBitOgg --------------------------------------
		class SixteenBitOgg : virtual public Base {
		public:
			explicit SixteenBitOgg(OggVorbisFileHolder& ogg_vorbis_file_holder, int frequency);

			virtual int ReadOneData(void);

			virtual bool DataRemains(void);

		private:
			void PreRead(void);

		private:
			OggVorbisFileHolder&	ogg_vorbis_file_holder_;
			DynamicBuffer<char>			buffer_;
			unsigned int			current_;
			unsigned int			last_read_size_;
		};

		// -- OggDataReader --------------------------------------------------
		template <class CHANNEL_READER, class BIT_READER>
		class OggDataReader : public CHANNEL_READER, public BIT_READER {
		public:
			explicit OggDataReader(OggVorbisFileHolder& ogg_vorbis_file_holder, int frequency) :
				DataReader::Base(frequency),
				CHANNEL_READER(frequency),
				BIT_READER(ogg_vorbis_file_holder, frequency) {}
		};
	}
}

// -- DataReader::SixteenBitOgg ------------------------------------------
DataReader::SixteenBitOgg::SixteenBitOgg(OggVorbisFileHolder& ogg_vorbis_file_holder, int frequency) :
DataReader::Base(frequency),
ogg_vorbis_file_holder_(ogg_vorbis_file_holder),
buffer_(1024 * 16),
current_(0),
last_read_size_(0)
{
	this->PreRead();
}

int
DataReader::SixteenBitOgg::ReadOneData(void)
{
	char tmp[2];
	if (current_ >= last_read_size_) {
		throw Bmx2WavInternalException(L"Internal Exception Occured");
	}
	if (current_ + 1 != last_read_size_) {
		tmp[0] = buffer_.GetPtr()[current_];
		tmp[1] = buffer_.GetPtr()[current_ + 1];
		current_ += 2;
	}
	else {
		tmp[0] = buffer_.GetPtr()[current_];
		this->PreRead();
		if (NOT(this->DataRemains())) {
			throw Bmx2WavInvalidWAVFile(ogg_vorbis_file_holder_.GetFileHolder().GetFileName(), L"File Read Error");
		}
		tmp[1] = buffer_.GetPtr()[current_];
		current_ += 1;
	}
	if (current_ >= last_read_size_) {
		this->PreRead();
	}
	return static_cast<int>(*reinterpret_cast<short*>(tmp));
}

bool
DataReader::SixteenBitOgg::DataRemains(void)
{
	return last_read_size_ > 0;
}

void
DataReader::SixteenBitOgg::PreRead(void)
{
	int bitstream;
	last_read_size_ = ov_read(ogg_vorbis_file_holder_.GetPtr(),
		buffer_.GetPtr(),
		buffer_.GetSize(),
		0, 2, 1, &bitstream);
	current_ = 0;
}


HQWav*
WavMaker::MakeNewWavFromOggFile(const std::wstring& filename)
{
	FileHolder file_holder(filename);

	OggVorbisFileHolder ogg_vorbis_file_holder(file_holder);

	vorbis_info *vorbis_info = ov_info(ogg_vorbis_file_holder.GetPtr(), -1);

	if (vorbis_info == NULL) {
		throw Bmx2WavCannotReadFile(filename);
	}

	if (vorbis_info->channels != 1 && vorbis_info->channels != 2) {
		throw Bmx2WavCannotReadFile(filename);
	}

	HQWav* wav = this->MakeNewWav();

	try {
		using namespace DataReader;
		if (vorbis_info->channels == 1) {
			this->ReadDataFromReader(wav, OggDataReader<OneChannel, SixteenBitOgg>(ogg_vorbis_file_holder, vorbis_info->rate));
		}
		else if (vorbis_info->channels == 2) {
			this->ReadDataFromReader(wav, OggDataReader<TwoChannel, SixteenBitOgg>(ogg_vorbis_file_holder, vorbis_info->rate));
		}
		else {
			throw Bmx2WavInternalException(L"Internal Error - Unknown Channel (ogg file)");
		}
		return wav;
	}
	catch (...) {
		delete wav;
		throw;
	}
}
