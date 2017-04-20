// Nap includes
#include <nap/core.h>

// local includes
#include <napinputservice.h>

namespace nap
{

	//////////////////////////////////////////////////////////////////////////
	// Filters
	//////////////////////////////////////////////////////////////////////////

	// Filters key input components
	static bool KeyFilter(Object& inComponent, Core& inCore)
	{
		return inComponent.get_type().is_derived_from(RTTI_OF(KeyInputComponent));
	}

	// Filters pointer input components
	static bool PointerFilter(Object& inComponent, Core& inCore)
	{
		return inComponent.get_type().is_derived_from(RTTI_OF(PointerInputComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// Registration
	//////////////////////////////////////////////////////////////////////////


	// All input components are automatically registered
	void InputService::sRegisterTypes(nap::Core& inCore, const nap::Service& inService)
	{
		inCore.registerType(inService, RTTI_OF(InputComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// Trigger
	//////////////////////////////////////////////////////////////////////////


	// Triggers all valid input enabled components based on the applied filters
	void InputService::trigger(UniqueEvent inEvent)
	{
		mDispatchMutex.lock();
		//std::cout << "trigger: " << inEvent->get_type().getName().c_str() << "\n";
		mInputEvents.emplace_back(std::move(inEvent));
		mDispatchMutex.unlock();
	}


	// Thread safe move of input events
	void InputService::consumeEvents()
	{
		// Lock for safe move
		mDispatchMutex.lock();

		// Clear existing dispatch events
		mDispatchEvents.clear();

		// Reserve space for faster move
		if (!mInputEvents.empty())
			mDispatchEvents.reserve(mInputEvents.size());

		// Move input events
		for (auto& event : mInputEvents)
			mDispatchEvents.emplace_back(std::move(event));

		// Clear input events
		mInputEvents.clear();

		// Unlock
		mDispatchMutex.unlock();
	}


	// Occurs when a component is removed and we have a lookup index
	void InputService::componentRemoved(nap::Object& object)
	{
		// Find element based on object memory location
		auto found_it = mInputMap.end();
		for (auto it = mInputMap.begin(); it != mInputMap.end(); it++)
		{
			if (it->second == &object)
			{
				found_it = it;
				break;
			}
		}

		// Erase element if found 
		if (found_it != mInputMap.end())
			mInputMap.erase(found_it);
	}


	// Dispatch events (called from main thread)
	void InputService::dispatch()
	{
		// Move events
		consumeEvents();

		// Ignore if input is disabled
		if (!mInputEnabled)
			return;

		for (auto& event : mDispatchEvents)
		{
			//std::cout << "process: " << event->get_type().getName().c_str() << "\n";

			// Add additional event information
			for (auto& filter : mInputFilters)
			{
				filter->filterEvent(*event);
			}

			// Do key lookup if a key event occured
			RTTI::TypeInfo event_info = event->get_type().getRawType();
			if (event_info.is_derived_from(RTTI_OF(KeyEvent)))
			{
				doLookup(*event, KeyFilter);
				continue;
			}

			// Make sure it's a pointer event
			assert(event_info.is_derived_from(RTTI_OF(PointerEvent)));

			// If it's a press, handle press lookup (caching component)
			if (event_info.is_derived_from(RTTI_OF(PointerPressEvent)))
			{
				PointerPressEvent* press_event = static_cast<PointerPressEvent*>(event.get());
				doPointerPressLookup(*press_event);
				pressed(*press_event);
				continue;
			}

			// Don't handle mouse moves
			PointerEvent* pointer_event = static_cast<PointerEvent*>(event.get());
			if (event_info.is_derived_from(RTTI_OF(PointerMoveEvent)))
			{
				PointerMoveEvent* move_event = static_cast<PointerMoveEvent*>(event.get());
				doPointerMoveLookup(*move_event);
				continue;
			}

			// Try to find corresponding input component when handling a drag or release
			auto it = mInputMap.find(pointer_event->mId.getValue());
			if (it != mInputMap.end())
			{
				(*it).second->trigger(*event);
				if (event->getHandled())
				{
					continue;
				}
			}
			else
			{
				nap::Logger::warn("Unable to find item for pointer event: %s", pointer_event->get_type().getName().c_str());
				continue;
			}

			// If the associated component did not accept the event, perform another lookup
			doLookup(*event, &PointerFilter);
		}

		// Clear all handled events (destroying them as we go)
		mDispatchEvents.clear();
	}


	// Dispatches a signal to all components in the system
	void InputService::doPointerMoveLookup(PointerMoveEvent& inEvent)
	{
		std::vector<InputComponent*> input_components, valid_components;
		getObjects<InputComponent>(valid_components, PointerFilter);

		for (auto& comp : valid_components)
		{
			comp->trigger(inEvent);
		}
	}


	// Apply a pointer press lookup
	// If the lookup is accepted, store the object and use for further move / touch operations
	void InputService::doPointerPressLookup(PointerPressEvent& inEvent)
	{
		// If the map already has an event associated with the event id, remove and disconnect
		auto it = mInputMap.find(inEvent.mId.getValue());
		if (it != mInputMap.end())
		{
			deactivated.trigger(*(it->second));
			it->second->removed.disconnect(mComponentRemoved);
			mInputMap.erase(inEvent.mId.getValue());
		}

		// Get input component for pointer lookup
		InputComponent* input_comp = doLookup(inEvent, &PointerFilter);

		// Don't do anything if a component wasn't found
		if (input_comp == nullptr)
			return;
		
		// Cache object
		assert(input_comp->get_type().getRawType().is_derived_from(RTTI_OF(PointerInputComponent)));

		// Connect for when removed
		input_comp->removed.connect(mComponentRemoved);

		// Add
		PointerInputComponent* new_pointer = static_cast<PointerInputComponent*>(input_comp);
		mInputMap.emplace(std::make_pair(inEvent.mId.getValue(), new_pointer));

		// Emit activated
		activated(*new_pointer);
	}


	// Performs general lookup using bound checks according to components found using inFilter
	InputComponent* InputService::doLookup(InputEvent& inEvent, ObjectFilterFunction inFilter)
	{
		// Get components based on type
		std::vector<InputComponent*> input_components, valid_components;
		getObjects<InputComponent>(valid_components, inFilter);

		// Perform recursive filtering
		for (auto& filter : mInputFilters)
		{
			// Don't continue if there are no valid input components
			if (valid_components.size() == 0)
				break;

			// Switch components
			input_components = valid_components;

			// Filter and swap later
			valid_components.clear();
			valid_components.reserve(input_components.size());
			filter->doFilter(inEvent, input_components, valid_components);
		}

		// Trigger all valid input components
		for (auto& input_component : valid_components)
		{
			if (!input_component->enabled.getValue())
				continue;

			// Trigger and stop when signal is handled
			input_component->trigger(inEvent);
			if (inEvent.getHandled())
			{
				return input_component;
			}
		}
		return nullptr;
	}

}

RTTI_DEFINE(nap::InputService)
RTTI_DEFINE(nap::InputFilter)