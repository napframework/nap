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


	class Serializer
	{
	public:
		Serializer(std::ostream& stream, Core& core) : stream(stream), mCore(core) {}
		virtual void writeObject(Object& object) = 0;

	protected:
		std::ostream& stream;
        Core& mCore;
	};


	class Deserializer
	{

	public:
		Deserializer(std::istream& stream, Core& core) : stream(stream), mCore(core) {}
		virtual Object* readObject(Object* parent) = 0;

	protected:
		std::istream& stream;
        Core& mCore;
	};
}
