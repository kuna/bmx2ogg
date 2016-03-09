#include "bmx2wav_common.h"

#include "iconv/iconv.h"
#include <sstream>
#include <sys/stat.h>
#include <cstdlib>
#include <wchar.h>
#include <array>
#include <algorithm>
#include <string.h>

std::wstring to_wstring (int i) { std::wostringstream wss; wss << i; return wss.str(); }
std::string to_string (int i) { std::ostringstream ss; ss << i; return ss.str(); }

namespace IO {
#ifdef _WIN32
	std::wstring substitute_extension(const std::wstring& filepath, const std::wstring& newext) {
		auto i = filepath.find_last_of(L'.');
		if (i == std::wstring::npos)
			return filepath + newext;
		return filepath.substr(0, i) + newext;
	}

	std::wstring substitute_filename(const std::wstring& filepath, const std::wstring& newname) {
		auto i_start = filepath.find_last_of(L"/\\");
		if (i_start == std::wstring::npos)
			i_start = 0;
		auto i_end = filepath.find_last_of(L'.');
		if (i_end == std::wstring::npos)
			i_end = filepath.size() - 1;
		return filepath.substr(0, i_start) + L"\\" + newname + filepath.substr(i_end);
	}

	std::wstring get_filedir(const std::wstring& filename) {
		return filename.substr(0, filename.find_last_of(L"/\\"));
	}

	std::wstring get_filename(const std::wstring& filename) {
		auto i = filename.find_last_of(L"/\\");
		if (i == std::wstring::npos)
			return L"";
		return filename.substr(i + 1);
	}

	bool is_file_exists(const std::wstring& filename) {
		FILE *f;
		_wfopen_s(&f, filename.c_str(), L"r");
		if (!f)
			return false;
		fclose(f);
		return true;
	}

	bool is_directory_exists(const std::wstring& dirpath) {
		struct _stat64i32 s;
		if (_wstat(dirpath.c_str(), &s) == 0) {
			if (s.st_mode & S_IFDIR)
				return true;
			else
				return false;
		}
		else {
			return false;
		}
	}

	std::wstring get_parentdir(const std::wstring& dirpath) {
		auto i = dirpath.find_last_of(L"/\\");
		if (i == std::wstring::npos)
			return L"";
		return dirpath.substr(0, i);
	}

	bool create_directory(const std::wstring& filepath) {
		return (_wmkdir(filepath.c_str()) == 0);
	}

	bool make_parent_directory_recursive(const std::wstring& filepath)
	{
		// if current directory is not exist
		// then get parent directory
		// and check it recursively
		// after that, create own.
		if (is_directory_exists(filepath))
			return true;
		if (is_file_exists(filepath))
			return false;
		if (!create_directory(filepath.c_str())) {
			std::wstring parent = get_parentdir(filepath);
			if (NOT(make_parent_directory_recursive(parent))) {
				return false;
			}
			return create_directory(filepath.c_str());
		}
	}

	std::wstring make_filename_safe(const std::wstring& filepath) {
		std::wstring fn = filepath;
#define REPLACESTR(s, o, r) (std::replace((s).begin(), (s).end(), (o), (r)))
		REPLACESTR(fn, L'/', L'_');
#ifdef _WIN32
			REPLACESTR(fn, L'|', L'_');
			REPLACESTR(fn, L'\\', L'_');
			REPLACESTR(fn, L':', L'_');
			REPLACESTR(fn, L'*', L'_');
			REPLACESTR(fn, L'<', L'_');
			REPLACESTR(fn, L'>', L'_');
#endif
		return fn;
	}
#endif
	std::string substitute_extension(const std::string& filepath, const std::string& newext) {
		auto i = filepath.find_last_of(L'.');
		if (i == std::string::npos)
			return filepath + newext;
		return filepath.substr(0, i) + newext;
	}

	std::string substitute_filename(const std::string& filepath, const std::string& newname) {
		auto i_start = filepath.find_last_of("/\\");
		if (i_start == std::string::npos)
			i_start = 0;
		auto i_end = filepath.find_last_of(L'.');
		if (i_end == std::string::npos)
			i_end = filepath.size() - 1;
		return filepath.substr(0, i_start) + "/" + newname + filepath.substr(i_end);
	}

	std::string get_filedir(const std::string& filename) {
		return filename.substr(0, filename.find_last_of("/\\"));
	}

	std::string get_filename(const std::string& filename) {
		auto i = filename.find_last_of("/\\");
		if (i == std::string::npos)
			return "";
		return filename.substr(i + 1);
	}

