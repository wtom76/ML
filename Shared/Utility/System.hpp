#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <system_error>
#include <filesystem>
//#include <Shared/Utility/Unicode.hpp>

namespace util
{
	//---------------------------------------------------------------------------------------------------------
	inline std::string environmentVariable(const char* variable_name)
	{
		std::vector<char> buf(1024);
		size_t buff_size = 0;
		errno_t err = getenv_s(&buff_size, buf.data(), buf.size(), variable_name);
		if (err == ERANGE)
		{
			buf.resize(buff_size);
			err = getenv_s(&buff_size, buf.data(), buf.size(), variable_name);
		}
		if (err)
		{
			throw std::system_error(std::error_code(err, std::system_category()),
				std::string("Failed to get ") + variable_name + " environment variable");
		}
		return buf.data();
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::string buildFolderPath(const std::string& root, const std::string& folder_name)
	{
		std::string path(root);

		if (path.empty())
		{
			path.append(".\\");
		}
		else if (path.back() != '\\' && path.back() != '/')
		{
			path.append("\\");
		}
		path.append(folder_name);
		path.append("\\");

		return path;
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::string getLogRoot(const std::string& folder_name)
	{
		static const std::string var_value(environmentVariable("LOG_ROOT"));
		return buildFolderPath(var_value, folder_name);
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::string configRoot(const std::string& folder_name)
	{
		static const std::string var_value(environmentVariable("CFG_ROOT"));
		return buildFolderPath(var_value, folder_name);
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::wstring exeFileNameW()
	{
		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (argv == nullptr || argc <= 0)
		{
			throw std::system_error(GetLastError(), std::generic_category(),
				"CommandLineToArgvW or GetCommandLineW failed.");
		}
		std::wstring name = argv[0];
		const size_t slash_idx = name.find_last_of(L"/\\");
		const size_t dot_idx = name.rfind(L'.');
		if (slash_idx == std::wstring::npos)
		{
			return name;
		}
		return name.substr(slash_idx + 1, dot_idx > slash_idx ? dot_idx - slash_idx - 1 : std::wstring::npos);
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::string exeFileName()
	{
		return mbcs(exeFileNameW());
	}
	//---------------------------------------------------------------------------------------------------------
	// directory + file name + file ext
	inline std::filesystem::path exePath()
	{
		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (argv == nullptr || argc <= 0)
		{
			throw std::system_error(GetLastError(), std::generic_category(),
				"CommandLineToArgvW or GetCommandLineW failed.");
		}
		return mbcs(argv[0]);
	}
	//---------------------------------------------------------------------------------------------------------
	inline bool createConsole(HWND hwnd)
	{
		if (!AllocConsole())
		{
			MessageBox(hwnd, L"Console creation failed", exeFileNameW().c_str(), MB_OK);
			return false;
		}
		FILE* fp = nullptr;
		if (_wfreopen_s(&fp, L"CONOUT$", L"w", stdout) != 0)
		{
			MessageBox(hwnd, L"cout redirection failed", exeFileNameW().c_str(), MB_OK);
			return false;
		}
		SetConsoleOutputCP(1251);
		return true;
	}
	//---------------------------------------------------------------------------------------------------------
	inline void disableConsoleCloseButton()
	{
		if (const HWND console_wnd = GetConsoleWindow())
		{
			if (const HMENU menu = GetSystemMenu(console_wnd, FALSE))
			{
				RemoveMenu(menu, SC_CLOSE, MF_BYCOMMAND);
			}
		}
	}
	//---------------------------------------------------------------------------------------------------------
	// Ret is assumed to have constructor with size as a single parameter
	template <class Ret>
	inline Ret readAll(const std::string& path, std::ios::openmode open_mode)
	{
		std::ifstream f(path, open_mode|std::ios::ate);
		std::ifstream::pos_type pos = f.tellg();
		Ret ret(pos, '\0');
		f.seekg(0, std::ios::beg);
		f.read(ret.data(), pos);
		return ret;
	}
	//---------------------------------------------------------------------------------------------------------
	template <class Ret>
	inline Ret readAllAsBinary(const std::string& path)
	{
		return readAll<Ret>(path, std::ios::binary);
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::string readAllAsText(const std::string& path)
	{
		std::stringstream buffer;
		buffer << std::ifstream(path).rdbuf();
		return buffer.str();
	}

	//----------------------------------------------------------------------------------------------------------
	template <typename NewLineHandler, typename FieldHandler>
	inline void readCsv(const std::string& filename, bool header, NewLineHandler&& new_line_handler, FieldHandler&& field_handler)
	{
		constexpr char delim = ';';
		constexpr char quote = '"';

		std::ifstream file(filename);
		if (file.bad())
		{
			throw std::runtime_error(
				std::string("Can't open file \"") + filename + "\". error: " + std::to_string(GetLastError()));
		}
		std::string line;
		std::string field;
		if (header && !std::getline(file, line))
		{
			return;
		}
		while (std::getline(file, line))
		{
			new_line_handler();
			std::stringstream sstrm(line);
			while (std::getline(sstrm, field, ';'))
			{
				if (quote && !field.empty() && field.front() == quote && field.back() == quote)
				{
					field_handler(std::move(field.substr(1, field.size() - 2)));
				}
				else
				{
					field_handler(std::move(field));
				}
			}
		}
	}
}
