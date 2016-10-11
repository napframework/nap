#include "xmlserializer.h"
#include "coremodule.h"
#include "object.h"

using namespace tinyxml2;
using namespace std;

#define X_ATTRIBUTE "attr"
#define X_VALUE "value"
#define X_NAME "name"
#define X_TYPE "type"
#define X_LINK "link"
#define X_VALUE_TYPE "vType"
#define X_OBJECT "obj"
#define X_PARENT "parent"

namespace nap
{


	XMLSerializer::XMLSerializer(std::ostream& os, Core& core) : Serializer(os, core) {}


	void XMLSerializer::writeObject(Object& object)
	{
		XMLDocument doc;
		auto root = doc.NewElement(object.getTypeInfo().getName().c_str());

        if (object.getParentObject())
            root->SetAttribute(X_PARENT, ObjectPath(object.getParentObject()).toString().c_str());

		doc.InsertEndChild(toXML(doc, object));
		XMLPrinter printer;
		doc.Print(&printer);
		stream << printer.CStr();
	}


	XMLNode* XMLSerializer::toXML(XMLDocument& doc, Object& object)
	{
		XMLElement* elm = nullptr;

		RTTI::TypeInfo type = object.getTypeInfo();

		if (type.isKindOf<AttributeBase>()) {
			elm = doc.NewElement(X_ATTRIBUTE);
			elm->SetAttribute(X_NAME, object.getName().c_str());

			AttributeBase& attrib = (AttributeBase&)object;
			const auto& valueType = attrib.getValueType();
			const auto converter = mCore.getModuleManager().getTypeConverter(valueType, RTTI_OF(string));
			if (converter) {
				Attribute<string> strAttr;
				converter->convert(&attrib, &strAttr);
				elm->SetAttribute(X_VALUE, strAttr.getValue().c_str());
			}
			elm->SetAttribute(X_VALUE_TYPE, attrib.getValueType().getName().c_str());

			if (attrib.isLinked()) {
				elm->SetAttribute(X_LINK, attrib.getLinkSource().toString().c_str());
			}

		} else {
			elm = doc.NewElement(X_OBJECT);
			elm->SetAttribute(X_NAME, object.getName().c_str());

			elm->SetAttribute(X_TYPE, type.getName().c_str());
		}


		for (Object* child : object.getChildren()) {
			elm->InsertEndChild(toXML(doc, *child));
		}

		return elm;
	}


	Object* XMLDeserializer::fromXML(XMLElement* xml, Object* parentObject)
	{
		Object* obj = nullptr;

		const char* tagName = xml->Name();

		if (!strcmp(tagName, X_ATTRIBUTE)) {
			obj = readAttribute(xml, parentObject);
		} else if (!strcmp(tagName, X_OBJECT)) {
			obj = readObject(xml, parentObject);
		} else {
			Logger::fatal("Unknown tag: '%s'", xml->Name());
		}

		if (!obj) return nullptr;

		// read children
		XMLElement* xChild = (XMLElement*)xml->FirstChild();
		while (xChild) {
			fromXML(xChild, obj);
			xChild = (XMLElement*)xChild->NextSibling();
		}

		return obj;
	}


	Object* XMLDeserializer::readAttribute(tinyxml2::XMLElement* xml, Object* parentObject)
	{
		// Resolve type
		std::string attributeType = xml->Attribute(X_VALUE_TYPE);

		// <HACK>
		// TODO: This filthy piece of shit is here because we cannot make Attributes of a certain RTTI value type
		attributeType = "nap::Attribute<" + attributeType + ">"; // <-- That...... :(
		const RTTI::TypeInfo& type = RTTI::TypeInfo::getByName(attributeType);
		// </HACK>

		if (type == RTTI::TypeInfo::empty()) {
			nap::Logger::fatal("Failed to retrieve type: %s", attributeType.c_str());
			return nullptr;
		}

		AttributeObject* parentAttrObject = dynamic_cast<AttributeObject*>(parentObject);
		assert(parentAttrObject);

		// Retrieve or create attribute
		std::string attrName(xml->Attribute(X_NAME));
		AttributeBase* attribute = nullptr;
		if (!parentAttrObject->hasAttribute(attrName)) {
			attribute = &parentAttrObject->addAttribute(attrName, type);
		} else {
			attribute = parentAttrObject->getAttribute(attrName);
		}
		assert(attribute);

		// Deserialize link
		const char* link = xml->Attribute(X_LINK);
		if (link) {
			attribute->linkPath(std::string(link));
		}

		// Retrieve and set value
		const TypeConverterBase* converter =
			mCore.getModuleManager().getTypeConverter(RTTI_OF(std::string), attribute->getValueType());
		if (!converter) {
			nap::Logger::fatal("Cannot convert");
			return attribute;
		}

		Attribute<std::string> strAttr;
		std::string valueStr = xml->Attribute(X_VALUE);
		strAttr.setValue(valueStr);
		if (!converter->convert(&strAttr, attribute)) {
			nap::Logger::fatal("Conversion failed from '%s' to '%s'", converter->inType().getName().c_str(),
							   converter->outType().getName().c_str());
		}

		return attribute;
	}

	nap::Object* XMLDeserializer::readObject(tinyxml2::XMLElement* xml, Object* parent)
	{
		const char* objectName = xml->Attribute(X_NAME);
		if (!objectName) {
			Logger::fatal("Object with no name encountered");
			return nullptr;
		}

		const char* objectTypeName = xml->Attribute(X_TYPE);
		if (!objectTypeName) {
			Logger::fatal("Object with no type encountered");
			return nullptr;
		}
		RTTI::TypeInfo objectType = RTTI::TypeInfo::getByName(objectTypeName);
		if (!objectType.isValid()) {
			Logger::fatal("Type not found: %s", objectTypeName);
			return nullptr;
		}

		if (!parent) {
			mCore.getRoot().setName(objectName);
			return &mCore.getRoot();
		}

		if (objectType.isKindOf<Entity>()) {
			assert(parent->getTypeInfo().isKindOf<Entity>());
			Entity* parentEntity = static_cast<Entity*>(parent);
			return &parentEntity->addEntity(objectName);
		}
		return &parent->addChild(objectName, objectType);
	}


	// Read the root elements and
	Object* XMLDeserializer::readObject(Object* parentObject)
	{
		std::istreambuf_iterator<char> eos;
		std::string s(std::istreambuf_iterator<char>(stream), eos);

		XMLDocument doc;
		XMLError err = doc.Parse(s.c_str(), s.size());
		if (err) {
			nap::Logger::fatal("Error parsing xml string");
			return nullptr;
		}

		XMLElement* elm = (XMLElement*)doc.FirstChild();
		if (!elm) {
			nap::Logger::fatal("XML string had no data");
			return nullptr;
		}

        if (parentObject) {
            std::string parent(elm->Attribute(X_PARENT));
            if (!parent.empty()) {
                parentObject = ObjectPath(parent).resolve(*parentObject);
            }
        }

		Object* result;
		while (elm) {
			result = fromXML(elm, parentObject);
			elm = (XMLElement*)elm->NextSibling();
		}
		return result;
	}
}