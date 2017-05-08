#pragma once

#include <string>
#include <assert.h>
#include <rtti/typeinfo.h>

namespace nap
{
	class Object;
}

namespace RTTI
{
	class RTTIPathElement
	{
	public:
		enum class Type
		{
			INVALID,
			ATTRIBUTE,
			ARRAY_ELEMENT
		};

		RTTIPathElement() {}
		RTTIPathElement(const std::string& attributeName) : mType(Type::ATTRIBUTE)		{ InitNonPOD(Attribute.Name, attributeName); }
		RTTIPathElement(int arrayIndex) : mType(Type::ARRAY_ELEMENT)					{ ArrayElement.Index = arrayIndex; }
		RTTIPathElement(const RTTIPathElement& other)									{ Copy(other); }

		~RTTIPathElement()																{ Destroy(); }

		RTTIPathElement& operator=(const RTTIPathElement& other)
		{
			if (&other == this)
				return *this;

			Destroy();
			Copy(other);

			return *this;
		}

		bool operator==(const RTTIPathElement& lhs) const
		{
			if (mType != lhs.mType)
				return false;

			if (mType == Type::ATTRIBUTE)
				return Attribute.Name == lhs.Attribute.Name;
			else if (mType == Type::ARRAY_ELEMENT)
				return ArrayElement.Index == lhs.ArrayElement.Index;

			assert(false);
			return false;
		}

		bool operator!=(const RTTIPathElement& lhs) const
		{
			return !(*this == lhs);
		}

		Type mType = Type::INVALID;
		union 
		{
			struct 
			{
				std::string Name;
			} Attribute;

			struct 
			{
				int Index;
			} ArrayElement;
		};

	private:
		template <typename T>
		inline void InitNonPOD(T& member, const T& val)
		{
			new (&member) T(val);
		}

		inline void Destroy()
		{
			if (mType == Type::ATTRIBUTE)
			{
				using std::string;
				Attribute.Name.~string();
			}
		}

		inline void Copy(const RTTIPathElement& other)
		{
			mType = other.mType;
			if (mType == Type::ATTRIBUTE)
			{
				InitNonPOD(Attribute.Name, other.Attribute.Name);
			}
			else if (mType == Type::ARRAY_ELEMENT)
			{
				ArrayElement.Index = other.ArrayElement.Index;
			}
		}
	};

	class ResolvedRTTIPathElement
	{
	public:
		enum class Type
		{
			INVALID,
			ROOT,
			ATTRIBUTE,
			ARRAY_ELEMENT
		};

		ResolvedRTTIPathElement() :
			mType(Type::INVALID)
		{
		}

		ResolvedRTTIPathElement(const RTTI::Instance& instance, const RTTI::Property& property) :
			mType(Type::ROOT)
		{
			InitNonPOD(Root.Instance, instance);
			InitNonPOD(Root.Property, property);
		}

		ResolvedRTTIPathElement(const RTTI::Variant& variant, const RTTI::Property& property) :
			mType(Type::ATTRIBUTE)
		{
			InitNonPOD(Attribute.Variant, variant);
			InitNonPOD(Attribute.Property, property);
		}

		ResolvedRTTIPathElement(const RTTI::Variant& array, int index) :
			mType(Type::ARRAY_ELEMENT)
		{
			InitNonPOD(ArrayElement.Array, array);
			ArrayElement.Index = index;
		}

		ResolvedRTTIPathElement(const ResolvedRTTIPathElement& other)
		{
			Copy(other);
		}

		~ResolvedRTTIPathElement()
		{
			Destroy();
		}

		ResolvedRTTIPathElement& operator=(const ResolvedRTTIPathElement& other)
		{
			if (&other == this)
				return *this;

			Destroy();
			Copy(other);

			return *this;
		}

		Type mType;

	private:
		friend class ResolvedRTTIPath;
		union 
		{
			struct 
			{
				RTTI::Instance Instance;
				RTTI::Property Property;
			} Root;

			struct 
			{
				RTTI::Variant Variant;
				RTTI::Property Property;
			} Attribute;

