#include "jsonrpcinterpreter.h"

RTTI_DEFINE(nap::JSONRPCInterpreter)

using ObjPtr = int64_t;


namespace nap
{

	ObjPtr toInt(Object* ptr) { return (ObjPtr) reinterpret_cast<uintptr_t>(ptr); }

	Object* fromInt(ObjPtr ptr) { return reinterpret_cast<Object*>((uintptr_t)ptr); }

	template <typename T>
	T* fromInt(ObjPtr ptr)
	{
		return (T*)reinterpret_cast<Object*>((uintptr_t)ptr);
	}


	JSONRPCInterpreter::JSONRPCInterpreter() : ScriptInterpreter()
	{

		mServer.RegisterFormatHandler(mFormatHandler);

		auto& disp = mServer.GetDispatcher();

		disp.AddMethod("getRoot", [&]() { return toInt(getRootObject()); });

        disp.AddMethod("getParent", [&](ObjPtr objPtr) -> ObjPtr {
           Object* obj = fromInt(objPtr);
            if (!obj)
                return 0;
            return toInt(obj->getParentObject());
        });

		disp.AddMethod("getAllChildren", [&](ObjPtr objPtr) -> std::vector<ObjPtr> {
			std::vector<ObjPtr> children;
			Object* parent = fromInt(objPtr);
			for (auto child : parent->getChildren())
				children.push_back(toInt(child));
			return children;
		});

		disp.AddMethod("getChildren", [&](ObjPtr objPtr, const std::string& typeName) {
			std::vector<ObjPtr> children;
			RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
			Object* parent = fromInt(objPtr);
			for (auto child : parent->getChildrenOfType(type))
				children.push_back(toInt(child));
			return children;
		});

        disp.AddMethod("addChild", [&](ObjPtr parentEntity, const std::string& typeName, const std::string& name) {
            Object* parent = fromInt(parentEntity);
            RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
            if (!type.isValid())
                return (ObjPtr) 0;
            return toInt(&parent->addChild(name, type));
        });

        disp.AddMethod("addEntity", [&](ObjPtr parentEntity, const std::string& name) {
            Entity* parent = (Entity*) fromInt(parentEntity);
            if (!parent)
                return (ObjPtr) 0;
            return toInt(&parent->addEntity(name));
        });

        disp.AddMethod("getName", [&](ObjPtr objPtr) { return fromInt(objPtr)->getName(); });

		disp.AddMethod("setName",
					   [&](ObjPtr objPtr, const std::string& name) { return fromInt(objPtr)->setName(name); });

		disp.AddMethod("getTypeName", [&](ObjPtr objPtr) { return fromInt(objPtr)->getTypeInfo().getName(); });

		disp.AddMethod("getAttributeValue", [&](ObjPtr objPtr) {
			AttributeBase* attrib = fromInt<AttributeBase>(objPtr);
			std::string val;
			attrib->toString(val);
			return val;
		});

		disp.AddMethod("setAttributeValue", [&](ObjPtr objPtr, const std::string& value) {
			AttributeBase* attrib = fromInt<AttributeBase>(objPtr);
			attrib->fromString(value);
		});
	}

    std::string JSONRPCInterpreter::evalScript(const std::string &cmd) {
        return mServer.HandleRequest(cmd)->GetData();
    }
}