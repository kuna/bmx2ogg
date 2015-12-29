#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include "bmx2wav_wav_maker.h"
#include "exception.h"
#include "bmx2wav_common.h"

using namespace Bmx2Wav;

#include <math.h>
#include <map>
#include <sstream>

#pragma warning(disable:4996)

// -- WavFileReader ------------------------------------------------------
WavFileReader::WavFileReader(const std::wstring& filename) :
filename_(filename),
file_(_wfopen(filename.c_str(), L"rb"))
{
	if (file_ == NULL) {
		throw Bmx2WavInvalidFile(filename, errno);
	}
}

WavFileReader::~WavFileReader()
{
	fclose(file_);
}

int
WavFileReader::ReadByte(void)
{
	unsigned char buf = 0x00;
	if (fread(reinterpret_cast<char*>(&buf), sizeof(char), 1, file_) != 1) {
		throw WavFileReader::ReadException();
	}
	return static_cast<int>(buf);
}

int
WavFileReader::Read2Byte(void)
{
	signed short int buf = 0x0000;
	if (fread(reinterpret_cast<char*>(&buf), sizeof(char), 2, file_) != 2) {
		throw WavFileReader::ReadException();
	}
	return static_cast<int>(buf);
}

int
WavFileReader::ReadInteger(void)
{
	signed int buf = 0x00000000;
	if (fread(reinterpret_cast<char*>(&buf), sizeof(char), 4, file_) != 4) {
		throw WavFileReader::ReadException();
	}
	return static_cast<int>(buf);
}

void
WavFileReader::Read(DynamicBuffer<unsigned char>& buffer)
{
	if (fread(buffer.GetPtr(), sizeof(char), buffer.GetSize(), file_) != buffer.GetSize()) {
		throw WavFileReader::ReadException();
	}
}


void
WavFileReader::Skip(unsigned int offset)
{
	if (fseek(file_, offset, SEEK_CUR) != 0) {
		throw WavFileReader::ReadException();
	}
}

// -- WavMaker -----------------------------------------------------------
WavMaker::WavMaker(bool not_use_filter) :
not_use_filter_(not_use_filter)
{
}

HQWav*
WavMaker::MakeNewWav(void)
{
	return new HQWav();
}

