#pragma once

#include <string>

#ifdef _WIN32
#define PATH_SEPARATOR		L"\\"
#define PATH_SEPARATOR_CHAR	L'\\'
#else
#define PATH_SEPARATOR		L"/"
#define PATH_SEPARATOR_CHAR	L'/'
#endif

#define NOT(v) (!(v))
std::wstring to_wstring(int i);

namespace IO {
	std::wstring substitute_extension(const std::wstring& filepath, const std::wstring& newext);
	std::wstring substitute_filename(const std::wstring& filepath, const std::wstring& newname);	// excludes extension
	std::wstring get_filedir(const std::wstring& filename);
	std::wstring get_filename(const std::wstring& filename);
	bool is_file_exists(const std::wstring& filename);
	bool is_directory_exists(const std::wstring& dirpath);
	std::wstring get_parentdir(const std::wstring& dirpath);
	bool create_directory(const std::wstring& filepath);
	bool make_parent_directory_recursive(const std::wstring& filepath);
	std::wstring make_filename_safe(const std::wstring& filepath);
}

namespace ENCODING {
	bool wchar_to_utf8(const wchar_t *org, char *out, int maxlen);
}