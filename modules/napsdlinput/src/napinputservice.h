#pragma once

// Rtti Inlcudes
#include <rtti/rtti.h>

// Nap Includes
#include <nap/service.h>
#include <nap/logger.h>

// Local Includes
#include <napinputevent.h>
#include <napinputcomponent.h>

// Nap Core
#include <nap/core.h>
#include <mutex>

namespace nap
{
	class InputService;

	// Defines
	using InputComponentList = std::vector<nap::InputComponent*>;

	/**
	@brief InputFilter
	Acts as a filter for finding valid input components
	**/
	class InputFilter
	{
		friend class InputService;
		RTTI_ENABLE()
	public:
		virtual ~InputFilter() = default;

	protected:
		nap::Core& getCore() { return *mCore; }

		// Only input-service is allowed to interact with input filters
		InputFilter() = default;
		virtual void doFilter(InputEvent& inEvent, InputComponentList& inComponents, InputComponentList& outComponents) = 0;
		virtual void filterEvent(nap::InputEvent& inEvent) {};

	private:
		nap::Core* mCore = nullptr;
		void setCore(nap::Core& inCore) { mCore = &inCore; }
	};


	/**
	@brief nap input service
	Forwards input messages to registered input components
	**/
	class InputService : public Service
	{
		// RTTI
		RTTI_ENABLE_DERIVED_FROM(Service)

		// Declare Service
		NAP_DECLARE_SERVICE()

	public:
		// Default constructor
		InputService() = default;

		// Disable copy
		InputService(const InputService& that) = delete;
		InputService& operator=(const InputService&) = delete;

		// Naming
		using UniqueEvent = std::unique_ptr<InputEvent>;

		// Event -> takes ownership of the event and dispatches it on update
		void trigger(UniqueEvent inEvent);

		// Dispatches events
		void dispatch();

		// Filter
		using InputFilterList = std::vector<std::unique_ptr<InputFilter>>;

		template <typename T>
		T& addFilter();

		template <typename T>
		T* getFilter();

		void setInputEnabled(bool enabled) { mInputEnabled = enabled; }
		bool isInputEnabled() const { return mInputEnabled; }

		using PointerInputEventMap = std::unordered_map<int, PointerInputComponent*>;
		const PointerInputEventMap& getActiveComponents() { return mInputMap; }

		// Signals emitted when a component is activated / deactivated
		nap::Signal<PointerInputComponent&> activated;
		nap::Signal<PointerInputComponent&> deactivated;
		nap::Signal<const PointerPressEvent&> pressed;

	private:
		bool mInputEnabled = true;
		// All filters
		InputFilterList	mInputFilters;

		// Event caching
		// When a new press event comes through, add it to the map based on it's id
		// If subsequent move, drag or release events happen, use the input component as base
		// Prevents a lot of scene lookups when mapping cached actions to new events
		PointerInputEventMap mInputMap;

		// Performs a new lookup
		void doPointerPressLookup(PointerPressEvent& inEvent);
		void doPointerMoveLookup(PointerMoveEvent& inEvent);
		InputComponent* doLookup(InputEvent&, ObjectFilterFunction inFilter);

		// Mutex for thread safe input handling
		std::mutex mDispatchMutex;
		std::vector<UniqueEvent> mInputEvents;			//< Thread safe map of input events
		std::vector<UniqueEvent> mDispatchEvents;		//< Moved input events to dispatch

		void consumeEvents();							//< Thread safe move event operation
		
		// Slot
		NSLOT(mComponentRemoved, nap::Object&, componentRemoved)
		void componentRemoved(nap::Object& object);		//< When an object is removed -> remove id from map
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	T& InputService::addFilter()
	{
		// Get type
		RTTI::TypeInfo type = RTTI_OF(T);
		assert(type.isKindOf<InputFilter>());

		// Check if don't have one already
		auto it = std::find_if(mInputFilters.begin(), mInputFilters.end(), [&](auto& filter)
		{ return filter->getTypeInfo() == type; });

		// If yes, return that one
		if (it != mInputFilters.end())
		{
			nap::Logger::warn("filter with type: %s already loaded", type.getName().c_str());
			return *(static_cast<T*>((*it).get()));
		}

		// Make new one
		std::unique_ptr<T> obj = std::make_unique<T>();

		// Set core
		obj->setCore(this->getCore());
		
		// Return
		T* return_filter = obj.get();
		mInputFilters.emplace_back(std::move(obj));
		return *return_filter;
	}


	// Returns filter of given type
	template <typename T>
	T* InputService::getFilter()
	{
		for (auto& filter : mInputFilters)
		{
			if (filter->getTypeInfo().isKindOf(RTTI_OF(T)))
				return static_cast<T*>(filter.get());
		}
		return nullptr;
	}
}

RTTI_DECLARE(nap::InputService)
RTTI_DECLARE_BASE(nap::InputFilter)