HQWav*
WavMaker::MakeNewWavFromWavFile(const std::wstring& filename, bool overlook_error)
{
	HQWav* wav = this->MakeNewWav();
	try {
		WavFileReader reader(filename);

		// RIFF header
		if (reader.ReadInteger() != HQWav::RIFF_HEADER && NOT(overlook_error)) {
			throw Bmx2WavInvalidWAVFile(filename, L"RIFF header");
		}

		// file size
		reader.ReadInteger();

		// WAVE header
		if (reader.ReadInteger() != HQWav::WAVE_HEADER && NOT(overlook_error)) {
			throw Bmx2WavInvalidWAVFile(filename, L"WAVE header");
		}

		// fmt チャンクになるまで読み飛ばす
		for (;;) {
			if (reader.ReadInteger() == HQWav::FMT_CHUNK) {
				break;
			}
			reader.Skip(reader.ReadInteger());
		}

		// fmt byte size
		int fmt_byte_size = reader.ReadInteger();

		// fmt id
		if (reader.Read2Byte() != HQWav::FMT_ID) {
			throw Bmx2WavInvalidWAVFile(filename, L"fmt id");
		}

		// channel count
		int channel_count = reader.Read2Byte();
		if (channel_count != 1 && channel_count != 2) {
			throw Bmx2WavInvalidWAVFile(filename, L"Unsupported WAV File. channel count is " + to_wstring(channel_count));
		}

		// frequency
		int frequency = reader.ReadInteger();

		// byte per second
		int byte_per_second = reader.ReadInteger();

		// block size
		int block_size = reader.Read2Byte();

		// bit rate
		int bit_rate = reader.Read2Byte();
		if (bit_rate != 8 && bit_rate != 16 && bit_rate != 24) {
			throw Bmx2WavInvalidWAVFile(filename, L"Unsupported WAV File. bit rate is " + to_wstring(bit_rate));
		}

		// 細かいチェック
		if (block_size != (bit_rate / 8) * channel_count && NOT(overlook_error)) {
			throw Bmx2WavInvalidWAVFile(filename, L"block size, bit rate, and channel count");
		}
		if (byte_per_second != frequency * block_size && NOT(overlook_error)) {
			throw Bmx2WavInvalidWAVFile(filename, L"byte per second, frequency, and block size");
		}

		// 拡張部分の対応
		if (fmt_byte_size - HQWav::FMT_BYTE_SIZE > 0) {
			reader.Skip(fmt_byte_size - HQWav::FMT_BYTE_SIZE);
		}

		// data チャンクになるまで読み飛ばす
		for (;;) {
			if (reader.ReadInteger() == HQWav::DATA_CHUNK) {
				break;
			}
			reader.Skip(reader.ReadInteger());
		}

		// data size
		int data_size = reader.ReadInteger();

		// data size のチェック
		if (data_size % ((bit_rate / 8) * channel_count) != 0) {
			if (NOT(overlook_error)) {
				throw Bmx2WavInvalidWAVFile(filename, L"data size can not be divided by block size");
			}
			data_size -= data_size % ((bit_rate / 8) * channel_count);
		}

		wav->data_.reserve(static_cast<unsigned int>(
			static_cast<double>(data_size / channel_count / (bit_rate / 8)) *
			(static_cast<double>(HQWav::FREQUENCY) / static_cast<double>(frequency))));

		DynamicBuffer<unsigned char> buffer(data_size);
		reader.Read(buffer);

		{
			using namespace DataReader;
			if (channel_count == 1 && bit_rate == 8) {
				this->ReadDataFromReader(wav, WavDataReader<OneChannel, EightBitWav>(buffer, frequency));
			}
			else if (channel_count == 1 && bit_rate == 16) {
				this->ReadDataFromReader(wav, WavDataReader<OneChannel, SixteenBitWav>(buffer, frequency));
			}
			else if (channel_count == 1 && bit_rate == 24) {
				this->ReadDataFromReader(wav, WavDataReader<OneChannel, TwentyFourBitWav>(buffer, frequency));
			}
			else if (channel_count == 2 && bit_rate == 8) {
				this->ReadDataFromReader(wav, WavDataReader<TwoChannel, EightBitWav>(buffer, frequency));
			}
			else if (channel_count == 2 && bit_rate == 16) {
				this->ReadDataFromReader(wav, WavDataReader<TwoChannel, SixteenBitWav>(buffer, frequency));
			}
			else if (channel_count == 2 && bit_rate == 24) {
				this->ReadDataFromReader(wav, WavDataReader<TwoChannel, TwentyFourBitWav>(buffer, frequency));
			}
			else {
				throw Bmx2WavInternalException(L"There\'s nothing matches channelcount & bitrate - THIS SHOULDnt BE OCCURED");
			}
		}
		return wav;
	}
	catch (WavFileReader::ReadException) {
		delete wav;
		throw Bmx2WavInvalidWAVFile(filename, L"Cannot Read WAV File.");
	}
	catch (...) {
		delete wav;
		throw;
	}
}

