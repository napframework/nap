#pragma once

#include <nap.h>
#include <thread>


namespace nap
{

	/**
	 *  This component will run a listening server receiving script calls and dispatch notifications to any remote
	 * listeners.
	 */
	class ScriptServerComponent : public Component
	{
    protected:
		/**
		* Holds a slot that may receive signals and forward them to the script server
		*/
		class RPCCallBack
		{
		public:
			RPCCallBack(ScriptServerComponent& server) : mServer(server) {}
			Slot<AttributeBase&> slot;

		private:
			void trigger(AttributeBase& attrib) { mServer.handleAttributeChanged(attrib); }
			ScriptServerComponent& mServer;
		};


		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		ScriptServerComponent();

		virtual void run() = 0;

		Attribute<int> rpcPort = {this, "rpcPort", 8888};
        Attribute<int> pubPort = {this, "pubPort", 8889};
		Attribute<bool> running = {this, "running", false};

		void addAttributeChangedCallback(const std::string& objPath) {
            auto attrib = resolve<AttributeBase>(objPath);
            if (!attrib)
                return;

            auto callback = std::make_unique<RPCCallBack>(*this);
            attrib->valueChanged.connect(callback->slot);
            mAttributeCallbacks.emplace(objPath, std::move(callback));
        }

        void removeAttributeChangedCallback(const std::string& path) {
            auto callbackIt = mAttributeCallbacks.find(path);
            if (callbackIt == mAttributeCallbacks.end())
                return;
            mAttributeCallbacks.erase(callbackIt);
        }


	protected:
		virtual std::string evalScript(const std::string& cmd) = 0;
		virtual void handleAttributeChanged(AttributeBase& attrib) = 0;

		Core& getCore() { return static_cast<Entity*>(getRootObject())->getCore(); }

		Object* resolvePath(const std::string& path);

		template <typename T>
		T* resolve(const std::string& path)
		{
			auto obj = resolvePath(path);
			if (!obj)
				return nullptr;

			if (!obj->getTypeInfo().isKindOf<T>()) {
				Logger::warn("Object is not of expected type '%s': '%s'", RTTI_OF(T).getName().c_str(), path.c_str());
				return nullptr;
			}
			return static_cast<T*>(obj);
		}

	protected:
		std::thread* mServerThread = nullptr;

	private:
		void onRunningChanged(const bool& running);

	private:
		std::map<std::string, std::unique_ptr<RPCCallBack>> mAttributeCallbacks;
	};
}
RTTI_DECLARE_BASE(nap::ScriptServerComponent)
