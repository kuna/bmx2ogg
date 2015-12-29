#ifndef BMX2WAV_WAV_MAKER_H_
#define BMX2WAV_WAV_MAKER_H_

#include <stdio.h>

#include "bmx2wav_wav.h"
#include "bmx2wav_buffer.h"

namespace Bmx2Wav {
	// -- WavFileReader ----------------------------------------------------
	class WavFileReader {
	public:
		explicit WavFileReader(const std::wstring& filename);
		virtual ~WavFileReader();

		class ReadException {};
		int ReadByte(void);
		int Read2Byte(void);
		int ReadInteger(void);
		void Read(DynamicBuffer<unsigned char>& buffer);

		void Skip(unsigned int offset);

	private:
		const std::wstring filename_;
		FILE* file_;
	};

	namespace DataReader{
		class Base;
	}
	namespace Filter{
		class Base;
	}


	// -- WavMaker ---------------------------------------------------------
	class WavMaker {
	public:
		WavMaker(bool not_use_filter = false);

		HQWav* MakeNewWav(void);
		HQWav* MakeNewWavFromWavFile(const std::wstring& filename, bool overlook_error);
		HQWav* MakeNewWavFromOggFile(const std::wstring& filename);

	private:
		void ReadDataFromReader(HQWav* wav, DataReader::Base& reader);

	private:
		bool     not_use_filter_;
	};

	namespace DataReader {
		// -- DataReader::Base -----------------------------------------------
		class Base {
		protected:
			explicit Base(int frequency) : frequency_(frequency) {}

		public:
			virtual ~Base() {}

			int GetFrequency(void) const { return frequency_; }

			virtual HQWav::Tick ReadTick(void) = 0;

			virtual int ReadOneData(void) = 0;

			virtual bool DataRemains(void) = 0;

		private:
			int frequency_;
		};

		// -- DataReader::OneChannel -----------------------------------------
		class OneChannel : virtual public Base {
		public:
			explicit OneChannel(int frequency);

			virtual HQWav::Tick ReadTick(void);
		};

		// -- DataReader::TwoChannel -----------------------------------------
		class TwoChannel : virtual public Base {
		public:
			explicit TwoChannel(int frequency);

			virtual HQWav::Tick ReadTick(void);
		};

		// -- DataReader::EightBitWav ----------------------------------------
		class EightBitWav : virtual public Base {
		public:
			explicit EightBitWav(DynamicBuffer<unsigned char>& buffer, int frequency);

			virtual int ReadOneData(void);

			virtual bool DataRemains(void);

		private:
			DynamicBuffer<unsigned char>& buffer_;
			unsigned int current_;
		};

		// -- DataReader::SixteenBitWav --------------------------------------
		class SixteenBitWav : virtual public Base {
		public:
			explicit SixteenBitWav(DynamicBuffer<unsigned char>& buffer, int frequency);

			virtual int ReadOneData(void);

			virtual bool DataRemains(void);

		private:
			DynamicBuffer<unsigned char>& buffer_;
			unsigned int current_;
		};

		// -- DataReader::TwentyFourBitWav -----------------------------------
		class TwentyFourBitWav : virtual public Base {
		public:
			explicit TwentyFourBitWav(DynamicBuffer<unsigned char>& buffer, int frequency);

			virtual int ReadOneData(void);

			virtual bool DataRemains(void);

		private:
			DynamicBuffer<unsigned char>& buffer_;
			unsigned int current_;
		};

		// -- WavDataReader --------------------------------------------------
		template <class CHANNEL_READER, class BIT_READER>
		class WavDataReader : public CHANNEL_READER, public BIT_READER {
		public:
			explicit WavDataReader(DynamicBuffer<unsigned char>& buffer, int frequency) :
				DataReader::Base(frequency),
				CHANNEL_READER(frequency),
				BIT_READER(buffer, frequency) {}
		};
	}

	namespace Filter {
		// -- Filter::Base ---------------------------------------------------
		class Base {
		protected:
			explicit Base(std::vector<HQWav::Tick>& data);

			void OutputToData(const HQWav::Tick& tick);

		public:
			virtual void Input(const HQWav::Tick& tick) = 0;

		private:
			std::vector<HQWav::Tick>& data_;
		};

		// -- Filter::AllPass ------------------------------------------------
		class AllPass : public Base {
		public:
			explicit AllPass(std::vector<HQWav::Tick>& data);

			virtual void Input(const HQWav::Tick& tick);
		};

		// -- Filter::ButterworthTwoOrder ------------------------------------
		class ButterworthTwoOrder : public Base {
		public:
			explicit ButterworthTwoOrder(std::vector<HQWav::Tick>& data, unsigned int cutoff_frequency);

			virtual void Input(const HQWav::Tick& tick);

		private:
			double       a1_;
			double       a2_;
			double       b0_;
			struct {
				double       z0_;
				double       z1_;
				double       z2_;
			} left_, right_;
		};
	}
}

#endif // BMX2WAV_WAV_MAKER_H_
