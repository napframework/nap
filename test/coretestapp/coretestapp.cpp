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
#include <nap/coremodule.h>


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
	nap::TransformComponent* new_ts_comp = static_cast<nap::TransformComponent*>(RTTI::TypeInfo::create("nap::TransformComponent"));

	// Register test Service
	Service& service = core.addService<TestService>();
	cout << service.getTypeName().c_str() << "\n";

	// Connect a receiver that can listen to attribute changes
	nap::Receiver receiver;

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

	bool has_transform = entity.hasComponent<nap::TransformComponent>();
	std::cout << "Entity has component: " << has_transform << "\n";

	// Print some data
	std::cout << "Create entity of type: " << entity.getTypeInfo().getName().c_str()
			  << ", with name: " << entity.getName().c_str() << "\n";
	std::cout << "Entity has component of type: " << new_td_comp->getTypeInfo().getName().c_str() << ", with name "
			  << new_td_comp->getName().c_str() << "\n";

	Signal<float*> floatSignal;
	Slot<float*> floatSlot = {[&](float* x) { cout << *x; }};
	floatSignal.connect(floatSlot);

	float a = 1;
	floatSignal(&a);
    
    cout << endl;
    cout << endl;
    
	return 0;
}
