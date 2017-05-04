#include "rttipath.h"
#include "stringutils.h"
#include "object.h"

namespace RTTI
{
	const std::string RTTIPath::ToString() const
	{
		std::string result;
		for (int index = 0; index < mLength; ++index)
		{
			const RTTIPathElement& element = mElements[index];
			switch (element.mType)
			{
				case RTTIPathElement::Type::ATTRIBUTE:
				{
					if (result.empty())
						result += element.Attribute.Name;
					else
						result += nap::stringFormat(":%s", element.Attribute.Name.c_str());

					break;
				}

				case RTTIPathElement::Type::ARRAY_ELEMENT:
				{
					if (result.empty())
						result += nap::stringFormat("[%d]", element.ArrayElement.Index);
					else
						result += nap::stringFormat(":[%d]", element.ArrayElement.Index);
					break;
				}
			}
		}

		return result;
	}

	const RTTIPath RTTIPath::FromString(const std::string& path)
	{
		RTTIPath result;

		std::list<std::string> parts;
		nap::gTokenize(path, parts, ":", true);

		for (const std::string& part : parts)
		{
			int array_index = 0;
			if (sscanf(part.c_str(), "[%d]", &array_index) == 1)
			{
				result.PushArrayElement(array_index);
			}
			else
			{
				result.PushAttribute(part);
			}
		}

		return result;
	}

	ResolvedRTTIPath::ResolvedRTTIPath(nap::Object* object, const RTTIPath& path)
	{
		for (int index = 0; index < path.Length(); ++index)
		{
			const RTTIPathElement& element = path.Get(index);

			if (element.mType == RTTIPathElement::Type::ATTRIBUTE)
			{
				if (index == 0)
				{
					RTTI::Property property = object->get_type().get_property(element.Attribute.Name);
					if (!property.is_valid())
						return;

					PushRoot(object, property);
				}
				else
				{
					RTTI::Variant current_context = GetCurrentValue();
					if (!current_context.is_valid())
						return;

					RTTI::TypeInfo value_type = current_context.get_type();

					RTTI::Property property = value_type.get_property(element.Attribute.Name);
					if (!property.is_valid())
						return;

					PushAttribute(current_context, property);
				}
			}
			else if (element.mType == RTTIPathElement::Type::ARRAY_ELEMENT)
			{
				RTTI::Variant current_context = GetCurrentValue();
				if (!current_context.is_valid())
					return;

				if (!current_context.is_array())
					return;

				PushArrayElement(current_context, element.ArrayElement.Index);
			}
		}
	}

	const RTTI::Variant ResolvedRTTIPath::GetCurrentValue() const
	{
		if (IsEmpty())
			return RTTI::Variant();

		const ResolvedRTTIPathElement& last_element = mElements[mLength - 1];
		
		if (last_element.mType == ResolvedRTTIPathElement::Type::ROOT)
		{
			return last_element.Root.Property.get_value(last_element.Root.Instance);
		}
		else if (last_element.mType == ResolvedRTTIPathElement::Type::ATTRIBUTE)
		{
			return last_element.Attribute.Property.get_value(last_element.Attribute.Variant);
		}
		else if (last_element.mType == ResolvedRTTIPathElement::Type::ARRAY_ELEMENT)
		{
			RTTI::VariantArray array = last_element.ArrayElement.Array.create_array_view();
			return array.get_value(last_element.ArrayElement.Index);
		}

		return RTTI::Variant();
	}

	const RTTI::TypeInfo ResolvedRTTIPath::GetCurrentType() const
	{
		return GetCurrentValue().get_type();
	}

	bool ResolvedRTTIPath::SetValue(const RTTI::Variant& value)
	{
		if (IsEmpty())
			return false;

		RTTI::Variant value_to_set = value;
		for (int index = mLength - 1; index >= 0; --index)
		{
			const ResolvedRTTIPathElement& element = mElements[index];
			if (element.mType == ResolvedRTTIPathElement::Type::ROOT)
			{
				if (!element.Root.Property.set_value(element.Root.Instance, value_to_set))
					return false;
			}
			else if (element.mType == ResolvedRTTIPathElement::Type::ATTRIBUTE)
			{
				if (!element.Attribute.Property.set_value(element.Attribute.Variant, value_to_set))
					return false;
					
				value_to_set = element.Attribute.Variant;
			}
			else if (element.mType == ResolvedRTTIPathElement::Type::ARRAY_ELEMENT)
			{
				RTTI::VariantArray array = element.ArrayElement.Array.create_array_view();
				if (!array.set_value(element.ArrayElement.Index, value_to_set))
					return false;

				value_to_set = element.ArrayElement.Array;
			}
		}

		return true;
	}

	const ResolvedRTTIPath RTTIPath::Resolve(nap::Object* object) const
	{
		return ResolvedRTTIPath(object, *this);
	}
}