#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/attributeobject.h>
#include <nap/coreattributes.h>
#include <nap/event.h>

namespace nap
{
	/**
	@brief InputEvent

	Defines an input event that is passed along the system
	**/
	class InputEvent : public Event
	{
		RTTI_ENABLE_DERIVED_FROM(Event)
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief KeyEvent

	Key input event, tied to pressed and released
	**/
	class KeyEvent : public InputEvent
	{
		RTTI_ENABLE_DERIVED_FROM(InputEvent)
	public:
		KeyEvent(int inKey) : mKey(this, "Key", inKey)	{ }
		
		// Attribute
		Attribute<int>	mKey;
	};

	
	class KeyPressEvent : public KeyEvent
	{
		RTTI_ENABLE_DERIVED_FROM(KeyEvent)
	public:
		KeyPressEvent(int inKey) : KeyEvent(inKey) { }
	};

	
	class KeyReleaseEvent : public KeyEvent
	{
		RTTI_ENABLE_DERIVED_FROM(KeyEvent)
	public:
		KeyReleaseEvent(int inKey) :KeyEvent(inKey) { }
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief Pointer Events

	Contains all relevant information for pointer specific interaction
	Can also be used to signal multi touch gestures (therefore the id)
	**/
	class PointerEvent : public InputEvent
	{
		RTTI_ENABLE_DERIVED_FROM(InputEvent)
	public:
		PointerEvent(float inX, float inY, int inId = 0) :  
			mX(this, "xPos", inX), 
			mY(this, "yPos", inY),
			mId(this, "id", inId)		{ }

		Attribute<float>	mX;
		Attribute<float>	mY;
		Attribute<int>		mId;
	};

	
	class PointerClickEvent : public PointerEvent
	{
		RTTI_ENABLE_DERIVED_FROM(PointerEvent)
	public:
		PointerClickEvent(float inX, float inY, int inButton, int inId = 0) : 
			PointerEvent(inX, inY, inId), mButton(this, "button", inButton)	{ }

		Attribute<int> mButton;
	};

	
	class PointerPressEvent : public PointerClickEvent
	{
		RTTI_ENABLE_DERIVED_FROM(PointerClickEvent)
	public:
		PointerPressEvent(float inX, float inY, int inButton, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, inId) { }
	};

	
	class PointerReleaseEvent : public PointerClickEvent
	{
		RTTI_ENABLE_DERIVED_FROM(PointerClickEvent)
	public:
		PointerReleaseEvent (float inX, float inY, int inButton, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, inId) { }
	};


	class PointerDragEvent : public PointerClickEvent
	{
		RTTI_ENABLE_DERIVED_FROM(PointerClickEvent)
	public:
		PointerDragEvent (float inX, float inY, int inButton, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, inId)	{ }
	};


	class PointerMoveEvent : public PointerEvent
	{
		RTTI_ENABLE_DERIVED_FROM(PointerEvent)
	public:
		PointerMoveEvent(float inX, float inY, int inId = 0) : 
			PointerEvent(inX, inY, inId)	{ }
	};
}

// RTTI Declarations
RTTI_DECLARE_BASE(nap::InputEvent)
RTTI_DECLARE_BASE(nap::PointerEvent)

RTTI_DECLARE_BASE(nap::KeyEvent)
RTTI_DECLARE_BASE(nap::KeyPressEvent)
RTTI_DECLARE_BASE(nap::KeyReleaseEvent)

RTTI_DECLARE_BASE(nap::PointerClickEvent)
RTTI_DECLARE_BASE(nap::PointerPressEvent)
RTTI_DECLARE_BASE(nap::PointerReleaseEvent)
RTTI_DECLARE_BASE(nap::PointerDragEvent)
RTTI_DECLARE_BASE(nap::PointerMoveEvent)

