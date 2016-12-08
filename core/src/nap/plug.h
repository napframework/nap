#pragma once

// Local Includes
#include "attribute.h"
#include "signalslot.h"
#include "link.h"

// External Includes
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <rtti/rtti.h>

namespace nap
{

	// Forward declarations

	class Operator;
	class Component;

	class InputPlugBase;
	class OutputPlugBase;

	template <typename T>
	class InputPullPlug;

	template <typename T>
	class InputPushPlug;

	template <typename T>
	class OutputPullPlug;

	template <typename T>
	class OutputPushPlug;

	template <typename T>
	class InputStreamPlug;

	template <typename T>
	class OutputStreamPlug;


	// A plug is a connector socket to connect operators to another in a patch.
	// An operator can have input as well
	// as output plugs. Output plugs can be connected to input plugs in a patch.
	// This is the base class of all
	// plugs.
	class Plug : public Object
	{
		RTTI_ENABLE_DERIVED_FROM(Object)
        
	public:
		// There can be four different kinds of plugs and connections:
		//
		// PUSH:
		// The input plug responds to a signal emitted by the
		// connected output plug
		//
		// PULL:
		// The output plug is be polled for output by a connected
		// input plug
		//
		// TRIGGER:
		// Basically the same as push but no data is being sent,
		// the connection merely "triggers" the operator to do something.
		//
		// STREAM:
		// On connection the output plug provides a pointer to a
		// data source to the input plug. The input plug calls a connect
		// function provided by its parent operator to
		// implement the nature of the connection. (useful
		// for example for audio or video streaming)
		enum class Type { PUSH, PULL, TRIGGER, STREAM };

	public:
		// Constructor
		Plug(Operator* parent, const std::string& name, Type plugType, const RTTI::TypeInfo dataType);

        // Virtual destructor because of virtual methods!
		virtual ~Plug() = default;

		// The plug has a name to display in guis or to use as identifiers in
		// script languages
		const std::string& getName() const { return mName; }

		// Returns the type of the plug: PUSH, PULL, TRIGGER or STREAM
		const Type getPlugType() const { return mPlugType; }

		// This is the RTTI type info of the data type that is being sent or
		// received by this plug
		const RTTI::TypeInfo getDataType() const { return mDataType; }

		// Returns operator owning this plug
		Operator* getParent() const;
        
        // Returns wether the plug is connected to another plug
        virtual bool isConnected() const = 0;
        
	protected:
        // locks the component the plug's operator resides in
        // TODO deprecate
        void lockComponent();
        // unlocks the component the plug's operator resides in
        // TODO deprecate
        void unlockComponent();
        
    private:
		Type mPlugType = Type::PUSH;
		RTTI::TypeInfo mDataType;
	};



	// This class is used as the base class of all output plugs
	class OutputPlugBase : public Plug
	{
		RTTI_ENABLE_DERIVED_FROM(Plug)
		friend class InputPlugBase;

	public:
		// Constructor takes a parent operator, a name, a type and an rtti data
		// type
		OutputPlugBase(Operator* parent, const std::string& name, Type plugType,
					   const RTTI::TypeInfo dataType);
        
		// Destructor
		virtual ~OutputPlugBase();
        
        // Returns the plugs to which this plug is connected
        const std::set<InputPlugBase*> getConnections() const { return mConnections; }
        
        bool isConnected() const override { return !mConnections.empty(); }
        
    private:
        std::set<InputPlugBase*> mConnections;
        
	};



	// This class is the base class of all input plugs. Because output plugs can
	// only be connected to input plugs and
	// vice versa connectivity is implemented in the input plug class.
	class InputPlugBase : public Plug
	{
		RTTI_ENABLE_DERIVED_FROM(Plug)
		friend class OutputPlugBase;

	public:
		// Constructor takes a parent operator, a name, a type and an rtti data
		// type
		InputPlugBase(Operator* parent, const std::string& name, Type plugType,
					  const RTTI::TypeInfo dataType);

		// Destructor
		virtual ~InputPlugBase();
        
        bool isConnected() const override { return mConnection.isLinked(); }

