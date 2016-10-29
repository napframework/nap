#include "jsonserializer.h"


#include <nap.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>

using namespace rapidjson;

#define J_NAME "name"
#define J_VALUE "value"
#define J_TYPE "type"
#define J_OBJECT "obj"
#define J_ATTR "attr"
#define J_VALUE_TYPE "vType"

namespace nap
{

	template <typename T>
	void writeAttributeValue(T& writer, AttributeBase& attrib)
	{
		const Entity* root = dynamic_cast<const Entity*>(attrib.getRootObject());
		if (!root)
			return;

		const TypeConverterBase* converter = root->getCore().getModuleManager().getTypeConverter(
			attrib.getValueType(), RTTI::TypeInfo::get<std::string>());
		if (!converter)
			return;

		Attribute<std::string> stringAttr;
		if (converter->convert(&attrib, &stringAttr)) {
			writer.String(J_VALUE);
			writer.String(stringAttr.getValue().c_str());
		}
	}

	template <typename T>
	void writeTheObject(T& writer, Object& obj)
	{
		bool isAttrib = obj.getTypeInfo().isKindOf<AttributeBase>() && !obj.getTypeInfo().isKindOf<CompoundAttribute>();
		writer.String(isAttrib ? J_ATTR : J_OBJECT);
		writer.StartObject();
		writer.String(J_NAME);
		writer.String(obj.getName().c_str());
		if (isAttrib) {
			AttributeBase& attrib = *static_cast<AttributeBase*>(&obj);
			writer.String(J_VALUE_TYPE);
			writer.String(attrib.getValueType().getName().c_str());
			writeAttributeValue(writer, attrib);
		} else {
			writer.String(J_TYPE);
			writer.String(obj.getTypeInfo().getName().c_str());
		}

		for (Object* child : obj.getChildren())
			writeTheObject(writer, *child);

		writer.EndObject();
	}

	void JSONSerializer::writeObject(std::ostream& ostream, Object& object) const
	{

		StringBuffer buf;
		PrettyWriter<StringBuffer> writer(buf);

		writer.StartObject();
		writeTheObject(writer, object);
		writer.EndObject();

		ostream << buf.GetString();
	}

	template <typename T, typename M>
	AttributeBase* jsonToAttribute(const GenericValue<T, M>& value, Core& core, Object* parent)
	{
		const char* attrName = value.FindMember(J_NAME)->value.GetString();
		const char* attrValueTypeName = value.FindMember(J_VALUE_TYPE)->value.GetString();

		assert(parent->getTypeInfo().isKindOf<AttributeObject>() || parent->getTypeInfo().isKindOf<CompoundAttribute>());
		AttributeObject* attributeObject = static_cast<AttributeObject*>(parent);

		AttributeBase* attrib = attributeObject->getAttribute(attrName);
		if (!attrib) {
			RTTI::TypeInfo attrType = RTTI::TypeInfo::getByName(dirtyHack(attrValueTypeName));
			attrib = &attributeObject->addAttribute(attrName, attrType);
		}
		assert(!strcmp(attrValueTypeName, attrib->getValueType().getName().c_str()));


		const TypeConverterBase* converter =
			core.getModuleManager().getTypeConverter(RTTI_OF(std::string), attrib->getValueType());
		if (!converter) {
			nap::Logger::fatal("Cannot convert to %s", attrib->getValueType().getName().c_str());
			return attrib;
		}

		Attribute<std::string> strAttr;
		std::string valueStr = value.FindMember(J_VALUE)->value.GetString();
		strAttr.setValue(valueStr);
		if (!converter->convert(&strAttr, attrib)) {
			nap::Logger::fatal("Conversion failed from '%s' to '%s'", converter->inType().getName().c_str(),
							   converter->outType().getName().c_str());
		}

		return attrib;
	};

	template <typename T, typename M>
	Object* jsonToObject(const GenericValue<T, M>& value, Core& core, Object* parent)
	{
		const char* objectName = value.FindMember(J_NAME)->value.GetString();
		const char* objectTypename = value.FindMember(J_TYPE)->value.GetString();

		Object* obj = nullptr;

		if (!parent) {
			// Reading root object
			core.getRoot().setName(objectName);
			obj = &core.getRoot();
		} else {
			RTTI::TypeInfo objectType = RTTI::TypeInfo::getByName(objectTypename);

			// Handle Entity
			if (objectType.isKindOf<Entity>()) {
				assert(parent->getTypeInfo().isKindOf<Entity>());
				Entity* parentEntity = static_cast<Entity*>(parent);
				obj = &parentEntity->addEntity(objectName);
			} else {
				// Handle other types
				if (parent->hasChild(objectName) && parent->getChild(objectName)->getTypeInfo().isKindOf(objectType)) {
					// Child alread exists
					obj = parent->getChild(objectName);
				} else {
					obj = &parent->addChild(objectName, objectType);
				}
			}
		}

		// Process children
		for (auto iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter) {
			const char* jObjectName = iter->name.GetString();
			if (!strcmp(jObjectName, J_OBJECT)) {
				jsonToObject(iter->value, core, obj);
			} else if (!strcmp(jObjectName, J_ATTR)) {
				jsonToAttribute(iter->value, core, obj);
			} else {
				//                assert(false); // Unknown object type
			}
		}

		if (obj->getTypeInfo().isKindOf<AttributeBase>() && !obj->getTypeInfo().isKindOf<CompoundAttribute>()) {
			Logger::info("Attribute");
		}

		return obj;
	};

	Object* JSONDeserializer::readObject(std::istream& istream, Core& core, Object* parent) const
	{
		IStreamWrapper is(istream);

		Document doc;
		doc.ParseStream(is);
		assert(doc.IsObject()); // JSON may only contain one root object

		Object* result = nullptr;
		for (auto iter = doc.MemberBegin(); iter != doc.MemberEnd(); ++iter) {
			assert(!result); // Assume only one root object

			const char* jObjectName = iter->name.GetString();
			assert(!strcmp(jObjectName, J_OBJECT));
			result = jsonToObject(iter->value, core, parent);
		}


		return nullptr;
	}
}