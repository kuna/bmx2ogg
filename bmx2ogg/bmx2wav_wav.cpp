#include <stdio.h>
#include <math.h>
#include <errno.h>

#include <limits>
#include <map>
#include <algorithm>

#include "exception.h"
#include "bmx2wav_wav.h"
#include "bmx2wav_common.h"

using namespace Bmx2Wav;

#define CONTRARY( i ) ((-1 * (i)) - 1)
#pragma warning(disable:4996)

/*
 * some utility functions
 */

// -- HQWav::Tick --------------------------------------------------------
HQWav::Tick::Tick(void) :
left_(0),
right_(0)
{
}

HQWav::Tick::Tick(int center) :
left_(center),
right_(center)
{
}

HQWav::Tick::Tick(int left, int right) :
left_(left),
right_(right)
{
}

HQWav::Tick::~Tick()
{
}

void
HQWav::Tick::Change(double ratio)
{
	{
		double l = static_cast<double>(left_);
		l *= ratio;
		if (l < static_cast<double>(std::numeric_limits<int>::min())) {
			left_ = std::numeric_limits<int>::min();
		}
		else if (l > static_cast<double>(std::numeric_limits<int>::max())) {
			left_ = std::numeric_limits<int>::max();
		}
		else {
			left_ = static_cast<int>(l);
		}
	}

  {
	  double r = static_cast<double>(right_);
	  r *= ratio;
	  if (r < static_cast<double>(std::numeric_limits<int>::min())) {
		  right_ = std::numeric_limits<int>::min();
	  }
	  else if (r > static_cast<double>(std::numeric_limits<int>::max())) {
		  right_ = std::numeric_limits<int>::max();
	  }
	  else {
		  right_ = static_cast<int>(r);
	  }
  }
}

void
HQWav::Tick::Mixin(const Tick& other)
{
	left_ += other.left_;
	right_ += other.right_;
}

void
HQWav::Tick::Deduct(const Tick& other)
{
	left_ -= other.left_;
	right_ -= other.right_;
}


// -- HQWav --------------------------------------------------------------
const HQWav
HQWav::Empty;

HQWav::HQWav(void) :
data_(),
contrary_data_()
{
}


unsigned int
HQWav::GetLength(void) const
{
	return data_.size();
}

HQWav::Tick&
HQWav::At(int index)
{
	return index < 0 ? contrary_data_[CONTRARY(index)] : data_[index];
}

const HQWav::Tick&
HQWav::At(int index) const
{
	return index < 0 ? contrary_data_[CONTRARY(index)] : data_[index];
}

bool
HQWav::IsInclude(int index) const
{
	return index < 0 ? (CONTRARY(index) < static_cast<int>(contrary_data_.size()))
		: (index < static_cast<int>(data_.size()));
}


void
HQWav::ChangeVolume(double ratio)
{
	for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
		it.GetTick().Change(ratio);
	}
}

void
HQWav::MixinAt(int pos, const HQWav& other)
{
	this->MixinAt(pos, other, 0);
}

void
HQWav::MixinAt(int pos, const HQWav& other, int other_pos)
{
	this->ConvoluteAt(pos, other, other_pos, &Tick::Mixin);
}


void
HQWav::DeductAt(int pos, const HQWav& other)
{
	this->DeductAt(pos, other, 0);
}

void
HQWav::DeductAt(int pos, const HQWav& other, int other_pos)
{
	this->ConvoluteAt(pos, other, other_pos, &Tick::Deduct);
}

void
HQWav::ConvoluteAt(int pos, const HQWav& other, int other_pos, void (Tick::*function)(const Tick&))
{
	if (pos + other.data_.size() > data_.size()) {
		data_.resize(pos + other.data_.size());
	}
	if ((-1 * pos) >= static_cast<int>(contrary_data_.size())) {
		contrary_data_.resize(-1 * pos);
	}

	for (int i = 0;; ++i) {
		if (NOT(other.IsInclude(other_pos + i))) {
			if (other_pos + i < 0) {
				continue;
			}
			break;
		}
		(this->At(pos + i).*function)(other.At(i + other_pos));
	}
}



void
HQWav::PeakNormalize(void)
{
	double change_ratio;
	this->PeakNormalize(&change_ratio);
}

