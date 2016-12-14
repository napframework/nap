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

	Plug::Plug(Operator* parent, const std::string& name, const RTTI::TypeInfo dataType) : Object()
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
    
    
	InputPlugBase::InputPlugBase(Operator* parent, const std::string& name, const RTTI::TypeInfo dataType)
		: Plug(parent, name, dataType)
	{
	}


	InputPlugBase::~InputPlugBase()
	{
        if (isConnected())
            disconnect();
	}


	void InputPlugBase::connect(OutputPlugBase& plug)
	{
		// if this assertion fails you are trying to connect incompatible plugs
		assert(canConnectTo(plug));
        
        // if this assertion fails the plug is already connected, disconnect first!
        assert(!isConnected());

        mConnection.setTarget(plug);
        
        connected(*this);
	}
    
    
    void InputPlugBase::connect(const std::string &objectPath)
    {
        // if this assertion fails the plug is already connected, disconnect first!
        assert(!isConnected());
        
        mConnection.setTarget(objectPath);
    }


	void InputPlugBase::disconnect()
	{
        assert(isConnected());
        
        disconnected(*this);
        mConnection.clear();
	}

    
	bool InputPlugBase::canConnectTo(OutputPlugBase& plug)
	{
		if (getDataType().getName() != plug.getDataType().getName()) return false;

		return true;
	}


	OutputPlugBase::OutputPlugBase(Operator* parent, const std::string& name, const RTTI::TypeInfo dataType)
		: Plug(parent, name, dataType)
	{
	}


	OutputPlugBase::~OutputPlugBase()
	{
	}

    const std::set<InputPlugBase*> OutputPlugBase::getConnections() const
    {
        std::set<InputPlugBase*> connections;
        Operator* thisOp = getParent();
        Patch* patch = (Patch*) thisOp->getParentObject();

        for (Operator* op : patch->getOperators())
        {
            if (op == thisOp)
                continue;

            for (InputPlugBase* inPlug : op->getInputPlugs()) {
                if (inPlug->isConnected() && inPlug->getConnection() == this)
                    connections.emplace(inPlug);
            }
        }

        return connections;
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
        for (auto& connection : getConnections())
            static_cast<InputTriggerPlug*>(connection)->trigger();
    }


}
