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
		std::vector<DLL_DIRECTORY_COOKIE> dllDirCookies;
		for (const auto& searchPath : paths)
		{
			auto abspath = utility::getAbsolutePath(searchPath);
			auto abspathw = toWStr(abspath);

			if (!utility::fileExists(abspath))
			{
				nap::Logger::fine("Path does not exist: %s (resolved from: %s)", abspath.c_str(), searchPath.c_str());
				continue;
			}

			auto cookie = AddDllDirectory(abspathw.c_str());
			if (!cookie)
			{
				nap::Logger::error("Failed to add dll path: %s (%s)", abspath.c_str(), getLastErrorStr().c_str());
				continue;
			}
			dllDirCookies.emplace_back(cookie);
		}
		return dllDirCookies;
	}


	// Remove dll search paths added by addDLLSearchPaths
	void removeDLLSearchPaths(const std::vector<DLL_DIRECTORY_COOKIE>& paths)
	{
		for (const auto& path : paths)
		{
			if (!RemoveDllDirectory(path))
				nap::Logger::error("Failed to remove path: %s", path);
		}
	}


	void initModules()
	{
		// On windows, disable DLL load failure dialog boxes (we handle the errors ourselves))
		SetErrorMode(SEM_FAILCRITICALERRORS);
	}


	void* loadModule(const nap::ModuleInfo& modInfo, const std::string& modulePath, std::string& errorString)
	{
		auto modulefile = utility::getAbsolutePath(modulePath);
		if (!utility::fileExists(modulefile))
		{
			errorString = nap::utility::stringFormat("Library doesn't exist: %s", modulefile.c_str());
			return nullptr;
		}

		// Construct library search path
		std::vector search_paths = { nap::utility::getFileDir(modulefile) };
		search_paths.reserve(search_paths.size() + modInfo.mLibSearchPaths.size());

		// Add module search paths for dependencies
		std::vector<DLL_DIRECTORY_COOKIE> searchPathCookies;
		if (modInfo.getProjectInfo().isEditorMode())
			search_paths.insert(search_paths.end(), modInfo.mLibSearchPaths.begin(), modInfo.mLibSearchPaths.end());

		// Temporarily set search paths for this module's dependencies
		searchPathCookies = addDLLSearchPaths(search_paths);

		// Load our module -> note that we search on file name instead of full path.
		// This allows the loader to return an already loaded module, speeding up the entire sourcing process
		// for both applications, the editor and applets.
		auto module_file_name = nap::utility::getFileName(modulefile);
		void* result = LoadLibraryExA(module_file_name.c_str(), 0, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

		// If we failed to load the module, get the error string
		if (result == nullptr)
		{
			// Normally we would return the last error code, but `getLastErrorStr` returns information that is easy to mis-interpret.
			// For example: if a dependency of the dll can't can't be found it states that the entire module can't be found.
			// However: we know the library exists and the issue is most likely related to a mis-configured dll search path.
			// Therefore, instead of returning the last error code as a string we attempt to be a bit more verbose. 
			// errorString = getLastErrorStr();
			errorString += utility::stringFormat("Library can't be loaded, most likely due to missing or mis-configured DLL search paths or a compiler mis-match", modulefile.c_str());
		}

		// Remove temporarily added search paths
		if (modInfo.getProjectInfo().isEditorMode())
			removeDLLSearchPaths(searchPathCookies);

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
