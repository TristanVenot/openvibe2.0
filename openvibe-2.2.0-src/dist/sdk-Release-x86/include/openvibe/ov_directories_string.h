#pragma once

#include <cstdlib>	 // For getenv()
#include <string>

#if defined TARGET_OS_Windows
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include "m_ConverterUtf8.h"
#include <memory>
#elif defined TARGET_OS_Linux
#include <unistd.h>
#include <cstring>
#elif defined TARGET_OS_MacOS
#include <mach-o/dyld.h>
#endif

namespace OpenViBE
{
	class StringDirectories
	{
	public:
		StringDirectories() = delete;

		static std::string getDistRootDir()
		{
#ifdef OV_USE_CMAKE_DEFAULT_PATHS
			return pathFromEnv("OV_PATH_ROOT", OV_CMAKE_PATH_ROOT);
#else
			return pathFromEnv("OV_PATH_ROOT", guessRootDir().c_str());
#endif
		}
		static std::string getBinDir()
		{
			return pathFromEnvOrExtendedRoot("OV_PATH_BIN", "/bin", OV_CMAKE_PATH_BIN);
		}
		static std::string getDataDir()
		{
			return pathFromEnvOrExtendedRoot("OV_PATH_DATA", "/share/openvibe", OV_CMAKE_PATH_DATA);
		}
		static std::string getLibDir()
		{
#if defined TARGET_OS_Windows
			return pathFromEnvOrExtendedRoot("OV_PATH_LIB", "/bin", OV_CMAKE_PATH_BIN);
#else
			return pathFromEnvOrExtendedRoot("OV_PATH_LIB", "/lib", OV_CMAKE_PATH_LIB);
#endif
		}
		static std::string getUserHomeDir()
		{
#if defined TARGET_OS_Windows
			return pathFromEnv("USERPROFILE", "openvibe-user");
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			return pathFromEnv("HOME", "openvibe-user");
#endif
		}
		static std::string getUserDataDir()
		{
#if defined TARGET_OS_Windows
			return  (pathFromEnv("APPDATA", "openvibe-user") + "/" + OV_CONFIG_SUBDIR);
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			return getUserHomeDir() + "/.config/" + OV_CONFIG_SUBDIR;
#endif
		}
		static std::string getAllUsersDataDir()
		{
#if defined TARGET_OS_Windows
			// first chance: Win7 and higher
			std::string l_sPath = pathFromEnv("PROGRAMDATA", "");
			if(l_sPath == "")
			{
				// second chance: WinXP
				l_sPath = pathFromEnv("ALLUSERSPROFILE", "");
			}
			// fallback
			if(l_sPath == "")
			{
				l_sPath = "openvibe-user";
			}

			return l_sPath + "/" + OV_CONFIG_SUBDIR;
#elif defined TARGET_OS_Linux || defined TARGET_OS_MacOS
			// On Linux, we redirect to current user data folder
			return getUserHomeDir() + "/.config/" + OV_CONFIG_SUBDIR;
#endif
		}
		static std::string getLogDir(void)
		{
			return getUserDataDir() + "/log";
		}

	private:

		// Used to convert \ in paths to /, we need this because \ is a special character for .conf token parsing
		static std::string convertPath(const std::string &strIn)
		{
			std::string l_sOut(strIn);
			size_t l_sLen = strIn.length();
			for (size_t i = 0; i<l_sLen; i++)
			{
				if(strIn[i]=='\\')
				{
					l_sOut[i]='/';
				}
			}
			return l_sOut;
		}