void
HQWav::PeakNormalize(double* change_ratio)
{
	int max = 0;
	for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
		int tmp;
		tmp = abs(it.GetTick().left_);
		if (tmp > max) {
			max = tmp;
		}
		tmp = abs(it.GetTick().right_);
		if (tmp > max) {
			max = tmp;
		}
	}

	if (max == 0) {
		*change_ratio = 1.0;
		return;
	}

	double ratio = static_cast<double>(Tick::MaxValue) / static_cast<double>(max);
	this->ChangeVolume(ratio);
	*change_ratio = ratio;
}

void
HQWav::AverageNormalize(void)
{
	double change_ratio;
	this->AverageNormalize(&change_ratio);
}

void
HQWav::AverageNormalize(double* change_ratio)
{
	double sum = 0.0;
	int count = 0;
	for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
		sum += static_cast<double>(abs(it.GetTick().left_));
		sum += static_cast<double>(abs(it.GetTick().right_));
		++count;
	}

	if (sum == 0 || count == 0) {
		*change_ratio = 1.0;
		return;
	}

	double ratio = (
		(static_cast<double>(Tick::MaxValue) / 10.0) /
		(sum / static_cast<double>(count * 2)));
	this->ChangeVolume(ratio);
	*change_ratio = ratio;
}

void
HQWav::OverNormalize(double over_ratio)
{
	double change_ratio;
	this->OverNormalize(over_ratio, &change_ratio);
}

void
HQWav::OverNormalize(double over_ratio, double* change_ratio)
{
	std::map<unsigned int, unsigned int> table;
	unsigned int all_count = 0;
	for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
		table[abs(it.GetTick().left_)] += 1;
		table[abs(it.GetTick().right_)] += 1;
		all_count += 2;
	}

	if (all_count == 0) {
		*change_ratio = 1.0;
		return;
	}

	unsigned int new_max = 0;
	unsigned int over_count = 0;
	for (std::map<unsigned int, unsigned int>::reverse_iterator it = table.rbegin(); it != table.rend(); ++it) {
		new_max = it->first;
		over_count += it->second;
		if (over_count > all_count * over_ratio) {
			break;
		}
	}
	if (new_max == 0) {
		*change_ratio = 1.0;
		return;
	}

	double ratio = static_cast<double>(Tick::MaxValue) / static_cast<double>(new_max);
	this->ChangeVolume(ratio);
	*change_ratio = ratio;
}


void
HQWav::Compress(double       threshold,
double       ratio,
unsigned int attack_time,
unsigned int release_time,
int          look_ahead)
{
	int threshold_level = static_cast<int>(floor(fabs(threshold) * Tick::MaxValue + 0.5));
	double compress_ratio = threshold + (1.0 - threshold) * ratio;
	const unsigned int not_over_time_limit = HQWav::FREQUENCY / 10;
	unsigned int rest_time = 0;
	double change_ratio = 1.0;

	enum { DEFAULT, ON_ATTACK, ON_COMPRESS } status = DEFAULT;

	for (int i = (-1 * contrary_data_.size()); i < static_cast<int>(data_.size()); ++i) {
		int look_position = (i + look_ahead < (-1 * static_cast<int>(contrary_data_.size())) ? (-1 * contrary_data_.size()) :
			i + look_ahead >= static_cast<int>(data_.size()) ? data_.size() - 1 :
			i + look_ahead);
		switch (status) {
		case ON_ATTACK:
			if (rest_time > 0) {
				change_ratio -= (1.0 - compress_ratio) / static_cast<double>(attack_time);
				this->At(i).Change(change_ratio);
				--rest_time;
			}
			else {
				status = ON_COMPRESS;
				change_ratio = compress_ratio;
				this->At(i).Change(compress_ratio);
			}
			break;

		case ON_COMPRESS:
			++rest_time;
			if (abs(this->At(look_position).left_) > threshold_level || abs(this->At(look_position).right_) > threshold_level) {
				rest_time = 0;
			}
			this->At(i).Change(compress_ratio);
			if (rest_time > not_over_time_limit) {
				status = DEFAULT;
				rest_time = release_time;
				change_ratio = compress_ratio;
			}
			break;

		case DEFAULT:
		default:
			if (rest_time > 0) {
				change_ratio += (1.0 - compress_ratio) / static_cast<double>(release_time);
				this->At(i).Change(change_ratio);
				--rest_time;
			}
			if (abs(this->At(look_position).left_) > threshold_level || abs(this->At(look_position).right_) > threshold_level) {
				status = ON_ATTACK;
				rest_time = attack_time;
				change_ratio = 1.0;
			}
			break;

		}
	}
	this->PeakNormalize();
}