			struct 
			{
				RTTI::Variant Array;
				int Index;
			} ArrayElement;
		};

		template <typename T>
		inline void InitNonPOD(T& member, const T& val)
		{
			new (&member) T(val);
		}

		inline void Destroy()
		{
			if (mType == Type::ROOT)
			{
				Root.Instance.~instance();
				Root.Property.~property();
			}
			else if (mType == Type::ATTRIBUTE)
			{
				Attribute.Variant.~variant();
				Attribute.Property.~property();
			}
			else if (mType == Type::ARRAY_ELEMENT)
			{
				ArrayElement.Array.~variant();
			}
		}

		inline void Copy(const ResolvedRTTIPathElement& other)
		{
			mType = other.mType;
			if (mType == Type::ROOT)
			{
				InitNonPOD(Root.Instance, other.Root.Instance);
				InitNonPOD(Root.Property, other.Root.Property);
			}
			else if (mType == Type::ATTRIBUTE)
			{
				InitNonPOD(Attribute.Variant, other.Attribute.Variant);
				InitNonPOD(Attribute.Property, other.Attribute.Property);
			}
			else if (mType == Type::ARRAY_ELEMENT)
			{
				InitNonPOD(ArrayElement.Array, other.ArrayElement.Array);
				ArrayElement.Index = other.ArrayElement.Index;
			}
		}
	};

	class RTTIPath
	{
	public:
		inline void PushAttribute(const std::string& attributeName)	{ assert(mLength < RTTIPATH_MAX_LENGTH); mElements[mLength++] = RTTIPathElement(attributeName); }
		inline void PushArrayElement(int index)						{ assert(mLength < RTTIPATH_MAX_LENGTH); mElements[mLength++] = RTTIPathElement(index); }
		inline void PopBack()										{ assert(mLength > 0); mElements[--mLength] = RTTIPathElement(); }

		const std::string ToString() const;
		static const RTTIPath FromString(const std::string& path);

		const ResolvedRTTIPath Resolve(nap::Object* object) const;

		int Length() const
		{
			return mLength;
		}

		const RTTIPathElement& Get(int index) const
		{
			assert(index < mLength);
			return mElements[index];
		}

		bool operator==(const RTTIPath& lhs) const
		{
			if (mLength != lhs.mLength)
				return false;

			for (int index = 0; index < mLength; ++index)
				if (mElements[index] != lhs.mElements[index])
					return false;

			return true;
		}

		bool operator!=(const RTTIPath& lhs) const
		{
			return !(*this == lhs);
		}

	private:
		static const int	RTTIPATH_MAX_LENGTH = 16;
		RTTIPathElement		mElements[RTTIPATH_MAX_LENGTH];
		int					mLength = 0;
	};

	class ResolvedRTTIPath
	{
	public:
		ResolvedRTTIPath(nap::Object* object, const RTTIPath& path);

		const RTTI::Variant GetValue() const;
		const RTTI::TypeInfo GetType() const;
		bool SetValue(const RTTI::Variant& value);

		inline void	PushRoot(const RTTI::Instance& instance, const RTTI::Property& property)		{ assert(mLength < RTTIPATH_MAX_LENGTH); mElements[mLength++] = ResolvedRTTIPathElement(instance, property); }
		inline void	PushAttribute(const RTTI::Variant& variant, const RTTI::Property& property)		{ assert(mLength < RTTIPATH_MAX_LENGTH); mElements[mLength++] = ResolvedRTTIPathElement(variant, property); }
		inline void	PushArrayElement(const RTTI::Variant& array, int index)							{ assert(mLength < RTTIPATH_MAX_LENGTH); mElements[mLength++] = ResolvedRTTIPathElement(array, index); }

		bool IsEmpty() const { return mLength == 0; }
		bool IsValid() const { return !IsEmpty(); }

	private:
		static const int		RTTIPATH_MAX_LENGTH = 16;
		ResolvedRTTIPathElement	mElements[RTTIPATH_MAX_LENGTH];
		int						mLength = 0;
	};
}
