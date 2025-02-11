/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <nap/module.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <nap/logger.h>
#include <windows.h> // Windows dll loading

namespace nap
{
	std::wstring toWStr(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}


	std::string getLastErrorStr()
	{
		// Get the error code
		DWORD error_code = ::GetLastError();
		if (error_code == 0)
			return std::string();

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);
		LocalFree(messageBuffer);

		return utility::rTrim(message);
	}


	// Add dll search paths such that when we load a module, it's dependencies can be resolved by Windows
	std::vector<DLL_DIRECTORY_COOKIE> addDLLSearchPaths(const std::vector<std::string>& paths)
	{
		std::vector<DLL_DIRECTORY_COOKIE> all_cookies;
		for (const auto& path : paths)
		{
			auto abspath = utility::getAbsolutePath(path);
			nap::Logger::debug("Adding library search path: %s", abspath.c_str());
			if (!utility::fileExists(abspath))
			{
				nap::Logger::fine("Path does not exist: %s (resolved from: %s)", abspath.c_str(), path.c_str());
				continue;
			}
			auto cookie = AddDllDirectory(toWStr(abspath).c_str());
			if (!cookie)
			{
				nap::Logger::error("Failed to add dll path: %s (%s)", abspath.c_str(), getLastErrorStr().c_str());
				continue;
			}
			all_cookies.emplace_back(cookie);
		}
		return all_cookies;
	}


	// Remove dll search paths added by addDLLSearchPaths
	void removeDLLSearchPaths(const std::vector<DLL_DIRECTORY_COOKIE>& paths)
	{
		for (const auto& path : paths)
		{
			if (!RemoveDllDirectory(path))
				nap::Logger::error("Failed to remove library search path");
		}
	}


	void initModules()
	{
		// On windows, disable DLL load failure dialog boxes (we handle the errors ourselves))
		SetErrorMode(SEM_FAILCRITICALERRORS);
	}


	void* loadModule(const nap::ModuleInfo& modInfo, const std::string& library, std::string& outLocation, std::string& errorString)
	{
		// Add library search paths to resolve library location when using napkin
		std::vector<DLL_DIRECTORY_COOKIE> search_cookies;
		if (modInfo.getProjectInfo().isEditorMode())
			search_cookies = addDLLSearchPaths(modInfo.mLibSearchPaths);

		// Load our module -> note that we search on file name instead of full path.
		// This allows the loader to return an already loaded module, speeding up the entire sourcing process
		// for both applications, the editor and applets.
		HMODULE result = LoadLibraryExA(library.c_str(), 0, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

		// If we failed to load the module, get the error string
		if (result == nullptr)
		{
			// Normally we would return the last error code, but `getLastErrorStr` returns information that is easy to mis-interpret.
			// For example: if a dependency of the dll can't can't be found it states that the entire module can't be found.
			// However: we know the library exists and the issue is most likely related to a mis-configured dll search path.
			// Therefore, instead of returning the last error code as a string we attempt to be a bit more verbose. 
			// errorString = getLastErrorStr();
			errorString += getLastErrorStr();
			errorString += utility::stringFormat("\n'%s' can't be loaded, most likely due to missing or mis-configured DLL search paths or a compiler mis-match",
				library.c_str());
		}

		// Fetch location
		std::array<char, 256> loc;
		if (GetModuleFileNameA(result, loc.data(), loc.size()) > 0)
			outLocation = loc.data();

		// Remove temporarily added search paths
		removeDLLSearchPaths(search_cookies);
		return result;
	}


	void unloadModule(void* module)
	{
		FreeLibrary((HMODULE)module);
	}


	void* findSymbolInModule(void* module, const char* symbolName)
	{
		return GetProcAddress((HMODULE)module, symbolName);
	}


	std::string getModuleExtension()
	{
		return "dll";
	}
}