void
HQWav::SimpleCompress(double shave_ratio)
{
	std::vector<unsigned int> distribution_map(32768 + 1);

	for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
		distribution_map[abs(it.GetTick().left_)] += 1;
		distribution_map[abs(it.GetTick().right_)] += 1;
	}

	unsigned int cumulation = 0;
	double threshold = 0.0;
	for (int i = distribution_map.size() - 1; i > 0; --i) {
		cumulation += distribution_map[i];
		if (cumulation > data_.size() * shave_ratio) {
			threshold = static_cast<double>(i) / 32768.0;
			break;
		}
	}
	double ratio = 0.5;
	this->Compress(threshold, ratio, 1764, 4410, 2000);
}


void
HQWav::Trim(int start, int end)
{
	if (start > end) {
		throw Bmx2WavInternalException(L"Internal Exception - Wrong HQWav::Trim() Argument");
	}

	std::vector<Tick> new_data;
	std::vector<Tick> new_contrary_reverse_data;
	for (int i = start; i <= end; ++i) {
		if (i < 0) {
			if (CONTRARY(i) < static_cast<int>(contrary_data_.size())) {
				new_contrary_reverse_data.push_back(contrary_data_[CONTRARY(i)]);
			}
			else {
				new_contrary_reverse_data.push_back(Tick(0, 0));
			}
		}
		else {
			if (i < static_cast<int>(data_.size())) {
				new_data.push_back(data_[i]);
			}
			else {
				new_data.push_back(Tick(0, 0));
			}
		}
	}
	data_ = new_data;
	contrary_data_.clear();
	for (std::vector<Tick>::reverse_iterator it = new_contrary_reverse_data.rbegin(); it != new_contrary_reverse_data.rend(); ++it) {
		contrary_data_.push_back(*it);
	}
}


void
HQWav::FadeIn(int start, int end, double ratio)
{
	if (start > end) {
		throw Bmx2WavInternalException(L"Internal Exception - Wrong HQWav::FadeIn() Argument");
	}

	double tmp = ratio;
	for (int i = start; i <= end; ++i) {
		if (i < 0 && CONTRARY(i) < static_cast<int>(contrary_data_.size())) {
			contrary_data_[CONTRARY(i)].Change(tmp);
		}
		if (i >= 0 && i < static_cast<int>(data_.size())) {
			data_[i].Change(tmp);
		}
		tmp += (1.0 - ratio) / static_cast<double>(end - start);
	}
}

void
HQWav::FadeOut(int start, int end, double ratio)
{
	if (start > end) {
		throw Bmx2WavInternalException(L"Internal Exception - Wrong HQWav::FadeOut() Argument");
	}

	double tmp = 1.0;
	for (int i = start; i <= end; ++i) {
		if (i < 0 && CONTRARY(i) < static_cast<int>(contrary_data_.size())) {
			contrary_data_[CONTRARY(i)].Change(tmp);
		}
		if (i >= 0 && i < static_cast<int>(data_.size())) {
			data_[i].Change(tmp);
		}
		tmp -= (1.0 - ratio) / static_cast<double>(end - start);
	}
}


void
HQWav::ExtendTo(unsigned int size)
{
	if (size > data_.size()) {
		data_.resize(size);
	}
}


void
HQWav::Clear(void)
{
	data_.clear();
	contrary_data_.clear();
}



namespace Bmx2Wav {
	// -- WavFileWriter ----------------------------------------------------
	class WavFileWriter {
	public:
		explicit WavFileWriter(const std::wstring& filename);
		~WavFileWriter();

		class WriteException {};
		void Write2Byte(short data);
		void WriteInteger(int data);

