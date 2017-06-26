#pragma once

// External Includes
#include <rtti/rtti.h>

namespace nap
{
	/**
	@brief InputEvent

	Defines an input event that is passed along the system
	**/
	class InputEvent
	{
		RTTI_ENABLE()
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief KeyEvent

	Key input event, tied to pressed and released
	**/
	class KeyEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		KeyEvent(int inKey) :
			mKey(inKey)	
		{
		}
		
		int	mKey;
	};

	
	class KeyPressEvent : public KeyEvent
	{
		RTTI_ENABLE(KeyEvent)
	public:
		KeyPressEvent(int inKey) : 
			KeyEvent(inKey) 
		{ 
		}
	};

	
	class KeyReleaseEvent : public KeyEvent
	{
		RTTI_ENABLE(KeyEvent)
	public:
		KeyReleaseEvent(int inKey) :
			KeyEvent(inKey) 
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	@brief Pointer Events

	Contains all relevant information for pointer specific interaction
	Can also be used to signal multi touch gestures (therefore the id)
	**/
	class PointerEvent : public InputEvent
	{
		RTTI_ENABLE(InputEvent)
	public:
		PointerEvent(float inX, float inY, int inId = 0) :  
			mX(inX), 
			mY(inY),
			mId(inId)		
		{ 
		}

		float	mX;
		float	mY;
		int		mId;
	};

	
	class PointerClickEvent : public PointerEvent
	{
		RTTI_ENABLE(PointerEvent)
	public:
		PointerClickEvent(float inX, float inY, int inButton, int inId = 0) : 
			PointerEvent(inX, inY, inId), 
			mButton(inButton)	
		{
		}

		int mButton;
	};

	
	class PointerPressEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerPressEvent(float inX, float inY, int inButton, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, inId)
		{
		}
	};

	
	class PointerReleaseEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerReleaseEvent (float inX, float inY, int inButton, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, inId)
		{
		}
	};


	class PointerDragEvent : public PointerClickEvent
	{
		RTTI_ENABLE(PointerClickEvent)
	public:
		PointerDragEvent (float inX, float inY, int inButton, int inId = 0) : 
			PointerClickEvent(inX, inY, inButton, inId)
		{
		}
	};


	class PointerMoveEvent : public PointerEvent
	{
		RTTI_ENABLE(PointerEvent)
	public:
		PointerMoveEvent(float inX, float inY, int inId = 0) : 
			PointerEvent(inX, inY, inId)
		{
		}
	};
}
