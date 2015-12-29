#pragma once

#include <stdexcept>

class Bmx2WavException : public std::exception {
public:
	virtual std::wstring Message() const = 0;
};

class Bmx2WavInvalidFile : public Bmx2WavException {
private:
	std::wstring message;
public:
	Bmx2WavInvalidFile(const std::wstring& filename, int errorno);
	virtual std::wstring Message() const;
};

class Bmx2WavCannotReadFile : public Bmx2WavException {
private:
	std::wstring message;
public:
	Bmx2WavCannotReadFile(const std::wstring& filename);
	virtual std::wstring Message() const;
};

class Bmx2WavCannotWriteFile : public Bmx2WavException {
private:
	std::wstring message;
public:
	Bmx2WavCannotWriteFile(const std::wstring& filename);
	virtual std::wstring Message() const;
};

class Bmx2WavInvalidWAVFile : public Bmx2WavException {
private:
	std::wstring message;
public:
	Bmx2WavInvalidWAVFile(const std::wstring& filename, const std::wstring& message);
	virtual std::wstring Message() const;
};

class Bmx2WavInternalException : public Bmx2WavException {
private:
	std::wstring message;
public:
	Bmx2WavInternalException(const std::wstring& message);
	virtual std::wstring Message() const;
};