		// Connect an output plug to this input plug. Checks wether the
		// connection is allowed first.
		virtual void connect(OutputPlugBase& plug);
        
        virtual void connect(const std::string& objectPath);

		// Disconnect this plug from its output
		virtual void disconnect();
        
		// Checks wether an output plug is allowed to be connected to this input plug.
        // This will only be the case if the plug types match and the data types emitted
        // or received by the plugs are the same.
        // This method is virtual because some plug types might need to extend it.
		virtual bool canConnectTo(OutputPlugBase& plug);
        
        OutputPlugBase* getConnection() { return mConnection.getTypedTarget(); }

		// emitted when connected to a plug
		nap::Signal<InputPlugBase&> connected;

		// emitted when disconnected from a plug
		nap::Signal<InputPlugBase&> disconnected;
        
    private:
        // The plug to which this plug is connected
        TypedLink<OutputPlugBase> mConnection = { *this };

	};


    // Same as InputPushPlug but without data, just receving triggers to initiate action
	class InputTriggerPlug : public InputPlugBase
	{
	public:
		using TriggerFunction = std::function<void(void)>;

		InputTriggerPlug(Operator* parent, const std::string& name, TriggerFunction func, bool locking = false)
			: InputPlugBase(parent, name, Plug::Type::TRIGGER, RTTI::TypeInfo::empty()), mTriggerFunction(func), mLocking(locking)
		{
		}

        // Triggers the action associated with this plug
        void trigger();

    private:
        TriggerFunction mTriggerFunction;
        bool mLocking = false;
	};



    class OutputTriggerPlug : public OutputPlugBase
    {
        friend class InputTriggerPlug;

    public:
        // The constructor takes the name of the plug
        OutputTriggerPlug(Operator* parent, const std::string& name)
        : OutputPlugBase(parent, name, Plug::Type::TRIGGER, RTTI::TypeInfo::empty())
        {
        }

        // The operator owning this plug can output through it using the trigger() method.
        void trigger();

    };



	// Output pushed out of an operator's output push plugs can enter another operator through this input push plug
	template <typename T>
	class InputPushPlug : public InputPlugBase
	{
		friend class OutputPushPlug<T>;

	public:
		// The type of the function that is executed when new data enters the
		// plug.
		using PushFunction = std::function<void(const T&)>;

		// The constructor takes the name of the plug and a function that is
		// executed when new data enters trough the
		// plug
		InputPushPlug(Operator* parent, const std::string& name, PushFunction pushFunction, bool locking = false)
			: InputPlugBase(parent, name, Plug::Type::PUSH, RTTI::TypeInfo::get<T>()), pushFunction(pushFunction), mLocking(locking)
		{
		}

		// This constructor takes a memberfunction of the parent that is executed
		// when new data enters the plug
		template <typename U, typename F>
		InputPushPlug(U* parent, F memberFunction, const std::string& name, bool locking = false)
			: InputPlugBase(parent, name, Plug::Type::PUSH, RTTI::TypeInfo::get<T>()),
			  pushFunction(std::bind(memberFunction, parent, std::placeholders::_1)), mLocking(locking)
		{
		}

		// This constructor takes the name of the plug and a reference to an
		// attribute that will be controlled by this
		// plug's input
		InputPushPlug(Operator* parent, const std::string& name, Attribute<T>& attribute, bool locking = false)
			: InputPlugBase(parent, name, Plug::Type::PUSH, RTTI::TypeInfo::get<T>()),
			  pushFunction([&attribute](const T& value) { attribute.setValue(value); }), mLocking(locking)
		{
		}

		// Connect an external attribute to control the input of this plug.
		// Value changes of the attribute will be pushed into this plug.
		void connectAttribute(Attribute<T>& attribute);

		// Disconnect an external attribute controlling the input of this plug
		void disconnectAttribute(Attribute<T>& attribute);

		// Push data into the plug
        void push(const T& value);

	private:
		// User function that is executed when data is pushed into the plug
		PushFunction pushFunction = nullptr;

		// Slot to connect attributes to the plug
		Slot<const T&> attributeSlot = {this, &InputPushPlug<T>::push};

        // Indicates wether the plug locks the containing component on acess
        bool mLocking = false;
	};



