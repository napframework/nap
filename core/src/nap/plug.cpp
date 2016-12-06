// Local Includes
#include "patch.h"
#include "component.h"
#include "operator.h"
#include "plug.h"

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
    
    
    Plug* Plug::getConnection()
    {
        return mConnection.getTypedTarget();
    }
    
    
    bool Plug::isConnected() const
    {
        return mConnection.isLinked();
    }
    
    
	InputPlugBase::InputPlugBase(Operator* parent, const std::string& name, Type plugType,
								 const RTTI::TypeInfo dataType)
		: Plug(parent, name, plugType, dataType)
	{
	}


	InputPlugBase::~InputPlugBase()
	{
        if (mConnection.getTypedTarget())
            disconnect();
	}


	void InputPlugBase::connect(OutputPlugBase& plug)
	{
		// if this assertion fails you are trying to connect incompatible plugs
		assert(canConnectTo(plug));
        assert(!plug.isConnected());

        plug.mConnection.setTarget(*this);
        mConnection.setTarget(plug);
        
        connected(*this);
	}
    
    
    void InputPlugBase::connect(const std::string &objectPath)
    {
        mConnection.setTarget(objectPath);
    }


	void InputPlugBase::disconnect()
	{
        assert(mConnection.isLinked());
        
        disconnected(*this);
        getConnection()->mConnection.clear();
        mConnection.clear();
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
        if (isConnected())
            static_cast<InputPlugBase*>(getConnection())->disconnect();
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
        if (isConnected())
            static_cast<InputTriggerPlug*>(getConnection())->trigger();
    }
 
}
