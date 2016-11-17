// Local Includes
#include "xmlserializer.h"
#include "coreattributes.h"

// External includes
#include <nap/entity.h>

using namespace tinyxml2;
using namespace std;

#define X_ATTRIBUTE "attr"
#define X_COMP_ATTRIB "compattr"
#define X_VALUE "value"
#define X_NAME "name"
#define X_TYPE "type"
#define X_LINK "link"
#define X_VALUE_TYPE "vType"
#define X_OBJECT "obj"
#define X_PARENT "parent"

namespace nap
{
	void XMLSerializer::writeObject(std::ostream& ostream, Object& object, bool writePointers) const
	{
		XMLDocument doc;


		XMLElement* root = toXML(doc, object);

		if (object.getParentObject()) {
			root->SetAttribute(X_PARENT, ObjectPath(object.getParentObject()).toString().c_str());
		}

		doc.InsertEndChild(root);
		XMLPrinter printer;
		doc.Print(&printer);
		ostream << printer.CStr();
	}


	XMLElement* XMLSerializer::toXML(XMLDocument& doc, Object& object) const
	{
		XMLElement* elm = nullptr;

		RTTI::TypeInfo type = object.getTypeInfo();

		if (type.isKindOf<CompoundAttribute>()) {
			elm = doc.NewElement(X_COMP_ATTRIB);
			elm->SetAttribute(X_NAME, object.getName().c_str());
			CompoundAttribute& attrib = (CompoundAttribute&)object;

			for (auto childAttrib : attrib.getAttributes()) {
				elm->InsertEndChild(toXML(doc, *childAttrib));
			}

		} else if (type.isKindOf<AttributeBase>()) {
			elm = doc.NewElement(X_ATTRIBUTE);
			elm->SetAttribute(X_NAME, object.getName().c_str());

			AttributeBase& attrib = (AttributeBase&)object;
            Entity* root = (Entity*) object.getRootObject();
            assert(root);

            const auto& valueType = attrib.getValueType();
			const auto converter =
				root->getCore().getModuleManager().getTypeConverter(valueType, RTTI_OF(string));
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


	Object* XMLDeserializer::fromXML(tinyxml2::XMLElement* xml, Core& core, Object* parentObject) const
	{
		Object* obj = nullptr;

		const char* tagName = xml->Name();

        const char* name = xml->Attribute(X_NAME);

		if (!strcmp(tagName, X_ATTRIBUTE)) {
			obj = readAttribute(xml, core, parentObject);
		} else if (!strcmp(tagName, X_OBJECT)) {
			obj = readObject(xml, core, parentObject);
		} else if (!strcmp(tagName, X_COMP_ATTRIB)) {
			obj = readCompoundAttribute(xml, core, parentObject);
		} else {
			Logger::fatal("Unknown tag: '%s'", xml->Name());
		}

		if (!obj)
			return nullptr;

		// read children
		XMLElement* xChild = (XMLElement*)xml->FirstChild();
		while (xChild) {
            fromXML(xChild, core, obj);
			xChild = (XMLElement*)xChild->NextSibling();
		}

		return obj;
	}

	Object* XMLDeserializer::readCompoundAttribute(tinyxml2::XMLElement* xml, Core& core, Object* parent) const
	{
        std::string name = xml->Attribute(X_NAME);
        CompoundAttribute* compAttr = nullptr;
        if (!parent->hasChild(name)) {
            compAttr = &parent->addChild<CompoundAttribute>(name);
        } else {
            // Attribute already exists
            compAttr = parent->getChild<CompoundAttribute>(name);
            assert(compAttr); // Fires on wrong type
        }

		//        Object* result;
		XMLElement* elm = (XMLElement*)xml->FirstChild();
		while (elm) {
            readAttribute(elm, core, compAttr);
			elm = (XMLElement*)elm->NextSibling();
		}
		return compAttr;
	}

	Object* XMLDeserializer::readAttribute(tinyxml2::XMLElement* xml, Core& core, Object* parentObject) const
	{
		// Resolve type
		std::string attributeType = dirtyHack(xml->Attribute(X_VALUE_TYPE));
		const RTTI::TypeInfo& type = RTTI::TypeInfo::getByName(attributeType);

		if (type == RTTI::TypeInfo::empty()) {
			nap::Logger::fatal("Failed to retrieve type: %s", attributeType.c_str());
			return nullptr;
		}

		std::string attrName(xml->Attribute(X_NAME));

		// Retrieve or create attribute
		AttributeBase* attribute = nullptr;

		if (parentObject->getTypeInfo().isKindOf<AttributeObject>()) {
            // Regular attribute
			AttributeObject* parentAttrObject = static_cast<AttributeObject*>(parentObject);
			if (!parentAttrObject->hasAttribute(attrName)) {
				attribute = &parentAttrObject->addAttribute(attrName, type);
			} else {
				attribute = parentAttrObject->getAttribute(attrName);
			}
		} else if (parentObject->getTypeInfo().isKindOf<CompoundAttribute>()) {
            // Compound child
			CompoundAttribute* parentCompound = static_cast<CompoundAttribute*>(parentObject);
			if (!parentCompound->hasChild(attrName)) {
				attribute = (AttributeBase*)&parentCompound->addChild(attrName, type);
			} else {
				attribute = parentCompound->getAttribute(attrName);
			}
		}
		assert(attribute);

		// Deserialize link
		const char* link = xml->Attribute(X_LINK);
		if (link) {
			attribute->linkPath(std::string(link));
		}

		// Retrieve and set value
		const TypeConverterBase* converter = core.getModuleManager().getTypeConverter(
			RTTI_OF(std::string), attribute->getValueType());
		if (!converter) {
			nap::Logger::fatal("Cannot convert");
			return attribute;
		}

		Attribute<std::string> strAttr;
		std::string valueStr = xml->Attribute(X_VALUE);
		strAttr.setValue(valueStr);
		if (!converter->convert(&strAttr, attribute)) {
			nap::Logger::fatal("Conversion failed from '%s' to '%s'",
							   converter->inType().getName().c_str(),
							   converter->outType().getName().c_str());
		}

		return attribute;
	}

	Object* XMLDeserializer::readObject(tinyxml2::XMLElement* xml, Core& core, Object* parent) const
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
			core.getRoot().setName(objectName);
			return &core.getRoot();
		}

		if (objectType.isKindOf<Entity>()) {
			assert(parent->getTypeInfo().isKindOf<Entity>());
			Entity* parentEntity = static_cast<Entity*>(parent);
			return &parentEntity->addEntity(objectName);
		}

		// If internal code has already added this child, return it
		if (parent->hasChild(objectName) &&
			parent->getChild(objectName)->getTypeInfo().isKindOf(objectType))
			return parent->getChild(objectName);

		return &parent->addChild(objectName, objectType);
	}


	// Read the root elements and
    Object* XMLDeserializer::readObject(std::istream& istream, Core& core, Object* parentObject) const
	{
		std::istreambuf_iterator<char> eos;
		std::string s(std::istreambuf_iterator<char>(istream), eos);

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

		Object* result = nullptr;
		while (elm) {
            assert(!result); // Assume only one root element
			result = fromXML(elm, core, parentObject);
			elm = (XMLElement*)elm->NextSibling();
		}
		return result;
	}
}