	// An output plug to which new output data can be pushed by the owning
	// operator
	template <typename T>
	class OutputPushPlug : public OutputPlugBase
	{
		friend class InputPushPlug<T>;

	public:
		// The constructor takes the name of the plug
		OutputPushPlug(Operator* parent, const std::string& name)
			: OutputPlugBase(parent, name, Plug::Type::PUSH,
							 RTTI::TypeInfo::get<T>())
		{
		}

		// This constructor takes the name of the plug and an attribute of which
		// value changes will be sent through this
		// plug
		OutputPushPlug(Operator* parent, const std::string& name, Attribute<T>& attribute)
			: OutputPlugBase(parent, name, Plug::Type::PUSH, RTTI::TypeInfo::get<T>()),
			  attributeSlot(std::make_unique<Slot<const T&>>([&](const T& value) { push(value); }))
		{
			attribute.connectToValue(*attributeSlot);
		}

		// The operator owning this plug can push output through it using the
		// push() method.
		void push(const T& output);

	private:
		// This slot is used to listen to changes in an attached attribute so
		// they can be emitted through this output
		// plug
		std::unique_ptr<Slot<const T&>> attributeSlot = nullptr;
	};



	// This plug polls a connected output plug for output when the owning
	// operator requests it. Only one output plug can
	// be connected to it at a time, otherwise it doesn't know where to pull
	// output from.
	template <typename T>
	class InputPullPlug : public InputPlugBase
	{
		friend class OutputPullPlug<T>;

	public:
		// The constructor takes the name of the plug
		InputPullPlug(Operator* parent, const std::string& name)
			: InputPlugBase(parent, name, Plug::Type::PULL,
							RTTI::TypeInfo::get<T>())
		{
		}

		// This function can be used by the owning operator to poll for data
		// from the connected plugs. The new polled
		// data will be stored in the input parameter.
		void pull(T& input);
	};



	// This plug can be polled for output by an InputPullPlug to which it is
	// connected.
	// When polled a pull-function is triggered that is provided to the
	// constructor of this plug.
	// The templated type is the data type that the plug emits
	template <typename T>
	class OutputPullPlug : public OutputPlugBase
	{
		friend class InputPullPlug<T>;

	public:
		// The closure type of the user function that is executed when the plug
		// is polled. The function is expected to
		// modify the parameter to contain the polled output
		using PullFunction = std::function<void(T&)>;

		// Constructor takes the name of the plug and the pull function that is executed when the plug is polled.
		OutputPullPlug(Operator* parent, const std::string& name, PullFunction pullFunction, bool locking = false)
			: OutputPlugBase(parent, name, Plug::Type::PULL, RTTI::TypeInfo::get<T>()), pullFunction(pullFunction), mLocking(locking)
		{
		}
        
        // This constructor takes a memberfunction of the parent that is executed
        // when new data enters the plug
        template <typename U, typename F>
        OutputPullPlug(U* parent, F memberFunction, const std::string& name, bool locking = false)
            : OutputPlugBase(parent, name, Plug::Type::PULL, RTTI::TypeInfo::get<T>()), pullFunction(std::bind(memberFunction, parent, std::placeholders::_1)), mLocking(locking)
        {
        }
        
		// This function is called by connected input pull plugs to poll for output. It calls the user defined pull funciton.
        void pull(T& output);

	private:
		PullFunction pullFunction;

        // Indicates wether the plug locks the containing component on access
        bool mLocking = false;
	};


	// This type input plug is made for operators to pass a pointer to a data source from an output plug to an input
	// plug at connection time. The data source should be of type T. To use this type of plug you have to subclass it
	// and override onConnect() and onDisconnect().
	template <typename T>
	class InputStreamPlug : public InputPlugBase
	{
	public:
		// Constructor takes owning operator of the plug, name and a connect-
		// and disconnect lambda to implement
		// (dis)connection of the data source
		InputStreamPlug(Operator* parent, const std::string& name)
			: InputPlugBase(parent, name, Plug::Type::STREAM,
							RTTI::TypeInfo::get<T>())
		{
		}

		// Connect an output plug to this plug
		void connect(OutputPlugBase& plug) override final;

