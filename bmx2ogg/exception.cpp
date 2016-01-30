#include "exception.h"
#include "bmx2wav_common.h"

#include <sstream>

Bmx2WavInvalidFile::Bmx2WavInvalidFile(const std::string& filename, int errorno)
	: message(filename + ": " + to_string(errorno)) {}
std::string Bmx2WavInvalidFile::Message() const { return message; }

Bmx2WavCannotReadFile::Bmx2WavCannotReadFile(const std::string& filename)
	: message("Cannot Read File : " + filename) {}
std::string Bmx2WavCannotReadFile::Message() const { return message; }

Bmx2WavCannotWriteFile::Bmx2WavCannotWriteFile(const std::string& filename)
	: message("Cannot Write File : " + filename) {}
std::string Bmx2WavCannotWriteFile::Message() const { return message; }

Bmx2WavInvalidWAVFile::Bmx2WavInvalidWAVFile(const std::string& filename, const std::string& message)
	: message(filename + ": " + message) {}
std::string Bmx2WavInvalidWAVFile::Message() const { return message; }

Bmx2WavInternalException::Bmx2WavInternalException(const std::string& message)
	: message(message) {}
std::string Bmx2WavInternalException::Message() const { return message; }
