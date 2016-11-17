#include <napawesomiumservice.h>

namespace nap
{
	// Valuable component type registration
	void AwesomiumService::sRegisterTypes(nap::Core& inCore, const nap::Service& inService)
	{
		inCore.registerType(inService, RTTI_OF(BrowserComponent));
	}


	// Service
	AwesomiumService::AwesomiumService()
	{
		initCore(ofToDataPath("Logs"), ofToDataPath("SessionData"));
	}


	// Update
	void AwesomiumService::update()
	{
		// Update core
		mWebCore->Update();

		// Update all active browsing components
		std::vector<nap::BrowserComponent*> mBrowsers;
		getObjects<nap::BrowserComponent>(mBrowsers);
		for (auto& browser : mBrowsers)
		{
			if(!browser->mUpdate.getValue())
				continue;
			browser->Update();
		}
	}


	// Initialize the Awesomium Core Module
	void AwesomiumService::initCore(std::string logsPath, std::string sessionPath)
	{
		Awesomium::WebConfig config;
		config.log_path = Awesomium::WSLit(logsPath.c_str());
		config.log_level = Awesomium::kLogLevel_Verbose; //kLogLevel_Normal;

		// Set core
		mWebCore = Awesomium::WebCore::Initialize(config);

		// Create web preferences
		Awesomium::WebPreferences prefs;
		prefs.enable_plugins = true;
		prefs.enable_smooth_scrolling = true;
		prefs.enable_gpu_acceleration = true;
		prefs.enable_web_gl = true;

		// Create web session
		mSession = mWebCore->CreateWebSession(Awesomium::WSLit(sessionPath.c_str()), prefs);
	}


	// Converts an OF Web Address to a local address ("data/index.html" -> file:///c:/data/index.html"
	std::string AwesomiumService::sOFAddressToLocalAddress(const std::string& inValue)
	{
		ofFile local_file(inValue);
		if (!local_file.exists())
		{
			Logger::fatal("file does not exist: " + inValue);
			return "";
		}
		std::string abs_path = local_file.getAbsolutePath();
		return "file:///" + abs_path;
	}
}

RTTI_DEFINE(nap::AwesomiumService)