	private:
		const std::wstring filename_;
		FILE* file_;
	};
}

// -- WavFileWriter ------------------------------------------------------
WavFileWriter::WavFileWriter(const std::wstring& filename) :
filename_(filename),
file_(_wfopen(filename.c_str(), L"wb+"))	// overwrites file
{
	if (file_ == NULL) {
		throw Bmx2WavInvalidFile(filename, errno);
	}
}

WavFileWriter::~WavFileWriter()
{
	fclose(file_);
}

void
WavFileWriter::Write2Byte(short data)
{
	if (fwrite(reinterpret_cast<char*>(&data), 1, sizeof(short), file_) != sizeof(short)) {
		throw WavFileWriter::WriteException();
	}
}

void
WavFileWriter::WriteInteger(int data)
{
	if (fwrite(reinterpret_cast<char*>(&data), 1, sizeof(int), file_) != sizeof(int)) {
		throw WavFileWriter::WriteException();
	}
}

void HQWav::WriteToBuffer(std::vector<char>& buffer)
{
	for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
		short buf_left = (std::max(std::min(it.GetTick().left_, Tick::MaxValue), Tick::MinValue));
		short buf_right = (std::max(std::min(it.GetTick().right_, Tick::MaxValue), Tick::MinValue));
		char *p;
		p = (char*)&buf_left;	buffer.push_back(p[0]);	buffer.push_back(p[1]);
		p = (char*)&buf_right;	buffer.push_back(p[0]);	buffer.push_back(p[1]);
	}
}

void
HQWav::WriteToFile(const std::wstring& filename)
{
	try {
		if (NOT(IO::make_parent_directory_recursive(IO::get_filedir(filename)))) {
			throw Bmx2WavInvalidFile(filename, ENOENT);
		}
		WavFileWriter writer(filename);

		writer.WriteInteger(HQWav::RIFF_HEADER);
		// file size
		// 36byte はヘッダ情報など
		writer.WriteInteger((contrary_data_.size() + data_.size()) * 2 * 2 + 36);
		writer.WriteInteger(HQWav::WAVE_HEADER);
		writer.WriteInteger(HQWav::FMT_CHUNK);
		writer.WriteInteger(HQWav::FMT_BYTE_SIZE);
		writer.Write2Byte(HQWav::FMT_ID);
		// channel count
		writer.Write2Byte(2);
		// frequency
		writer.WriteInteger(HQWav::FREQUENCY);
		// byte per second
		writer.WriteInteger(HQWav::FREQUENCY * 2 * 2);
		// block size
		writer.Write2Byte(4);
		// bit rate
		writer.Write2Byte(16);
		writer.WriteInteger(HQWav::DATA_CHUNK);
		writer.WriteInteger((contrary_data_.size() + data_.size()) * 2 * 2);
		for (SimpleIterator it = this->MakeSimpleIterator(); it.DataRemain(); it.MoveNext()) {
			writer.Write2Byte(static_cast<short>(std::max(std::min(it.GetTick().left_, Tick::MaxValue), Tick::MinValue)));
			writer.Write2Byte(static_cast<short>(std::max(std::min(it.GetTick().right_, Tick::MaxValue), Tick::MinValue)));
		}
	}
	catch (WavFileWriter::WriteException) {
		throw Bmx2WavCannotWriteFile(filename);
	}
}

// -- HQWav::SimpleIterator ----------------------------------------------
HQWav::SimpleIterator::SimpleIterator(std::vector<Tick>& data, std::vector<Tick>& contrary_data) :
position_(-1 * contrary_data.size()),
data_(data),
contrary_data_(contrary_data)
{
}

HQWav::Tick&
HQWav::SimpleIterator::GetTick(void)
{
	if (position_ < 0) {
		return contrary_data_[CONTRARY(position_)];
	}
	else {
		return data_[position_];
	}
}

void
HQWav::SimpleIterator::MoveNext(void)
{
	position_ += 1;
}

bool
HQWav::SimpleIterator::DataRemain(void)
{
	return position_ < 0 || position_ < static_cast<int>(data_.size());
}

HQWav::SimpleIterator
HQWav::MakeSimpleIterator(void)
{
	return SimpleIterator(data_, contrary_data_);
}
