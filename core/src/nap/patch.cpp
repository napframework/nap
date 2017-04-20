#include "patch.h"
#include <nap/component.h>

RTTI_DEFINE(nap::Patch)

namespace nap
{

    
	Operator* Patch::getOperator(const std::string& name)
	{
        auto operators = getOperators();
		for (auto& op : operators)
			if (op->getName() == name)
                return op;
		return nullptr;
	}


	void Patch::connect(OutputPlugBase& source, InputPlugBase& destination)
	{
		// if this assert fails the input and output plug type or data type are not compatible
		assert(destination.canConnectTo(source));

		// add the connection if it does not exist yet
		if (!connectionExists(source, destination))
			mConnections.emplace_back(std::make_unique<Connection>(source, destination));

		connected(*mConnections.back());
	}


	void Patch::disconnect(OutputPlugBase& source, InputPlugBase& destination)
	{
		// search th connection and erase
		for (auto it = mConnections.begin(); it < mConnections.end(); ++it)
			if (&(*it)->getSource() == &source || &(*it)->getDestination() == &destination) {
				willBeDisconnected(**it);
				mConnections.erase(it);
				return;
			}
	}


	bool Patch::connectionExists(OutputPlugBase& source, InputPlugBase& destination)
	{
		for (auto& connection : mConnections)
			if (&connection->getSource() == &source && &connection->getDestination() == &destination) return true;

		return false;
	}


	Patch::Connection::Connection(OutputPlugBase& source, InputPlugBase& destination)
		: mSource(source), mDestination(destination)
	{
		assert(mDestination.canConnectTo(mSource));

		mDestination.connect(mSource);
	}


	Patch::Connection::~Connection() { mDestination.disconnect(); }

    
    Component* Patch::getComponent()
    {
        Object* object = getParentObject();
        while (object && !object->get_type().is_derived_from<Component>())
            object = object->getParentObject();
        
        if (object)
            return static_cast<Component*>(object);
        else
            return nullptr;
    }
}
