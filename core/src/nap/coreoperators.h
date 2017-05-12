#pragma once

#include "coreattributes.h"
#include "logger.h"
#include "operator.h"
#include "signalslot.h"


namespace nap
{



	// TODO: OWNERSHIP BROKEN
	template <typename T>
	class MultiInputPlug
	{
	public:
		MultiInputPlug(nap::Operator& parent, const std::string& name) : mParent(parent), mBaseName(name)
		{
			// TODO -> FIX OWNERSHIP
			addPlug();
		}

		const std::vector<nap::InputPullPlug<T>*>& plugs() const { return mPlugs; }

	private:
		void addPlug()
		{
			auto plug = new nap::InputPullPlug<float>(&mParent, mBaseName);
			plug->connected.connect([&](Plug&) { addPlug(); });
			plug->disconnected.connect([&](Plug&) { condensePlugs(); });
			mPlugs.push_back(plug);
		}

		void condensePlugs()
		{
			assert(false); // Not implemented
		}

		nap::Operator& mParent;
		const std::string mBaseName;
		std::vector<nap::InputPullPlug<T>*> mPlugs;
	};



    /*
	class GetAttributesOperator : public Operator
	{
		RTTI_ENABLE(Operator)
	public:
		GetAttributesOperator() : Operator()
		{
			objectLink.valueChanged.connect([&](AttributeBase& attrib) { objectChanged(); });
		}

	private:
		void objectChanged()
		{
			// Clean up previously set target
			if (mCurrentObject) {
				clearOutplugs();
				mCurrentObject->childAdded.disconnect(onChildAddedSlot);
				mCurrentObject->childRemoved.disconnect(onChildRemovedSlot);
			}

			mCurrentObject = objectLink.getTarget<AttributeObject>();
			if (!mCurrentObject)
				return; // Target was cleared

			// Keep plugs in sync with attributes
			mCurrentObject->childAdded.connect(onChildAddedSlot);
			mCurrentObject->childRemoved.connect(onChildRemovedSlot);

			// Initialize plugs with target's attributes
			for (auto attrib : mCurrentObject->getAttributes())
				onChildAdded(*attrib);
		}

		void onChildAdded(Object& obj)
		{
			auto attrib = rtti_cast<AttributeBase*>(&obj);
			if (!attrib)
				return;

			RTTI::TypeInfo plugType = getOutpullPlugFromValueType(attrib->getValueType());
			OutputPullPlugBase& outPlug = *static_cast<OutputPullPlugBase*>(&addChild(plugType));
			outPlug.setAttribute(attrib);
		}


		Slot<Object&> onChildAddedSlot = {this, &GetAttributesOperator::onChildAdded};

		void onChildRemoved(Object& obj)
		{
			if (auto attrib = rtti_cast<AttributeBase*>(&obj))
				removeChild(*findPlug(attrib));
		}
		Slot<Object&> onChildRemovedSlot = {this, &GetAttributesOperator::onChildRemoved};

		OutputPullPlugBase* findPlug(AttributeBase* attrib)
		{
			if (!attrib)
				return nullptr;

			for (auto& plug : getChildrenOfType<OutputPullPlugBase>())
				if (plug->getAttribute() == attrib)
					return plug;

			assert(false); // Local bookkeeping failed
			return nullptr;
		}

		void clearOutplugs()
		{
			for (auto plug : getChildrenOfType<OutputPullPlugBase>())
				removeChild(*plug);
		}

		ObjectLinkAttribute objectLink = {this, "sourceObject", RTTI::TypeInfo::get<AttributeObject>()};

		AttributeObject* mCurrentObject = nullptr;
	};
    */

	class FloatOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		InputPullPlug<float> input = { this, "input", 0 };
		OutputPullPlug<float> output = { this, &FloatOperator::pullValue, "output" };

