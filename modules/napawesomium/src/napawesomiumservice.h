#pragma once
// Nap Includes
#include <nap/service.h>
#include <nap/coremodule.h>
#include <napbrowsercomponent.h>

// RTTI Includes
#include <rtti/rtti.h>

// Awesomium Includes
#include <Awesomium/WebCore.h>

namespace nap
{
	/**
	@brief nap awesomiumservice

	Returns all registered web-kit related components
	Manages the Awesomium core, note that only 1 Awesomiumservice can be used at once
	Multiple services of this type will result in unexpected behavior
	**/
	class AwesomiumService : public Service
	{
		// Derived
		RTTI_ENABLE_DERIVED_FROM(Service)

		// Declare the service
		NAP_DECLARE_SERVICE()

	public:
		// Constructor
		AwesomiumService();
		virtual ~AwesomiumService() { mWebCore->Shutdown(); }

		// Update
		void update();

		// Getters
		Awesomium::WebCore&			getCore() { return *mWebCore; }

		// Converts an OF Web Address to a local address ("data/index.html" -> file:///c:/data/index.html"
		static std::string			sOFAddressToLocalAddress(const std::string& inValue);

	private:
		// Awesomium Core
		Awesomium::WebCore*			mWebCore = nullptr;
		Awesomium::WebSession*		mSession = nullptr;
		void initCore(std::string	logsPath, std::string sessionPath);
	};
}

RTTI_DECLARE(nap::AwesomiumService)