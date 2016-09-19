// coretestapp.cpp : Defines the entry point for the console application.
//

#if defined(__WIN32__) || defined(_WIN23)
#include "stdafx.h"
#endif
#include "mathoperators.h"
#include "testcomponent.h"
#include "testservice.h"
#include <fstream>
#include <nap/entity.h>
#include <nap/serializer.h>
#include <nap/binaryserializer.h>


using namespace std;
using namespace nap;

void CallableBySlot(float inValue) { std::cout << "Called with value: " << inValue << "\n"; }


int main()
{
	nap::Core core;

	// Test RTTI
	int i = 0;
	if (RTTI::TypeInfo::get(i).isKindOf<int>()) std::cout << "Created an int!" << endl;

	//    cout << "attribute type info test " << RTTI::TypeInfo::get<Attribute<float>>().getName() << endl;

	std::vector<RTTI::TypeInfo> attributeTypes = RTTI::TypeInfo::getRawTypes(RTTI::TypeInfo::get<nap::AttributeBase>());
	for (auto& type : attributeTypes)
		cout << type.getName() << endl;

	// Create entity
	nap::Entity& entity = core.addEntity("TestEntity");

	// Create bogus transform component
	nap::TransformComponent* new_td_comp = RTTI::TypeInfo::create<nap::TransformComponent>();
	nap::TransformComponent* new_ts_comp =
		static_cast<nap::TransformComponent*>(RTTI::TypeInfo::create("nap::TransformComponent"));

	// Register test Service
	Service& service = core.addService<TestService>();
	cout << service.getTypeName().c_str() << "\n";

	// Add component using perfect forwarding of construction parameters
	// Causing the component's correct constructor to be called
	nap::TransformComponent* transform =
		entity.addComponent<nap::TransformComponent>("This Should Work", 0.0f, 1.0f, 2.0f);

	// Set some other attribute values
	transform->mX.setValue(4.0f);
	transform->mY.setValue(0.0f);
	transform->mZ.setValue(10.0f);

	std::string x, y, z;
	transform->mX.toString(x);
	transform->mY.toString(y);
	transform->mZ.toString(z);

	// Connect a receiver that can listen to attribute changes
	nap::Receiver receiver;

	// Connect mx to slot 1
	//	transform->mX.connectToAttribute(receiver.mSlotOne);
	transform->mX.setValue(2.0f);

	// Connect my to slot 2
	transform->mY.connectToAttribute(receiver.mAttrSlot);
	transform->mY.setValue(22.0f);

	// Create random slot that takes in std::function created using a lambda (lot of typing)
	nap::Slot<float> new_slot_one([&](float inFloat) -> void { std::cout << "called with value" << inFloat << "\n"; });

	// Create slot from existing function using a macro that wraps a lambda
	NSLOT(new_slot_two, float, CallableBySlot)

	// Create random signal and slot -> invoke signal
	nap::Signal<float> new_signal;
	new_signal.connect(new_slot_one);
	new_signal(1.0f);

	// invoke signal two
	new_signal.connect(new_slot_two);
	new_signal(12.0f);

	// connect value changed and trigger
	transform->mZ.setValue(20.0f);

	bool has_transform = entity.hasComponent<nap::TransformComponent>();
	std::cout << "Entity has component: " << has_transform << "\n";

	// Print some data
	std::cout << "Create entity of type: " << entity.getTypeInfo().getName().c_str()
			  << ", with name: " << entity.getName().c_str() << "\n";
	std::cout << "Entity has component of type: " << transform->getTypeInfo().getName().c_str() << ", with name "
			  << transform->getName().c_str() << "\n";
	std::cout << "Entity has attribute of type: " << transform->mX.getValueType().getName().c_str() << "\n";
	std::cout << "With values of: " << x.c_str() << "-" << y.c_str() << "-" << z.c_str() << "\n";

	// construct a patch with a bunch of operators
	nap::PatchComponent& component = *entity.addComponent<PatchComponent>("Patcher");

	// construct a patch
	FloatPushOperator* opAB = component.getPatch().addOperator<FloatPushOperator>("a2");
	FloatPushOperator* opBB = component.getPatch().addOperator<FloatPushOperator>("b2");
	FloatPushOperator* opCB = component.getPatch().addOperator<FloatPushOperator>("c2");
	PlusPushOperator* opPB = component.getPatch().addOperator<PlusPushOperator>("plus2");

	component.getPatch().connect(*opAB->getOutputPlug(0), *opPB->getInputPlug(0));
	component.getPatch().connect(*opBB->getOutputPlug(0), *opPB->getInputPlug(1));
	component.getPatch().connect(*opPB->getOutputPlug(0), *opCB->getInputPlug(0));

	opAB->attribute.setValue(1.0f);
	cout << "value of c: " << opCB->attribute.getValue() << std::endl;
	opBB->attribute.setValue(2.0f);
	cout << "value of c: " << opCB->attribute.getValue() << std::endl;

	// serialize the entity
	ofstream ostream("/Users/stijnvanbeek/Desktop/test.nap");
	BinarySerializer serializer(ostream, core);
	serializer.writeEntity(entity);
	ostream.close();

	// deserialize a copy
	ifstream istream("/Users/stijnvanbeek/Desktop/test.nap");
	BinaryDeserializer deserializer(istream, core);
	deserializer.readEntity(core.getRoot(), "DeserializedEntity");
	istream.close();

	Signal<float*> floatSignal;
	Slot<float*> floatSlot = {[&](float* x) { cout << *x; }};
	floatSignal.connect(floatSlot);

	float a = 1;
	floatSignal(&a);
    
    cout << endl;
    cout << endl;
    
	return 0;
}
