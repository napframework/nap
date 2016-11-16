#pragma once

#include <vector>

#include <nap/operator.h>
#include <nap/plug.h>

namespace nap
{

	//! This is a patch of operators that can be connected to one another through their plugs
	class Patch : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		class Connection;

		using ConnectionList = std::vector<std::unique_ptr<Connection>>;

	public:
		Patch() : AttributeObject() {}

		// Virtual destructor because of virtual methods!
		virtual ~Patch() = default;

		// Constructor takes parent object, could be a patch component or a patch operator
		Patch(AttributeObject* parent) : AttributeObject()
		{
			// TODO: SHOULD BE INCOMING REF
			assert(parent != nullptr);
			setParent(*parent);
		}

		// Add an operator to the patch of type T, returns nullptr if T is not an
		// operator
		template <typename T>
		T& addOperator(const std::string& name)
		{
            auto type = RTTI::TypeInfo::get<T>();
            assert(type.template isKindOf<Operator>());
            return addChild<T>(name);
		}

		// Factory, create an operator based on typeinfo or
		Operator& addOperator(RTTI::TypeInfo type)
		{
            assert(type.isKindOf<Operator>());
            return static_cast<Operator&>(addChild(type));
		}

		// Add an operator to the patch using move semantics to transfer parentship
		Operator& addOperator(std::unique_ptr<Operator> op)
        {
            Operator& result = *op;
            addChild(std::move(op));
            return result;
        }

		// Remove operator from the patch
		bool removeOperator(Operator& op) { return removeChild(op); }

		// Connect two plugs
		void connect(OutputPlugBase& source, InputPlugBase& destination);

		// Disconnect two plugs
		void disconnect(OutputPlugBase& source, InputPlugBase& destination);

		// checks if two plugs are connected
		bool connectionExists(OutputPlugBase& source, InputPlugBase& destination);

		// returns the parent object
		// TODO: Patch::getParent() purpose is ambiguous, rethink
		AttributeObject* getParent() const
		{
			return static_cast<AttributeObject*>(getParentObject());
		}

		// Returns all operators in the patch
        std::vector<Operator*> getOperators() { return getChildrenOfType<Operator>(); }

		// Returns an operator by name. Returns nullptr if it doesn't exist
		Operator* getOperator(const std::string& name);

		// Returns all connections read-only
		const ConnectionList& getConnections() const { return mConnections; }

        // Returns the component that this patch resides in (also takes care of the case of a nested patch)
        Component* getComponent();

		// Signal emited when a new operator is added to the patch
		Signal<Operator&> operatorAdded;
		// Signal emitted before an operator is removed
		Signal<Operator&> operatorWillBeRemoved;
		// Signal emitted when a conneciton is established
		Signal<Connection&> connected;
		// Sgnal emitted before a connection is removed/disconnected
		Signal<Connection&> willBeDisconnected;
        

	private:
		// TODO: double bookkeeping! Remove from here and use connection data stored with plugs.
		ConnectionList mConnections;
	};


	// A connection between two operators' plugs in a patch
	class Patch::Connection
	{
	public:
		// Construction of the connection establishes the connection
		Connection(OutputPlugBase& source, InputPlugBase& destination);

		// The connection is disconnected on destruction
		~Connection();

		Operator& getSourceOperator() const { return *mSource.getParent(); }
		Operator& getDestinationOperator() const { return *mDestination.getParent(); }
		OutputPlugBase& getSource() const { return mSource; }
		InputPlugBase& getDestination() const { return mDestination; }

	private:
		OutputPlugBase& mSource;
		InputPlugBase& mDestination;
	};



}
RTTI_DECLARE(nap::Patch)