void
WavMaker::ReadDataFromReader(HQWav* wav, DataReader::Base& reader)
{
	int frequency = reader.GetFrequency();
	Filter::AllPass             all_pass_filter(wav->data_);
	Filter::ButterworthTwoOrder low_pass_filter(wav->data_, frequency / 2);
	Filter::Base& filter = *(not_use_filter_ ?
		static_cast<Filter::Base*>(&all_pass_filter) :
		static_cast<Filter::Base*>(&low_pass_filter));

	switch (frequency) {
		// 44kHz 読み込み
	case HQWav::FREQUENCY: {
		while (reader.DataRemains()) {
			wav->data_.push_back(reader.ReadTick());
		}
		break;
	}

						   // 22kHz 読み込み
	case 22050: {
		HQWav::Tick prev1 = reader.ReadTick();
		HQWav::Tick prev2;
		filter.Input(prev1);
		while (reader.DataRemains()) {
			HQWav::Tick current(reader.ReadTick());

			filter.Input(HQWav::Tick((prev1.left_ + current.left_) / 2,
				(prev1.right_ + current.right_) / 2));
			filter.Input(current);

			prev2 = prev1;
			prev1 = current;
		}
		filter.Input(HQWav::Tick((prev1.left_ * 3 - prev2.left_) / 2,
			(prev1.right_ * 3 - prev2.right_) / 2));
		break;
	}

				// 11kHz 読み込み
	case 11025: {
		HQWav::Tick prev1 = reader.ReadTick();
		HQWav::Tick prev2;
		filter.Input(prev1);
		while (reader.DataRemains()) {
			HQWav::Tick current(reader.ReadTick());
			HQWav::Tick tmp((prev1.left_ + current.left_) / 2,
				(prev1.right_ + current.right_) / 2);

			filter.Input(HQWav::Tick((prev1.left_ + tmp.left_) / 2,
				(prev1.right_ + tmp.right_) / 2));
			filter.Input(tmp);
			filter.Input(HQWav::Tick((tmp.left_ + current.left_) / 2,
				(tmp.right_ + current.right_) / 2));
			filter.Input(current);

			prev2 = prev1;
			prev1 = current;
		}
		filter.Input(HQWav::Tick((prev1.left_ * 5 - prev2.left_) / 2,
			(prev1.right_ * 5 - prev2.right_) / 2));
		filter.Input(HQWav::Tick((prev1.left_ * 3 - prev2.left_) / 2,
			(prev1.right_ * 3 - prev2.right_) / 2));
		filter.Input(HQWav::Tick((prev1.left_ * 7 - prev2.left_ * 3) / 2,
			(prev1.right_ * 7 - prev2.right_ * 3) / 2));
		break;
	}

				// 他周波数 読み込み
	default: {
		std::map<double, HQWav::Tick> origin;
		double origin_tick_step = 1.0 / static_cast<double>(frequency);

		for (int i = 0; reader.DataRemains(); ++i) {
			origin[origin_tick_step * static_cast<double>(i)] = reader.ReadTick();
		}

		std::map<double, HQWav::Tick>::iterator it = origin.begin();
		filter.Input(origin[0.0]);
		std::pair<double, HQWav::Tick> prev = *it;
		++it;
		std::pair<double, HQWav::Tick> next = *it;
		for (unsigned int i = 1;; ++i) {
			double current = (1.0 / static_cast<double>(HQWav::FREQUENCY)) * static_cast<double>(i);
			if (prev.first <= current && current <= next.first) {
				filter.Input(HQWav::Tick(
					(
					(static_cast<double>(prev.second.left_) * (next.first - current)) +
					(static_cast<double>(next.second.left_) * (current - prev.first))
					) / origin_tick_step,
					(
					(static_cast<double>(prev.second.right_) * (next.first - current)) +
					(static_cast<double>(next.second.right_) * (current - prev.first))
					) / origin_tick_step));
			}
			else {
				prev = next;
				++it;
				if (it == origin.end()) {
					break;
				}
				next = *it;
				// ループをやり直し
				--i;
				continue;
			}
		}
		break;
	}

	} // end switch
}

// -- DataReader::OneChannel ---------------------------------------------
DataReader::OneChannel::OneChannel(int frequency) :
DataReader::Base(frequency)
{
}

HQWav::Tick
DataReader::OneChannel::ReadTick(void)
{
	return HQWav::Tick(this->ReadOneData());
}

// -- DataReader::TwoChannel ---------------------------------------------
DataReader::TwoChannel::TwoChannel(int frequency) :
DataReader::Base(frequency)
{
}

HQWav::Tick
DataReader::TwoChannel::ReadTick(void)
{
	int l = this->ReadOneData();
	int r = this->ReadOneData();
	return HQWav::Tick(l, r);
}


// -- DataReader::EightBitWav --------------------------------------------
DataReader::EightBitWav::EightBitWav(DynamicBuffer<unsigned char>& buffer, int frequency) :
DataReader::Base(frequency),
buffer_(buffer),
current_(0)
{
}

int
DataReader::EightBitWav::ReadOneData(void)
{
	if (current_ + 1 > buffer_.GetSize()) {
		throw Bmx2WavInternalException(L"Internal Error - Buffer Overflow.");
	}
	int tmp = static_cast<int>(buffer_.GetPtr()[current_]);
	current_ += 1;
	return (tmp - 128) * 256;
}

bool
DataReader::EightBitWav::DataRemains(void)
{
	return current_ < buffer_.GetSize();
}

