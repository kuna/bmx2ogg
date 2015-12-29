#ifndef BMX2WAV_WAV_H_
#define BMX2WAV_WAV_H_

#include <string>
#include <vector>

namespace Bmx2Wav {
	// 44kHz 16bit 2channel
	// -- HQWav ------------------------------------------------------------
	class HQWav {
		friend class WavMaker;
	public:
		struct Tick {
		public:
			static const int MaxValue = 32767;
			static const int MinValue = -32768;

		public:
			explicit Tick(void);
			explicit Tick(int center);
			explicit Tick(int left, int right);
			~Tick();

			void Change(double ratio);
			void Mixin(const Tick& other);
			void Deduct(const Tick& other);

		public:
			int left_;
			int right_;
		};

	public:
		static const int RIFF_HEADER = 0x46464952; // *((int *)"RIFF");
		static const int WAVE_HEADER = 0x45564157; // *((int *)"WAVE");
		static const int FMT_CHUNK = 0x20746D66; // *((int *)"fmt ");
		static const int DATA_CHUNK = 0x61746164; // *((int *)"data");
		static const int FMT_BYTE_SIZE = 16;
		static const int FMT_ID = 1;
		static const int FREQUENCY = 44100;

		static const HQWav Empty;

	public:
		explicit HQWav(void);

	public:
		unsigned int GetLength(void) const;
		Tick& At(int index);
		const Tick& At(int index) const;
		bool IsInclude(int index) const;

		void ChangeVolume(double ratio);

		void MixinAt(int pos, const HQWav& other);
		void MixinAt(int pos, const HQWav& other, int other_pos);
		void DeductAt(int pos, const HQWav& other);
		void DeductAt(int pos, const HQWav& other, int other_pos);
	private:
		void ConvoluteAt(int pos, const HQWav& other, int other_pos, void (Tick::*function)(const Tick&));

	public:
		void PeakNormalize(void);
		void PeakNormalize(double* change_ratio);
		void AverageNormalize(void);
		void AverageNormalize(double* change_ratio);
		void OverNormalize(double over_ratio);
		void OverNormalize(double over_ratio, double* change_ratio);

		void Compress(double       threshold,
			double       ratio,
			unsigned int attack_time,
			unsigned int release_time,
			int          look_ahead);

		void SimpleCompress(double shave_ratio);

		void Trim(int start, int end);

		void FadeIn(int start, int end, double ratio = 0.0);
		void FadeOut(int start, int end, double ratio = 0.0);

		void ExtendTo(unsigned int size);

		void Clear(void);

		void WriteToFile(const std::wstring& filename);
		void WriteToBuffer(std::vector<char>& buffer);
	private:
		// -- SimpleIterator -------------------------------------------------
		class SimpleIterator {
		public:
			explicit SimpleIterator(std::vector<Tick>& data, std::vector<Tick>& contrary_data);

			Tick& GetTick(void);
			void MoveNext(void);
			bool DataRemain(void);

		private:
			int                position_;
			std::vector<Tick>& data_;
			std::vector<Tick>& contrary_data_;
		};

		SimpleIterator MakeSimpleIterator(void);

	private:
		std::vector<Tick> data_;
		std::vector<Tick> contrary_data_;
	};
}

#endif // BMX2WAV_WAV_H_
