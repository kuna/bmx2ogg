#include "exception.h"
#include "bmx2wav_common.h"

#include <sstream>

Bmx2WavInvalidFile::Bmx2WavInvalidFile(const std::wstring& filename, int errorno)
	: message(filename + L": " + to_wstring(errorno)) {}
std::wstring Bmx2WavInvalidFile::Message() const { return message; }

Bmx2WavCannotReadFile::Bmx2WavCannotReadFile(const std::wstring& filename)
	: message(L"Cannot Read File : " + filename) {}
std::wstring Bmx2WavCannotReadFile::Message() const { return message; }

Bmx2WavCannotWriteFile::Bmx2WavCannotWriteFile(const std::wstring& filename)
	: message(L"Cannot Write File : " + filename) {}
std::wstring Bmx2WavCannotWriteFile::Message() const { return message; }

Bmx2WavInvalidWAVFile::Bmx2WavInvalidWAVFile(const std::wstring& filename, const std::wstring& message)
	: message(filename + L": " + message) {}
std::wstring Bmx2WavInvalidWAVFile::Message() const { return message; }

Bmx2WavInternalException::Bmx2WavInternalException(const std::wstring& message)
	: message(message) {}
std::wstring Bmx2WavInternalException::Message() const { return message; }