		// Disconnect an output plug from this plug
		void disconnect() override final;

		// Function to be invoked on connection to this plug, to be overriden by
		// descendants
		virtual void onConnect(T* source) = 0;

		// Function to be invoked on disconnection from this plug, to be
		// overriden by descendants
		virtual void onDisconnect(T* source) = 0;
	};


	// This type input plug is made for operators to pass a pointer to a data
	// source from an output plug to an input
	// plug. The output plug manages a pointer to a data source of type T that
	// is passed to an input
	// plug on connection. The input plug should know what to do with the data
	// source.
	template <typename T>
	class OutputStreamPlug : public OutputPlugBase
	{
		friend class InputStreamPlug<T>;

	public:
		// Constructor takes a parent operator, a name and a pointer to a data
		// source that will be passed to an input
		// plug on connection
		OutputStreamPlug(Operator* parent, const std::string& name, T* source)
			: OutputPlugBase(parent, name, Plug::Type::STREAM,
							 RTTI::TypeInfo::get<T>()),
			  source(source)
		{
		}

	private:
		// Pointer to the data source that the plug passes on to connected input
		// plugs
		T* source = nullptr;
	};



	// --- InputPushPlug --- //


	// Connects the push signal of the provided output plug to this input plug's
	// slot
	// connects the push slot to the signal emitted by the attribute when it's
	// value changes.
	template <typename T>
	void InputPushPlug<T>::connectAttribute(Attribute<T>& attribute)
	{
		attribute.connectToValue(attributeSlot);
	}


	// disconnects the push slot from the signal emitted by the attribute when
	// it's value changes.
	template <typename T>
	void InputPushPlug<T>::disconnectAttribute(Attribute<T>& attribute)
	{
		attribute.disconnectFromValue(attributeSlot);
	}


	// triggers the user defined push function
	template <typename T>
	void InputPushPlug<T>::push(const T& value)
	{
		if (pushFunction)
        {
            if (mLocking)
            {
                lockComponent();
                pushFunction(value);
                unlockComponent();
            }
            else
                pushFunction(value);
        }
	}



	// --- OutputPushPlug --- //


	// The push signal is emitted to all the connected input plugs using the
	// provided output as parameter
	template <typename T>
	void OutputPushPlug<T>::push(const T& output)
	{
        for (auto connection : getConnections())
			static_cast<InputPushPlug<T>*>(connection)->push(output);
	}



	// --- InputPullPlug --- //


	// A connected output plug is polled for new output data by calling the pull
	// method.
	template <typename T>
	void InputPullPlug<T>::pull(T& input)
	{
		if (isConnected())
			static_cast<OutputPullPlug<T>*>(getConnection())->pull(input);
	}



	// --- OutputPullPlug --- //


	// Polls the output plug for output by calling the user defined pull
	// function
	template <typename T>
	void OutputPullPlug<T>::pull(T& output)
	{
		if (pullFunction)
        {
            if (mLocking)
            {
                lockComponent();
                pullFunction(output);
                unlockComponent();
            }
            else
                pullFunction(output);
        }
    }


	// --- InputStreamPlug --- //


	// connection is established by saving a link to this plug in the connected
	// output plug and by invoking the connect
	// function with the output plug's data source pointer as argument
	template <typename T>
	void InputStreamPlug<T>::connect(OutputPlugBase& plug)
	{
		InputPlugBase::connect(plug);
		auto outputPlug = dynamic_cast<OutputStreamPlug<T>*>(&plug);
		onConnect(outputPlug->source);
	}


	// connection is established by erasing the link to this plug in the
	// connected output plug and by invoking the
	// disconnect function with the output plug's data source pointer as
	// argument
	template <typename T>
	void InputStreamPlug<T>::disconnect()
	{
        auto outputPlug = dynamic_cast<OutputStreamPlug<T>*>(mConnection);
        onDisconnect(outputPlug->source);
		InputPlugBase::disconnect();
	}



}

RTTI_DECLARE_BASE(nap::Plug)
RTTI_DECLARE_BASE(nap::InputPlugBase)
RTTI_DECLARE_BASE(nap::OutputPlugBase)