// -- DataReader::SixteenBitWav ------------------------------------------
DataReader::SixteenBitWav::SixteenBitWav(DynamicBuffer<unsigned char>& buffer, int frequency) :
DataReader::Base(frequency),
buffer_(buffer),
current_(0)
{
}

int
DataReader::SixteenBitWav::ReadOneData(void)
{
	if (current_ + 2 > buffer_.GetSize()) {
		throw Bmx2WavInternalException(L"Internal Error - Buffer Overflow.");
	}
	char tmp[2];
	tmp[0] = buffer_.GetPtr()[current_];
	tmp[1] = buffer_.GetPtr()[current_ + 1];
	current_ += 2;
	return static_cast<int>(*reinterpret_cast<short*>(tmp));
}

bool
DataReader::SixteenBitWav::DataRemains(void)
{
	return current_ < buffer_.GetSize();
}

// -- DataReader::TwentyFourBitWav ---------------------------------------
DataReader::TwentyFourBitWav::TwentyFourBitWav(DynamicBuffer<unsigned char>& buffer, int frequency) :
DataReader::Base(frequency),
buffer_(buffer),
current_(0)
{
}

int
DataReader::TwentyFourBitWav::ReadOneData(void)
{
	if (current_ + 3 > buffer_.GetSize()) {
		throw Bmx2WavInternalException(L"Internal Error - Buffer Overflow.");
	}
	char tmp[4];
	tmp[0] = buffer_.GetPtr()[current_];
	tmp[1] = buffer_.GetPtr()[current_ + 1];
	tmp[2] = buffer_.GetPtr()[current_ + 2];
	tmp[3] = static_cast<char>(tmp[2] & 0x80 ? 0xFF : 0x00);
	current_ += 3;
	return *reinterpret_cast<int*>(tmp) / 256;
}

bool
DataReader::TwentyFourBitWav::DataRemains(void)
{
	return current_ < buffer_.GetSize();
}


// -- Filter::Base -------------------------------------------------------
Filter::Base::Base(std::vector<HQWav::Tick>& data) :
data_(data)
{
}

void
Filter::Base::OutputToData(const HQWav::Tick& tick)
{
	data_.push_back(tick);
}


// -- Filter::AllPass ----------------------------------------------------
Filter::AllPass::AllPass(std::vector<HQWav::Tick>& data) :
Filter::Base(data)
{
}

void
Filter::AllPass::Input(const HQWav::Tick& tick)
{
	this->OutputToData(tick);
}

// -- Filter::ButterworthTwoOrder ----------------------------------------
Filter::ButterworthTwoOrder::ButterworthTwoOrder(std::vector<HQWav::Tick>& data, unsigned int cutoff_frequency) :
Filter::Base(data)
{
	const double Q = M_SQRT2;
	const double T = 1.0 / static_cast<double>(HQWav::FREQUENCY);
	const double omega = 2.0 * M_PI * cutoff_frequency;

	left_.z0_ = 0.0;
	left_.z1_ = 0.0;
	left_.z2_ = 0.0;
	right_.z0_ = 0.0;
	right_.z1_ = 0.0;
	right_.z2_ = 0.0;

	const double denominator = 4 * Q + 2 * T * omega + Q * T * T * omega * omega;
	a1_ = (8 * Q - 2 * Q * T * T * omega * omega) / denominator;
	a2_ = (-(4 * Q - 2 * T * omega + Q * T * T * omega * omega)) / denominator;
	b0_ = (Q * T * T * omega * omega) / denominator;
}

void
Filter::ButterworthTwoOrder::Input(const HQWav::Tick& tick)
{
	left_.z0_ = (tick.left_) + (a1_ * left_.z1_) + (a2_ * left_.z2_);
	right_.z0_ = (tick.right_) + (a1_ * right_.z1_) + (a2_ * right_.z2_);

	this->OutputToData(HQWav::Tick(
		(b0_ * left_.z0_) + (b0_ * 2.0 * left_.z1_) + (b0_ * left_.z2_),
		(b0_ * right_.z0_) + (b0_ * 2.0 * right_.z1_) + (b0_ * right_.z2_)));

	left_.z2_ = left_.z1_;
	right_.z2_ = right_.z1_;
	left_.z1_ = left_.z0_;
	right_.z1_ = right_.z0_;
}
