#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <nap/attributeobject.h>
#include <nap/component.h>
#include <nap/entity.h>
#include <nap/operator.h>
#include <nap/patch.h>
#include <nap/patchcomponent.h>
#include <rtti/rtti.h>

namespace nap
{

    static std::string dirtyHack(const std::string& attrib) { return "nap::Attribute<" + attrib + ">"; }

	class Serializer
	{
	public:
		virtual void writeObject(std::ostream& ostream, Object& object) const = 0;
		std::string toString(Object& object) const;
	};


	class Deserializer
	{

	public:
		virtual Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const = 0;
		Object* fromString(const std::string& str, Core& core, Object* parent = nullptr) const;
	};
}
