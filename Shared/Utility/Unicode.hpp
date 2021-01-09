#pragma once
#include <iostream>     // for std::cout
#include <ostream>      // for std::endl
#include <system_error>
#include <string>
#include <cwchar>       // for mbrtowc
//#include <Shared/Utility/SysIncludes.hpp>

namespace util
{
	using namespace std::literals;

	//-----------------------------------------------------------------------------
	// https://stackoverflow.com/questions/12917098/conversion-from-ascii-to-unicode-char-code-freetype2
	// Modern STL-based C++ wrapper to Win32's MultiByteToWideChar() C API.
	// (based on http://code.msdn.microsoft.com/windowsdesktop/C-UTF-8-Conversion-Helpers-22c0a664)
	//-----------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------
	// Converts an ANSI/MBCS string to Unicode UTF-8.
	//------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------
	inline std::string utf8(const std::string& source, const UINT source_code_page = CP_ACP)
	{
		const int size =
			MultiByteToWideChar(source_code_page, MB_COMPOSITE, source.c_str(), static_cast<int>(source.length()), nullptr, 0);
		std::wstring utf16_str(size, '\0');
		MultiByteToWideChar(source_code_page, MB_COMPOSITE, source.c_str(), static_cast<int>(source.length()), &utf16_str[0], size);
		const int utf8_size =
			WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), static_cast<int>(utf16_str.length()), nullptr, 0, nullptr, nullptr);
		std::string utf8_str(utf8_size, '\0');
		WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), static_cast<int>(utf16_str.length()), &utf8_str[0], utf8_size, nullptr, nullptr);
		return utf8_str;
	}

	//-----------------------------------------------------------------------------
	// Converts an ANSI/MBCS string to Unicode UTF-16.
	// Wraps MultiByteToWideChar() using modern C++ and STL.
	// Throws a StringConversionException on error.
	//-----------------------------------------------------------------------------
	inline std::wstring utf16(const std::string& source, const UINT codePage = CP_UTF8)
	{
		if (source.empty())
		{
			return std::wstring();
		}

		// Fail if an invalid input character is encountered
		static const DWORD conversion_flags = MB_ERR_INVALID_CHARS;

		// Require size for destination string
		const int utf16_length = MultiByteToWideChar(
			codePage,							// code page for the conversion
			conversion_flags,					// flags
			source.c_str(),						// source string
			static_cast<int>(source.length()),	// length (in chars) of source string
			nullptr,							// unused - no conversion done in this step
			0									// request size of destination buffer, in wchar_t's
		);
		if (utf16_length == 0)
		{
			throw std::system_error(std::error_code(GetLastError(), std::system_category()),
				"MultiByteToWideChar() failed: Can't get length of destination UTF-16 string."s);
		}

		// Allocate room for destination string
		std::wstring utf16_text;
		utf16_text.resize(utf16_length);

		// Convert to Unicode UTF-16
		if (!MultiByteToWideChar(
			codePage,								// code page for conversion
			0,										// validation was done in previous call
			source.c_str(),							// source string
			static_cast<int>(source.length()),		// length (in chars) of source string
			&utf16_text[0],							// destination buffer
			static_cast<int>(utf16_text.length())	// size of destination buffer, in wchar_t's
		)) 
		{
			throw std::system_error(std::error_code(GetLastError(), std::system_category()),
				"MultiByteToWideChar() failed: Can't convert to UTF-16 string."s);
		}

		return utf16_text;
	}

	//-----------------------------------------------------------------------------
	// Converts an Unicode UTF-16 string to ANSI/MBCS.
	// Wraps MultiByteToWideChar() using modern C++ and STL.
	// Throws a StringConversionException on error.
	//-----------------------------------------------------------------------------
	inline std::string mbcs(const std::wstring& source, const UINT codePage = CP_UTF8)
	{
		if (source.empty())
		{
			return std::string();
		}

		// Fail if an invalid input character is encountered
		static const DWORD conversion_flags = WC_ERR_INVALID_CHARS;

		// Require size for destination string
		const int ansi_length = WideCharToMultiByte(
			codePage,							// code page for the conversion
			conversion_flags,					// flags
			source.c_str(),						// source string
			static_cast<int>(source.length()),	// length (in chars) of source string
			nullptr,							// unused - no conversion done in this step
			0,									// request size of destination buffer, in wchar_t's
			nullptr, nullptr
		);
		if (ansi_length == 0) 
		{
			throw std::system_error(std::error_code(GetLastError(), std::system_category()),
				"WideCharToMultiByte() failed: Can't get length of destination ANSI string."s);
		}

		// Allocate room for destination string
		std::string ansi_text;
		ansi_text.resize(ansi_length);

		// Convert to Unicode UTF-16
		if (!WideCharToMultiByte(
			codePage,								// code page for conversion
			0,										// validation was done in previous call
			source.c_str(),							// source string
			static_cast<int>(source.length()),		// length (in chars) of source string
			&ansi_text[0],							// destination buffer
			static_cast<int>(ansi_text.length()),	// size of destination buffer, in wchar_t's
			nullptr, nullptr
		)) 
		{
			throw std::system_error(std::error_code(GetLastError(), std::system_category()),
				"WideCharToMultiByte() failed: Can't convert to ANSI string."s);
		}

		return ansi_text;
	}

	//-----------------------------------------------------------------------------------------------------
	// Converts an ANSI/MBCS char to Unicode UTF-16.
	// Throws a StringConversionException on error.
	//-----------------------------------------------------------------------------------------------------
	inline wchar_t utf16(char source)
	{
		std::mbstate_t state = std::mbstate_t();
		wchar_t result = 0;
		if (std::mbrtowc(&result, &source, 1, &state) < 0)
		{
			throw std::system_error(std::error_code(errno, std::system_category()),
				"mbrtowc() failed: Can't convert to utf16 char."s);
		}
	}
}
