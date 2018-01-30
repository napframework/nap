#pragma once

#include <rtti/rttipath.h>
#include <QMetaType>

namespace napkin
{
	/**
	 * A path to a property, including its object.
	 * This class carries both the object and the property path.
	 */
	class PropertyPath
	{
	public:
		PropertyPath() {}

		/**
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::RTTIObject& obj, const nap::rtti::RTTIPath& path);

		/**
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::RTTIObject& obj, const std::string& path);

		/**
		 * @return The value of this property
		 */
		rttr::variant getValue() const;

		/**
		 * Set the value of this property
		 */
		void setValue(rttr::variant value);

		/**
		 * @return The property this path points to
		 */
		rttr::property getProperty() const;

		/**
		 * @return The type of the property
		 */
		rttr::type getType() const;

		/**
		 * @return A child path of this propertypath
		 */
		PropertyPath getChild(const std::string& name) const;

		/**
		 * @return obj The object this property is on
		 */
		nap::rtti::RTTIObject& object() const { return *mObject; }

		/**
		 * @return path The path to the property
		 */
		const nap::rtti::RTTIPath& path() const { return mPath; }

		/**
		 * Resolve a property path
		 */
		nap::rtti::ResolvedRTTIPath resolve() const;

		/**
		 * If this path refers to an array property, get the element type
		 * If this path does not refer to an array property, return type::empty()
		 */
		rttr::type getArrayElementType();

		/**
		 * If this path refers to an array property, return the length of the array.
		 */
		size_t getArrayLength()const;

		/**
		 * @return Wrapped type
		 */
		rttr::type getWrappedType() const;

		/**
		 * @return A string representation of this path
		 */
		std::string toString() const;

		/**
		 * @return If the path is a valid one
		 */
		bool isValid() const;

	private:
		/**
		 * If this property is an array, get its arrayview
		 */
		rttr::variant_array_view getArrayView();



		nap::rtti::RTTIObject* mObject = nullptr;
		nap::rtti::RTTIPath mPath;
		// Temps
		nap::rtti::ResolvedRTTIPath mResolvedPath;
		nap::rtti::Variant mVariant;
		nap::rtti::VariantArray mVariantArray;
	};
}

Q_DECLARE_METATYPE(napkin::PropertyPath)