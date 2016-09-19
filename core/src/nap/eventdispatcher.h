#pragma once

// RTTI Includes
#include <rtti/rtti.h>

// Nap Includes
#include <nap/event.h>
#include <nap/attributeobject.h>
#include <nap/configure.h>
#include <nap/component.h>
#include <nap/eventdispatcher.h>


namespace nap
{
	// Direction of dispatching events
	enum class DispatchMethod : nap::int8
	{
		Upstream,
		Siblings,
		Downstream,
	};

	// Binds dispatch methods to a string value
	using DispatchMethodMap = std::unordered_map<DispatchMethod, std::string>;
	extern const DispatchMethodMap gDispatchMethods;

	// Dispatch method type converters
	bool convert_string_to_dispatchmethod(const std::string& inValue, DispatchMethod& outMethod);
	bool convert_dispatchmethod_to_string(const DispatchMethod& inMethod, std::string& outString);


	//////////////////////////////////////////////////////////////////////////


	class EventDispatcher : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		// Construction
		EventDispatcher() = default;
		EventDispatcher(Object* parent, const std::string& name);

		// Destruction
		virtual ~EventDispatcher() = default;

		// Event dispatching -> takes parentship of the event
		void dispatchEvent(Event& event, Object& source);

		// Event creation
		template<typename T, typename... Args>
		std::unique_ptr<T> createEvent(Args&&... args)
		{
			// Make sure it's an event
			RTTI::TypeInfo info = RTTI_OF(T);
			if (!info.isKindOf<Event>())
			{
				assert(false);
				Logger::warn("type: %s is not of type Event", info.getName().c_str());
				return nullptr;
			}

			// Create unique event
			std::unique_ptr<T> new_event = std::make_unique<T>(std::forward<Args>(args)...);

			// Populate it with data if necessary
			if (eventDelegate != nullptr)
				eventDelegate(*new_event.get());
			return new_event;
		}

		// Attributes
		Attribute<DispatchMethod> mDirection = { this, "direction", DispatchMethod::Downstream };

		// Delegate function used when creating the event
		using EventDelegate = std::function<void(Event&)>;
		EventDelegate eventDelegate = nullptr;
	};

}

namespace std
{
	template<>
	struct hash<nap::DispatchMethod> {
		size_t operator()(const nap::DispatchMethod &k) const {
			return hash<int>()((int) k);
		}
	};
}


RTTI_DECLARE(nap::EventDispatcher)
RTTI_DECLARE_DATA(nap::DispatchMethod)