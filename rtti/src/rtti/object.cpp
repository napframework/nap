#include "object.h"
#include "objectptr.h"



RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::rtti::Object)
	RTTI_PROPERTY(nap::rtti::sIDPropertyName, &nap::rtti::Object::mID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace rtti
	{
		bool Object::isIDProperty(rtti::Instance& object, const rtti::Property& property)
		{
			return object.get_derived_type().is_derived_from<Object>() && std::string(property.get_name().data()) == sIDPropertyName;
		}


		void Object::setEnableObjectPtrs(bool enable)
		{
			mEnableObjectPtrs = enable;
		}


		// Note: Even though the RTTIObject constructor is empty, we have to keep it in the CPP. 
		// This is because otherwise this CPP is empty, causing the RTTI registration code above to be optimized away, causing the ID property to not be registered in RTTI.
		Object::Object()
		{
		}

		Object::~Object()
		{
			// During deserialization, there is an option to use only raw pointers instead of ObjectPtrs (EPointerPropertyMode). When using only raw pointers, the ObjectPtrManager
			// is never used for these objects. The main reason why we disable this behaviour is for scenarios where we are deserializing on multiple threads. The ObjectPtrManager 
			// is not thread-safe and we prefer not to add locks to each ObjectPtr access. In such scenarios, we don't need the ObjectPtr functionality that enable us to hotload. 
			// The deserializer disables any ObjectPtrManager access. It is therefore also forbidden to use ObjectPtrs to point to objects that were deserialized
			// with 'OnlyRawPointers' enabled (the ObjectPtrs will never be reset to nullptr when the Object goes out of scope).
			if (mEnableObjectPtrs)
				ObjectPtrManager::get().resetPointers(*this);
		}
	}
}
