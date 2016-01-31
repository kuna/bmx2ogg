#pragma once

#include <stdio.h>
#include <string>

// TODO: obsolete. should remove this.
#ifdef _WIN32
#define PATH_SEPARATOR		"\\"
#define PATH_SEPARATOR_CHAR	'\\'
#else
#define PATH_SEPARATOR		"/"
#define PATH_SEPARATOR_CHAR	'/'
#endif

#define NOT(v) (!(v))

// win32
#ifdef _WIN32
#define USE_MBCS
#endif

// vs compatible
#ifndef __STDC_WANT_SECURE_LIB__

typedef long long int __int64;
#define vsprintf_s(buf, fmt, vl)            vsprintf(buf, fmt, vl)
#define strcpy_s(dst, size, src)            strcpy(dst, src)
#define scanf_s(fmt, size, ...)             scanf(fmt, ##__VA_ARGS__)
#define sprintf_s(buf, fmt, ...)            sprintf(buf, fmt, ##__VA_ARGS__)
#define memmove_s(dst, size, src, count)    memmove(dst, src, count)
#define gets_s(buf, size)                   gets(buf)
#define fopen_s(pFile,filename,mode)        ((*(pFile))=fopen((filename),(mode)))==NULL
#define _fopen_s(a,b,c)                     fopen_s(a,b,c)

#else

#endif // __STDC_WANT_SECURE_LIB__

// it's unnecessary now but ... I'm too lazy to remove this
#define _T(x) x

std::string to_string(int i);

namespace IO {
#ifdef _WIN32
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
#endif
	std::string substitute_extension(const std::string& filepath, const std::string& newext);
	std::string substitute_filename(const std::string& filepath, const std::string& newname);	// excludes extension
	std::string get_filedir(const std::string& filename);
	std::string get_filename(const std::string& filename);
	bool is_file_exists(const std::string& filename);
	bool is_directory_exists(const std::string& dirpath);
	std::string get_parentdir(const std::string& dirpath);
	bool create_directory(const std::string& filepath);
	bool make_parent_directory_recursive(const std::string& filepath);
	std::string make_filename_safe(const std::string& filepath);

	/* utf8 filepath */
	FILE* openfile(const char* filepath, const char* mode);
#ifdef _WIN32
	FILE* openfile(const wchar_t* filepath, const wchar_t* mode);
#endif
}

namespace ENCODING {
	bool utf8_to_wchar(const char *org, wchar_t *out, int maxlen);
	bool wchar_to_utf8(const wchar_t *org, char *out, int maxlen);
}
