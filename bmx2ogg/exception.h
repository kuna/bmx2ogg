#pragma once
#include "bmx2wav_common.h"

class Bmx2WavException {
public:
	virtual std::string Message() const = 0;
};

class Bmx2WavInvalidFile : public Bmx2WavException {
private:
	std::string message;
public:
	Bmx2WavInvalidFile(const std::string& filename, int errorno);
	virtual std::string Message() const;
};

class Bmx2WavCannotReadFile : public Bmx2WavException {
private:
	std::string message;
public:
	Bmx2WavCannotReadFile(const std::string& filename);
	virtual std::string Message() const;
};

class Bmx2WavCannotWriteFile : public Bmx2WavException {
private:
	std::string message;
public:
	Bmx2WavCannotWriteFile(const std::string& filename);
	virtual std::string Message() const;
};

class Bmx2WavInvalidWAVFile : public Bmx2WavException {
private:
	std::string message;
public:
	Bmx2WavInvalidWAVFile(const std::string& filename, const std::string& message);
	virtual std::string Message() const;
};

class Bmx2WavInternalException : public Bmx2WavException {
private:
	std::string message;
public:
	Bmx2WavInternalException(const std::string& message);
	virtual std::string Message() const;
};
