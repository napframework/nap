Scene Management {#scene}
=======================
*	[Overview](@ref scene_overview)
*	[Setup](@ref scene_setup)
*	[Resources vs Instances](@ref resources_instances)
*	[Components](@ref components)
*	[Creating Components](@ref creating_components)

Overview {#scene_overview}
=======================

Modern applications can grow considerably in size when it comes to the amount of data they have to manage and the complex logic they need to support. NAP uses a powerful entity component [system](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system) to aid the development process. This system allows you to identify and organize all the essential parts of your application. What separates entities and components from regular resources is that they allow you to group content together in a meaningful way and update specific parts of your app at runtime. All the individual parts in the system can reference and talk to each other in a generic fashion. 

Listed below are the most important objects and their role within the system:

- Every application has one or more [scenes](@ref nap::Scene)
- A scene contains a hierarchy of [entities](@ref nap::Entity)
- Entites hold [components](@ref nap::Component)
- Components add functionality to an entity, ie: define behaviour
- Entities and components can link to each other
- Entities are [instantiable](@ref nap::EntityInstance), ie: you can create multiple versions based on the same template
- [Components](@ref nap::ComponentInstance) are updated every frame

Scene Setup {#scene_setup}
=======================

Consider this example:

```
Scene 
	Entity “Bike”
		Component “BikeInput”
		Component “Transform”
		Entity “Frame”
				Component “Transform”
				Component “RenderableMesh”
			Entity “Wheel”
				Component “RenderableMesh”
				Component “Transform”
			Entity “Wheel”
				Component “RenderableMesh”
				Component “Transform”
 ```

In this example we have a single scene. This scene contains 1 'Bike'. The bike is an entity that has 2 input components and 1 'Frame'. The frame has two 'Wheels'. The bike, frame and wheels are entities. The names of these entities are very descriptive, they allow you to identify the different parts of the bike. 

The 'BikeInput' component moves the entire bike in the right direction. Every entity in this scene has a position that is defined by their respective transform component. It's good to know that the position of an entity is always relative to the position of its parent entity. People that work with 3D applications should recognize this pattern. 

In this example the bike has two wheels, one for the front and one for the back. They are both the same 'wheel' but placed differently. Other parts that could vary between wheels are the wheel color or size. What this means is that the same wheel is 'instantiated' twice, but with different properties. You can change these properties for every wheel individually but the template for both wheels is the same 'Wheel' entity. 

You can create this structure yourself or use our editor (Napkin) to do it for you. But the end result in json should look something like this:

```
{
	"Type" : "nap::Scene",
	"mID": "Scene",
	"Entities" : 
	[
		{
			"Entity" : "Bike"
		}
	]
},
{
	"Type" : "nap::Entity",
	"mID": "Wheel"
},
{
	"Type" : "nap::Entity",
	"mID": "Frame",
	"Children" :
	[
		"Wheel",
		"Wheel"
	]
},
{
	"Type" : "nap::Entity",
	"mID": "Bike",
	"Children" :
	[
		"Frame"
	]
}
```

To make things a bit easier to read we have removed the components.

Resources vs Instances {#resources_instances}
=======================

We briefly touched upon the difference between resources and instances in the example above. Resources are 'simple' data containers that can be authored in json. Resources are rather static and often remain in their original state. An instance is never part of a json file. An instance only exists inside the running application. NAP creates these instances for you. An [Entity](@ref nap::Entity) declared in json is such a resource that has a [runtime counterpart](@ref nap::EntityInstance) that is updated by NAP every frame. 

The position of the bike (in the example above) changes as it moves through the world. The initial position of the bike is declared in json but the runtime position changes each frame. When there are multiple bikes in the scene, each bike has its own position. As a programmer you want to change the position of each bike programmatically, ie: set it based on a set of conditions. When you do that you modify the run-time state of a bike, not the resource that was used to create 'an instance of' the bike.

To summarize:
- Resources contain static, shared, read-only data
- Instances contain runtime-varying data and can be updated each frame

Both the Entity and Component have a resource and instance counterpart. NAP omits the resource part of the class name for readability:

- An [Entity](@ref nap::Entity) becomes an [EntityInstance](@ref nap::EntityInstance)
- A [Component](@ref nap::Component) becomes a [ComponentInstance](@ref nap::ComponentInstance)

The resources are defined in json. When a resources is created (instantiated) NAP creates an instance of the resource behind the scenes and adds that to the scene hierarchy. In your application scenes contain entity instances which in turn hold component instances. This structure mirrors the structure in json. Just remember that at run-time, in your application, you work with instances of entities and components.

Components {#components}
=======================

A scene is a container for entities and an entity is a container for components. Scenes and entities do not execute any behavior by themselves. They allow you to group and organize your objects. Components are used to add functionality to an entity, ie: define it's behaviour. It is the component that receives an [init()](@ref nap::ComponentInstance::init) and [update()](@ref nap::ComponentInstance::update) call. Any programmable behavior is therefore executed in the component.

NAP offers a number of components off the shelve such as the [TransformComponent](@ref nap::TransformComponent) and [RenderableMeshComponent](@ref nap::RenderableMeshComponent). These can be used to build hierarchies of visual objects. There are however many more components that ship with NAP. Most modules expose their own components, including: input, osc, midi and audio components.

You probably want to create new components for specific tasks. The video modulation demo uses two custom components. Both components are only available to the videomodulation application. One of these components allows the user to select a shape from a selection of 3 dimensional shapes. The component makes sure that every shape in the list can be rendered to screen and offers an interface to select the one to draw.

Creating Components {#creating_components}
=======================

A component is a resource. Everything you know about [resources](@ref resources) also applies to components. To create a new component derive it from [Component](@ref nap::Component):

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI PerspCameraComponent : public Component
{
	RTTI_ENABLE(Component)
	DECLARE_COMPONENT(PerspCameraComponent, PerspCameraComponentInstance)

	virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override 
	{ 
		components.push_back(RTTI_OF(TransformComponent)); 
	}

public:
	float mFieldOfView = 50.0f;				// Property: Camera Field Of View
};
~~~~~~~~~~~~~~~

and register it together with the properties you need:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS(PerspCameraComponent)
	RTTI_PROPERTY("mFieldOfView", &PerspCameraComponent::mFieldOfView,  nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

Here we create a perspective camera with a field of view property. Some concepts are familiar, others are new:

- We derive from Component instead of RTTIObject
- The DECLARE_COMPONENT macro tells the system which instance of this component to create
- [getDependentComponents()](@ref nap::Component::getDependentComponents) tells the system that this component depends on a transform 

If your component needs another component you can hint at it. In the example above the perspective camera needs a transform to figure out its position in the world. NAP will make sure that a transform is available and initialized before the camera component is initialized. If the entity doesn't have a transform the component can't be created and initialization fails. In json we can extend the scene with an entity that holds both the new component and a transform component 

```
{
    "Type" : "nap::Scene",
    "mID": "Scene", 
    "Entities" : 
    [
        {
            "Entity" : "Bike",
            "Entity" : "CameraEntity"
        }
    ]
},
{
    "Type" : "nap::Entity",
    "mID": "CameraEntity",
    "Components" : 
    [
        {
            "Type" : "nap::PerspCameraComponent",
            "mID" : " PerspCameraComponent",
            "FieldOfView" : 90
        },
        {
            "Type" : "nap::TransformComponent"
        }
    ]
}
```

 When calling [loadFile()](@ref nap::ResourceManager::loadFile()) the perspective camera component is created as part of the 'CameraEntity'. On [init()](@ref nap::ComponentInstance::init()) the camera component is able to access the transform because the dependency to the transform was setup correctly. NAP will now attempt to to create the run time counterpart (instance) of the PerspCameraComponent: a PerspCameraComponentInstance. But we haven't created that object yet:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI PerspCameraComponentInstance : public ComponentInstance
{
	RTTI_ENABLE(ComponentInstance)
public:
	PerspCameraComponentInstance(EntityInstance& entity, Component& resource);

	/**
	 * Copy over the field of view property value
	 * and find the transform component
	 */
	virtual bool init(utility::ErrorState& errorState) override;
};
~~~~~~~~~~~~~~~

The instance part of the camera needs to be registered in the cpp as well. Because every instance is create at run-time and never read from file you don't have to register any properties. The init pattern and the error handling is exactly the same as with regular resources. A small difference is the fact that this object does not contain a default constructor. The constructor of a component instance receives:

- the entity this component belongs to.
- the resource that created this instance. 

To make sure this object can be created we have to tell the system what constructor to use. When registering the instance you explicitly tell the system that there is no default constructor available and add one explicitly:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PerspCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

The instance is created when the file is loaded and the system encounters a 'PerspCameraComponent' resource. The instance is constructed using the registered constructor and is given the resource that created it and the entity it belongs to. When the instance is initialized you know that the transform is available and everything up to that point went well. You can now safely locate the transform component and use it to compute the camera position in world space. 

The [update()](@ref nap::ComponentInstance::update) function can be overridden to add per-frame functionality to your instance. The system calls the update function together with a time stamp for you. The camera instance in the example above doesn't need it but other components do, for example: a component that blends two lines over time.