		/// Try to guess the root directory by assuming that any program that uses the kernel is in the bin
		/// subdirectory of the dist folder.
		static std::string guessRootDir()
		{
			static std::string rootDir;
			if (!rootDir.empty())
			{
				return rootDir;
			}

			std::string fullpath;
#if defined TARGET_OS_Windows
			// Unlike GetEnvironmentVariableW, this function can not return the length of the actual path
			std::unique_ptr<wchar_t> utf16value(new wchar_t[1024]);
			GetModuleFileNameW(nullptr, utf16value.get(), 1024);
			int multiByteSize = WideCharToMultiByte(CP_UTF8, 0, utf16value.get(), -1, nullptr, 0, nullptr, nullptr);
			if (multiByteSize == 0) {
				// There are no sensible values to return if the above call fails and the program will not be
				// able to run in any case.
				std::abort();
			}
			std::unique_ptr<char> utf8Value(new char[static_cast<size_t>(multiByteSize)]);
			if (WideCharToMultiByte(CP_UTF8, 0, utf16value.get(), -1, utf8Value.get(), multiByteSize, nullptr, nullptr) == 0) {
				std::abort();
			}

			fullpath = convertPath(utf8Value.get());
#elif defined TARGET_OS_Linux
			char path[2048];
			memset(path, 0, sizeof(path)); // readlink does not produce 0 terminated strings
			readlink("/proc/self/exe", path, sizeof(path));
			fullpath = std::string(path);
#elif defined TARGET_OS_MacOS
			uint32_t size = 0;
			_NSGetExecutablePath(nullptr, &size);
			std::unique_ptr<char> path(new char[size + 1]);

			if (_NSGetExecutablePath(path.get(), &size) != 0)
			{
				std::abort();
			}

			fullpath = std::string(path.get());
#endif
			auto slash_before_last = fullpath.find_last_of('/', fullpath.find_last_of('/') - 1);
			rootDir = fullpath.substr(0, slash_before_last);
			return rootDir;
		}

		// Returns ENV variable value or sDefaultPath if the variable doesn't exist. The path is converted with each \ to /.
		static std::string pathFromEnv(const char *sEnvVar, const char *sDefaultPath)
		{
#if defined TARGET_OS_Windows
			// Using std::getenv on Windows yields UTF7 strings which do not work with the utf8_to_utf16 function
			// as this seems to be the only place where we actually get UTF7, let's get it as UTF16 by default
			DWORD wideBufferSize = GetEnvironmentVariableW(Common::Converter::utf8_to_utf16(sEnvVar).c_str(), nullptr, 0);
			if (wideBufferSize == 0) {
				return convertPath(sDefaultPath);
			}
			std::unique_ptr<wchar_t> utf16value(new wchar_t[wideBufferSize]);
			GetEnvironmentVariableW(Common::Converter::utf8_to_utf16(sEnvVar).c_str(), utf16value.get(), wideBufferSize);

			int multiByteSize = WideCharToMultiByte(CP_UTF8, 0, utf16value.get(), -1, nullptr, 0, nullptr, nullptr);
			if (multiByteSize == 0) {
				return convertPath(sDefaultPath);
			}
			std::unique_ptr<char> utf8Value(new char[static_cast<size_t>(multiByteSize)]);
			if (WideCharToMultiByte(CP_UTF8, 0, utf16value.get(), -1, utf8Value.get(), multiByteSize, nullptr, nullptr) == 0) {
				return convertPath(sDefaultPath);
			}

			const char* l_sPathPtr = utf8Value.get();
#else
			const char *l_sPathPtr = std::getenv(sEnvVar);
#endif
			std::string l_sPath = (l_sPathPtr ? l_sPathPtr : sDefaultPath);
			return convertPath(l_sPath);
		}

		// Returns ENV variable if it is defined, otherwise it extends the ROOT variable if it exists, finally returns a default path
		static std::string pathFromEnvOrExtendedRoot(const char* envVar, const char* rootPostfix, const char* defaultPath)
		{
			if (std::getenv(envVar))
			{
				return pathFromEnv(envVar, defaultPath);
			}
			if (std::getenv("OV_PATH_ROOT"))
			{
				// the default case for this one is wrong but it should never happen
				return pathFromEnv("OV_PATH_ROOT", "") + rootPostfix;
			}
#ifdef OV_USE_CMAKE_DEFAULT_PATHS
			return convertPath(defaultPath);
#else
			return guessRootDir() + rootPostfix;
#endif
		}
	};
}