	private:
		void pullValue(float& outValue)
		{
            input.pull(outValue);
		}
	};



	class FloatToStringOperator : public Operator
	{
		RTTI_ENABLE(Operator)
	public:
		FloatToStringOperator() : Operator() {}

		InputPullPlug<float> floatInput = {this, "input", 0};
		OutputPullPlug<std::string> stringOutput = {this, &FloatToStringOperator::pullValue, "output"};

	private:
		void pullValue(std::string& outValue)
		{
			float v;
			floatInput.pull(v);
			outValue = std::to_string(v);
		}
	};



	class AddFloatOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		AddFloatOperator() : nap::Operator()
		{
			// Add initial term
		}

		InputPullPlug<float> mTermA = {this, "termA", 0};
		InputPullPlug<float> mTermB = {this, "termb", 0};

		//        MultiInputPlug<float> mTerms = {*this, "term"};
		nap::OutputPullPlug<float> sum = {this, "Sum", [&](float& sum) { calcSum(sum); }};

	private:
		// Calculate sum of all the inlets
		void calcSum(float& outSum)
		{
			float a;
			float b;
			mTermA.pull(a);
			mTermB.pull(b);

			outSum = a + b;
		}
	};



	class MultFloatOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		MultFloatOperator() : nap::Operator() {}

		//        MultiInputPlug<float> mFactors = {*this, "factor"};
		nap::InputPullPlug<float> mFactorA = {this, "factorA", 0};
		nap::InputPullPlug<float> mFactorB = {this, "factorB", 0};
		nap::OutputPullPlug<float> product = {this, "product", [&](float& product) { calcProduct(product); }};

	private:
		void calcProduct(float& outProduct)
		{
			float a;
			mFactorA.pull(a);
			float b;
			mFactorB.pull(b);
			outProduct = a * b;
		}
	};


	class SimpleTriggerOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		nap::OutputTriggerPlug mOutTrigger = {this, "OutTrigger"};
		nap::InputTriggerPlug mInTrigger = {this, "InTrigger", [&]() { mOutTrigger.trigger(); }};
	};

	class OSCOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		nap::OutputTriggerPlug outTrigger = {this, "OSCReceived"};
		nap::OutputPullPlug<float> oscValue = {this, "OSCValue"};
		nap::OutputPullPlug<std::string> oscPath = {this, "OSCPath"};
	};

	class CompareStringOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		nap::OutputPullPlug<bool> condition = {this, "equals"};
		nap::InputPullPlug<std::string> mFactorA = {this, "stringA", ""};
		nap::InputPullPlug<std::string> mFactorB = {this, "stringB", ""};
	};

	class SwitchOperator : public nap::Operator
	{
		RTTI_ENABLE(nap::Operator)
	public:
		nap::InputTriggerPlug mInTrigger = {this, "InTrigger", [&]() {
												bool value;
												condition.pull(value);
												if (value) {
													triggerTrue.trigger();
												} else {
													triggerFalse.trigger();
												}
											}};
		nap::InputPullPlug<bool> condition = {this, "condition", false};
		nap::OutputTriggerPlug triggerTrue = {this, "true"};
		nap::OutputTriggerPlug triggerFalse = {this, "false"};
	};


	class IntOperator : public Operator
	{
		RTTI_ENABLE(Operator)
	public:
		Attribute<int> mValue = {this, "valueAttr", 0};
		OutputPullPlug<int> output = {this, &IntOperator::pullValue, "value"};
		InputPullPlug<int> input = {this, "input", 0};

	private:
		void pullValue(int& outValue)
		{
			if (input.isConnected()) {
				int result;
				input.pull(result);
				mValue.setValue(result);
			}
			outValue = mValue.getValue();
		}
	};

	class LogOperator : public Operator
	{
		RTTI_ENABLE(Operator)
	public:
		InputPullPlug<std::string> input = {this, "message", ""};
        nap::InputTriggerPlug mInTrigger = {this, &LogOperator::trigger, "InTrigger"};

	private:
        void trigger()
        {
            std::string message;
			input.pull(message);
			Logger::info(message);
        }
	};
}
