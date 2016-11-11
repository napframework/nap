#include "scriptservercomponent.h"

RTTI_DEFINE(nap::ScriptServerComponent)



namespace nap
{

	ScriptServerComponent::ScriptServerComponent()
	{
		running.valueChangedSignal.connect([&](const bool& running) {  onRunningChanged(running); });
	}


	void ScriptServerComponent::onRunningChanged(const bool& running)
	{
		assert(mServerThread == nullptr);
		if (running) {
			mServerThread = new std::thread(&ScriptServerComponent::run, this);
		}
	}

	Object* ScriptServerComponent::resolvePath(const std::string& path)
	{
		Object* obj = ObjectPath(path).resolve(*getRootObject());
		if (!obj) {
			Logger::warn("Failed to resolve path '%s'", path.c_str());
			return nullptr;
		}
		return obj;
	}


}