#include "bmx2wav_common.h"

#include "iconv\iconv.h"
#include <sstream>
#include <sys/stat.h>
#include <wchar.h>
#include <array>

std::wstring to_wstring (int i) { std::wostringstream wss; wss << i; return wss.str(); }

namespace IO {
	std::wstring substitute_extension(const std::wstring& filepath, const std::wstring& newext) {
		auto i = filepath.find_last_of(L'.');
		if (i == std::wstring::npos)
			return filepath + newext;
		return filepath.substr(0, i) + newext;
	}

	std::wstring substitute_filename(const std::wstring& filepath, const std::wstring& newname) {
		auto i_start = filepath.find_last_of(PATH_SEPARATOR_CHAR);
		if (i_start == std::wstring::npos)
			i_start = 0;
		auto i_end = filepath.find_last_of(L'.');
		if (i_end == std::wstring::npos)
			i_end = filepath.size() - 1;
		return filepath.substr(0, i_start) + PATH_SEPARATOR + newname + filepath.substr(i_end);
	}

	std::wstring get_filedir(const std::wstring& filename) {
		return filename.substr(0, filename.find_last_of(PATH_SEPARATOR_CHAR));
	}

	std::wstring get_filename(const std::wstring& filename) {
		auto i = filename.find_last_of(PATH_SEPARATOR_CHAR);
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
		auto i = dirpath.find_last_of(PATH_SEPARATOR_CHAR);
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
		std::wstring fn = get_filename(filepath);
#define REPLACESTR(s, o, r) (std::replace((s).begin(), (s).end(), (o), (r)))
		REPLACESTR(fn, L'/', L'_');
		if (_WIN32) {
			REPLACESTR(fn, L'|', L'_');
			REPLACESTR(fn, L'\\', L'_');
			REPLACESTR(fn, L':', L'_');
			REPLACESTR(fn, L'*', L'_');
			REPLACESTR(fn, L'<', L'_');
			REPLACESTR(fn, L'>', L'_');
		}
		return get_filedir(filepath) + PATH_SEPARATOR + fn;
	}
}

namespace ENCODING {
	bool wchar_to_utf8(const wchar_t *org, char *out, int maxlen)
	{
		iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
		if ((int)cd == -1)
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
}