	bool is_file_exists(const std::string& filename) {
		FILE *f = openfile(filename.c_str(), "r");
		if (!f)
			return false;
		fclose(f);
		return true;
	}

	bool is_directory_exists(const std::string& dirpath) {
#ifdef _WIN32
		wchar_t buf[1024];
		ENCODING::utf8_to_wchar(dirpath.c_str(), buf, 1024);
		struct _stati64 s;
		if (_wstat64(buf, &s) == 0) {
#else
		struct stat s;
		if (stat(dirpath.c_str(), &s) == 0) {
#endif
			if (s.st_mode & S_IFDIR)
				return true;
			else
				return false;
		}
		else {
			return false;
		}
	}

	std::string get_parentdir(const std::string& dirpath) {
		auto i = dirpath.find_last_of("/\\");
		if (i == std::string::npos)
			return "";
		return dirpath.substr(0, i);
	}

	bool create_directory(const std::string& filepath) {
#ifdef _WIN32
		wchar_t buf[1024];
		ENCODING::utf8_to_wchar(filepath.c_str(), buf, 1024);
		return (_wmkdir(buf) == 0);
#else
		// WARNING buffer overflow attack
		char buf[1024];
		sprintf(buf, "mkdir -p %s", filepath.c_str());
		if (!system(buf)) return false; else return true;
#endif
	}

	bool make_parent_directory_recursive(const std::string& filepath)
	{
#ifdef _WIN32
		// if current directory is not exist
		// then get parent directory
		// and check it recursively
		// after that, create own.
		if (is_directory_exists(filepath))
			return true;
		if (is_file_exists(filepath))
			return false;
		if (!create_directory(filepath.c_str())) {
			std::string parent = get_parentdir(filepath);
			if (NOT(make_parent_directory_recursive(parent))) {
				return false;
			}
			return create_directory(filepath.c_str());
		}
#else
		create_directory(filepath);
#endif
	}

	std::string make_filename_safe(const std::string& filepath) {
		std::string fn = filepath;
#define REPLACESTR(s, o, r) (std::replace((s).begin(), (s).end(), (o), (r)))
//		for (int i=0; i<(s).length(); i++) if ((s)[i] == o) (s)[i] = r;)
		REPLACESTR(fn, '/', '_');
		REPLACESTR(fn, '\'', '_');
		REPLACESTR(fn, '\"', '_');
#ifdef _WIN32
		REPLACESTR(fn, '|', '_');
		REPLACESTR(fn, '\\', '_');
		REPLACESTR(fn, ':', '_');
		REPLACESTR(fn, '*', '_');
		REPLACESTR(fn, '<', '_');
		REPLACESTR(fn, '>', '_');
#endif
		return fn;
	}

	FILE* openfile(const char* filepath, const char* mode) {
#ifdef _WIN32
		wchar_t filepath_w[1024], mode_w[20];
		ENCODING::utf8_to_wchar(filepath, filepath_w, 1024);
		ENCODING::utf8_to_wchar(mode, mode_w, 20);
		return openfile(filepath_w, mode_w);
#else
		return fopen(filepath, mode);
#endif
	}

#ifdef _WIN32
	FILE* openfile(const wchar_t* filepath, const wchar_t* mode) {
		FILE *f = 0;
		_wfopen_s(&f, filepath, mode);
		return f;
	}
#endif
}

namespace ENCODING {
	bool wchar_to_utf8(const wchar_t *org, char *out, int maxlen)
	{
		iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
		if (cd == (void*)-1)
			return false;

		out[0] = 0;
		const char *buf_iconv = (const char*)org;
		char *but_out_iconv = (char*)out;
		size_t len_in = wcslen(org) * 2;
		size_t len_out = maxlen;

		int r = iconv(cd, &buf_iconv, &len_in, &but_out_iconv, &len_out);
		if ((int)r == -1)
			return false;
		*but_out_iconv = 0;

		return true;
	}
	bool utf8_to_wchar(const char *org, wchar_t *out, int maxlen)
	{
		iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
		if (cd == (void*)-1)
			return false;

		out[0] = 0;
		const char *buf_iconv = (const char*)org;
		char *buf_out_iconv = (char*)out;
		size_t len_in = strlen(org);
		size_t len_out = maxlen;

		int r = iconv(cd, &buf_iconv, &len_in, &buf_out_iconv, &len_out);
		if ((int)r == -1)
			return false;
		*buf_out_iconv = *(buf_out_iconv+1) = 0;

		return true;
	}
}
