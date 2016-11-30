
#include <nap/patch.h>
#include <nap/component.h>
#include <nap/operator.h>
#include <nap/plug.h>

RTTI_DEFINE(nap::Plug)
RTTI_DEFINE(nap::InputPlugBase)
RTTI_DEFINE(nap::OutputPlugBase)

namespace nap
{

	Plug::Plug(Operator* parent, const std::string& name, Type plugType, const RTTI::TypeInfo dataType) : Object(), mPlugType(plugType), mDataType(dataType)
	{
        mName = name;
        parent->addChild(*this);
	}


	Operator* Plug::getParent() const
	{
		return static_cast<Operator*>(getParentObject());
	}


    void Plug::lockComponent()
    {
        getParent()->getPatch()->getComponent()->lockComponent();
    }
    
    
    void Plug::unlockComponent()
    {
        getParent()->getPatch()->getComponent()->unlockComponent();
    }
    
    
	InputPlugBase::InputPlugBase(Operator* parent, const std::string& name, Type plugType,
								 const RTTI::TypeInfo dataType)
		: Plug(parent, name, plugType, dataType)
	{
	}


	InputPlugBase::~InputPlugBase()
	{
		for (auto& connection : connections)
			connection->connections.erase(this);
	}


	void InputPlugBase::connect(OutputPlugBase& plug)
	{
		// if this assertion fails you are trying to connect incompatible plugs
		assert(canConnectTo(plug));

		connections.emplace(&plug);
		plug.connections.emplace(this);
        
        connected({plug, *this});
	}


	void InputPlugBase::disconnect(OutputPlugBase& plug)
	{
		connections.erase(&plug);
		plug.connections.erase(this);
        
        disconnected({plug, *this});
	}

    
    void InputPlugBase::disconnectAll()
    {
        while (!connections.empty())
            disconnect(**connections.begin());
    }


	bool InputPlugBase::canConnectTo(OutputPlugBase& plug)
	{
		if (getPlugType() != plug.getPlugType()) return false;

		if (getDataType().getName() != plug.getDataType().getName()) return false;

		return true;
	}


	OutputPlugBase::OutputPlugBase(Operator* parent, const std::string& name, Type plugType,
								   const RTTI::TypeInfo dataType)
		: Plug(parent, name, plugType, dataType)
	{
	}


	OutputPlugBase::~OutputPlugBase()
	{
		for (auto& connection : connections)
			connection->connections.erase(this);
	}
    
    
    void OutputPlugBase::disconnectAll()
    {
        while (!connections.empty())
            (*connections.begin())->disconnect(*this);        
    }
    
    
    void InputTriggerPlug::trigger()
    {
        if (mTriggerFunction)
        {
            if (mLocking)
            {
                lockComponent();
                mTriggerFunction();
                unlockComponent();
            }
            else
                mTriggerFunction();
        }
    }

    
    void OutputTriggerPlug::trigger()
    {
        // an reinterpret_cast is used here because it's fast and we already checked the input plug's type at connection time in canConnectTo()
        for (auto& connection : connections)
            reinterpret_cast<InputTriggerPlug*>(connection)->trigger();
    }
 
}
