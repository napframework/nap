#pragma once

#include "operator.h"
#include "coreattributes.h"
#include "signalslot.h"


namespace nap {

    class AttributeOutplug : public OutputPlugBase {
        RTTI_ENABLE_DERIVED_FROM(OutputPlugBase)
    public:
        AttributeOutplug(Operator* parent, AttributeBase& attrib) : OutputPlugBase(parent, attrib.getName() + "_outPlug", Plug::Type::PULL, attrib.getValueType()), mAttrib(attrib) {

        }

        AttributeBase& getAttribute() { return mAttrib; }

    private:
        AttributeBase& mAttrib;
    };

    // TODO: OWNERSHIP BROKEN
    template<typename T>
    class MultiInputPlug {
    public:
        MultiInputPlug(nap::Operator &parent, const std::string &name) : mParent(parent), mBaseName(name) {
            // TODO -> FIX OWNERSHIP
            addPlug();
        }

        const std::vector<nap::InputPullPlug<T> *> &plugs() const { return mPlugs; }

    private:
        void addPlug() {
            auto plug = new nap::InputPullPlug<float>(&mParent, mBaseName);
            plug->connected.connect([&](Plug&) { addPlug(); });
            plug->disconnected.connect([&](Plug&) { condensePlugs(); });
            mPlugs.push_back(plug);
        }

        void condensePlugs() {
            assert(false); // Not implemented
        }

        nap::Operator &mParent;
        const std::string mBaseName;
        std::vector<nap::InputPullPlug<T> *> mPlugs;
    };



    class GetAttributesOperator : public Operator
    {
    RTTI_ENABLE_DERIVED_FROM(Operator)
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
                childAdded(*attrib);
        }

        void onChildAdded(Object& obj)
        {
            if (auto attrib = rtti_cast<AttributeBase*>(&obj))
                addChild(std::make_unique<AttributeOutplug>(this, *attrib));
        }
        Slot<Object&> onChildAddedSlot = {this, &GetAttributesOperator::onChildAdded};

        void onChildRemoved(Object& obj)
        {
            if (auto attrib = rtti_cast<AttributeBase*>(&obj))
                removeChild(*findPlug(attrib));
        }
        Slot<Object&> onChildRemovedSlot = {this, &GetAttributesOperator::onChildRemoved};

        AttributeOutplug* findPlug(AttributeBase* attrib)
        {
            for (auto& plug : getChildrenOfType<AttributeOutplug>())
                if (&plug->getAttribute() == attrib)
                    return plug;
            assert(false); // Local bookkeeping failed
            return nullptr;
        }

        void clearOutplugs()
        {
            for (auto plug : getChildrenOfType<AttributeOutplug>())
                removeChild(*plug);
        }

        ObjectLinkAttribute objectLink = {this, "sourceObject", RTTI::TypeInfo::get<AttributeObject>()};

        AttributeObject* mCurrentObject = nullptr;
    };
    
    
    class FloatOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        Attribute<float> mValue = { this, "valueAttr", 0. };
        nap::OutputPullPlug<float> output = { this, &FloatOperator::pullValue, "value" };
        nap::InputPullPlug<float> input = { this, "input" };
        
    private:
        void pullValue(float& outValue)
        {
            if (input.isConnected())
            {
                float result;
                input.pull(result);
                mValue.setValue(result);
            }
            outValue = mValue.getValue();
        }
        
    };


    class AddFloatOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        AddFloatOperator() : nap::Operator() {
            // Add initial term
        }

        InputPullPlug<float> mTermA = {this, "termA"};
        InputPullPlug<float> mTermB = {this, "termb"};

//        MultiInputPlug<float> mTerms = {*this, "term"};
        nap::OutputPullPlug<float> sum = {this, "Sum", [&](float &sum) { calcSum(sum); }};

    private:
        // Calculate sum of all the inlets
        void calcSum(float &outSum) {
            float a;
            float b;
            mTermA.pull(a);
            mTermB.pull(b);

            outSum = a + b;
        }

    };


    class MultFloatOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        MultFloatOperator() : nap::Operator() { }

//        MultiInputPlug<float> mFactors = {*this, "factor"};
        nap::InputPullPlug<float> mFactorA = {this, "factorA"};
        nap::InputPullPlug<float> mFactorB = {this, "factorB"};
        nap::OutputPullPlug<float> product = {this, "product", [&](float &product) { calcProduct(product); }};
    private:
        void calcProduct(float &outProduct) {
            float a;
            mFactorA.pull(a);
            float b;
            mFactorB.pull(b);
            outProduct = a * b;
        }

    };


    class SimpleTriggerOperator : nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        SimpleTriggerOperator() : nap::Operator() { }

        nap::OutputTriggerPlug mOutTrigger = {this, "OutTrigger"};
        nap::InputTriggerPlug mInTrigger = {this, "InTrigger", [&]() {
            mOutTrigger.trigger();
        }};
    };
    
    
    class IntOperator : public nap::Operator {
        RTTI_ENABLE_DERIVED_FROM(nap::Operator)
    public:
        Attribute<int> mValue = { this, "valueAttr", 0. };
        nap::OutputPullPlug<int> output = { this, &IntOperator::pullValue, "value" };
        nap::InputPullPlug<int> input = { this, "input" };
        
    private:
        void pullValue(int& outValue)
        {
            if (input.isConnected())
            {
                int result;
                input.pull(result);
                mValue.setValue(result);
            }
            outValue = mValue.getValue();
        }
        
    };
    

}

RTTI_DECLARE_BASE(nap::AttributeOutplug)
RTTI_DECLARE(nap::AddFloatOperator)
RTTI_DECLARE(nap::SimpleTriggerOperator)
RTTI_DECLARE(nap::MultFloatOperator)
RTTI_DECLARE(nap::FloatOperator)
RTTI_DECLARE(nap::IntOperator)
RTTI_DECLARE_BASE(nap::GetAttributesOperator)
