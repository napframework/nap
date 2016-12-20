// attributeobject.h template definitions

namespace nap {

	template<typename T>
	Attribute <T> *AttributeObject::getOrCreateAttribute(const std::string &name) {
		auto attribute = getAttribute<T>(name);

		if (!attribute) {
			if (hasAttribute(name)) // Attribute of wrong type?
				return nullptr;

			auto type = RTTI_OF(Attribute < T > );

			attribute = static_cast<Attribute <T> *>(&addChild(name, type));
		}
		return attribute;
	}


	template<typename T>
	Attribute <T> *AttributeObject::getOrCreateAttribute(const std::string &name, const T &defaultValue) {
		bool valueExisted = hasAttribute(name);
		auto attribute = getOrCreateAttribute < T > (name);
		if (!valueExisted) attribute->setValue(defaultValue);
		return attribute;
	}


	template<typename T>
	Attribute <T> *AttributeObject::getAttribute(const std::string &name) {
		Attribute <T> *attribute = dynamic_cast<Attribute <T> *>(getAttribute(name));
		if (attribute) return attribute;
		return nullptr;
	}


	template<typename T>
	Attribute <T> &AttributeObject::addAttribute(const std::string &name, const T &defaultValue) {
		Attribute <T> &attribute = addAttribute < T > (name);
		attribute.setValue(defaultValue);
		return attribute;
	}

}