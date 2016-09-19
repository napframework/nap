#pragma once
#include <nap/attribute.h>


namespace nap
{


	class AttributeAnimatorBase
	{
	public:
		AttributeAnimatorBase() {}

        virtual void activate() = 0;
		virtual void update(float dt) = 0;
		virtual AttributeBase& getAttribute() = 0;
		virtual bool isFinished() { return true; }
		virtual void finish() = 0;
        virtual void setAttribute(AttributeBase& attrib) = 0;
		virtual RTTI::TypeInfo getAttributeValueType() = 0;

	private:
	};

	template <typename T>
	class AttributeAnimator : public AttributeAnimatorBase
	{
	public:
		AttributeAnimator() : AttributeAnimatorBase() {}

		AttributeBase& getAttribute() override { return *mAttribute; }

		void setAttribute(AttributeBase& attrib) override {
			assert(attrib.getValueType().isKindOf<T>());
			mAttribute = static_cast<Attribute<T>*>(&attrib);
		}

        void setTargetValue(T value) { mTargetValue = value; }
		const T& getTargetValue() { return mTargetValue; }

		// Finish the animation immediately
		virtual void finish() override { mAttribute->setValue(getTargetValue()); }

		T getCurrentValue() { return mAttribute->getValue();  }
		void setCurrentValue(const T& value) { mAttribute->setValue(value); }

		RTTI::TypeInfo getAttributeValueType() override { return mAttribute->getValueType(); }

	private:
        T mTargetValue;
		Attribute<T>* mAttribute;
	};
}


