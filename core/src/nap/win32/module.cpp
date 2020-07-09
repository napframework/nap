#include "../module.h"
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
		SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS);
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
		// Temporarily set search paths for this module's dependencies
		std::vector<DLL_DIRECTORY_COOKIE> searchPathCookies;
		if (modInfo.getProjectInfo().isEditorMode())
			searchPathCookies = addDLLSearchPaths(modInfo.mLibSearchPaths);

		auto modulefile = utility::getAbsolutePath(modulePath);

		if (!utility::fileExists(modulefile))
		{
			errorString = "File not found: " + modulefile;
			return nullptr;
		}

		// Load our module
		void* result;
		result = LoadLibraryA(modulefile.c_str());

		// If we failed to load the module, get the error string
		if (!result)
			errorString = getLastErrorStr();

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