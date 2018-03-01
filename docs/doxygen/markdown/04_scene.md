Scene Management {#scene}
=======================
*	[Setup](@ref scene_setup)
*	[Resources vs Instances](@ref resources_instances)
*	[Creating Components](@ref creating_components)

Scene Setup {#scene_setup}
=======================
Modern applications can grow considerably in size when it comes to the amount of data they have to manage and the complex logic the app needs to support. NAP uses a powerful [Entity Component](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system) system to aid the development process and manage your application. What separates entities and components from regular resources are the following properties:

- At the root level, we have one or more [Scenes](@ref nap::Scene). 
- A scene contains a hierarchy of [Entities](@ref nap::Entity)
- Entities hold [Components](@ref nap::Component). 
- Components add functionality to an Entity, ie: define it's behaviour
- Entities and Components can link to each other
- It is possible to create multiple ['instances'](@ref nap::EntityInstance) of a single [Entity](@ref nap::Entity)
- Entity and Component instances are updated each frame

Here is an example of a Scene containing Entities and Components:

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

In this example there is a single Scene with a 'Bike' in it. The bike consists of a 'Frame' and two 'Wheels'. The 'BikeInput' component takes care of moving the bike in the right direction. Notice that the same Entity is placed under the Bike Entity two times. This means it is instantiated two times. But we can change the transformation for each instantation, as discussed here.

In json, such a structure looks like this (the components are omitted for brevity):
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
		"mID": "Bike",
		
		"Components" : 
		[
			...
		],
		"Children" :
		[
			"Wheel",
			"Wheel"
		]
	},
	{
		"Type" : "nap::Entity",
		"mID": "Wheel",
		
		"Components" : 
		[
			...
		]
	},
	{
		"Type" : "nap::Entity",
		"mID": "Frame",
		
		"Components" : 
		[
			...
		]
	}
}
```

Resources vs Instances {#resources_instances}
=======================

Resources are completely static objects. They are read-only data containers. An [Entity](@ref nap::Entity) is a Resource but has a [runtime counterpart](@ref nap::EntityInstance) that is updated by NAP every frame. In the 'Bike' example, the position of the bike changes as it moves through the world. The bike's initial position is declared in json but the runtime position changes each frame. When there are multiple bikes in the scene, each bike has its own position. As a programmer you want to change the position of each bike programmatically, ie: set it based on a set of conditions. When you do that you modify the run-time state of a bike, not the resource that was used to create 'an instance of' the bike.

To summarize:
- Resources contain static, shared, read-only data
- Instances contain runtime-varying data and can be updated each frame

Both the Scene, Entity and Component have a resource and instance counterpart. NAP omits the resource part of the class name for readability:

- nap::Scene becomes a nap::SceneInstance
- nap::Entity becomes a nap::EntityInstance
- nap::Component becomes a nap::ComponentInstance

The resources are defined in json. When a resources is created (instantiated) NAP creates an instance of the resource behind the scenes and adds that to the scene hierarchy. SceneInstances contain EntityInstances which in turn hold ComponentInstances. This structure mirrors the structure in json. Just remember that at run-time, in your application, you work with the instances of Scenes, Entities and Components.

Creating Components {#creating_components}
=======================

A scene is a container for entities and an entity is a container for components. Scenes and entities do not execute any behavior by themselves. They allow you to group and organize your objects. Components are used to add functionality to an Entity, ie: define it's behaviour. It’s the component that receives an [init()](@ref nap::ComponentInstance::init) and [update()](@ref nap::ComponentInstance::update) call. Any programmable behavior is therefore executed in the Component.

NAP offers a number of components off the shelve, like the [TransformComponent](@ref nap::TransformComponent) and the [RenderableMeshComponent](@ref nap::RenderableMeshComponent). These can be used to build hierarchies of visual objects. Many other components exist as well, for instance, input, osc, midi and audio components. However, it is very likely that you want to create your own components. When creating your own component, derive from nap::Component and add the properties that need to be edited in json:

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

Here we create a perspective camera with a field of view property. Some concepts are familiar to creating resources, but some others are new:

- Instead of deriving from nap::RTTIObject, we derive from nap::Component (which is derived from nap::RTTIObject).
- The DECLARE_COMPONENT macro tells NAP that this component has a PerspCameraComponentInstance counterpart, see [Resources vs Instances](@ref resources_instances)
- [getDependentComponents()](@ref nap::Component::getDependentComponents) can be overridden to create a dependency towards another component. 

If your component needs another component, in this case a transform to position the camera in the world, you can hint at it. NAP will make sure that a transform is available and initialized before the camera is initialized. If the entity doesn't have a transform the component can't be created and initialization fails. In json we can extend the “Bike“ scene with an entity that holds both the new component and a nap::TransformComponent

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
            "Type" : "nap::TransformComponent",
            "Properties": {
                "Translate": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                },
                "Rotate": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                },
                "Scale": {
                    "x": 1.0,
                    "y": 1.0,
                    "z": 1.0
                },
                "UniformScale": 1.0                        
            }
        }
    ]
}
```

 When calling [loadFile()](@ref nap::ResourceManager::loadFile()) the PerspCameraComponent is created as part of the CameraEntity. On [init()](@ref nap::ComponentInstance::init()) the camera component is able to access the transform because the dependency to the transform was set-up correctly. NAP will now attempt to to create an instance of the PerspCameraComponent: a PerspCameraComponentInstance. But we haven't created that object yet:

~~~~~~~~~~~~~~~{.cpp}
class NAPAPI PerspCameraComponentInstance : public ComponentInstance
{
	RTTI_ENABLE(ComponentInstance)
public:
	PerspCameraComponentInstance(EntityInstance& entity, Component& resource);

	virtual bool init(utility::ErrorState& errorState) override;
};
~~~~~~~~~~~~~~~

The RTTI needs to be setup in a similar way (although registering properties is not required as this getObject is not read from json). The init pattern and the error handling is exactly the same as with regular resources. A small difference is the fact that this getObject does not contain a default constructor. The constructor receives the entity this component belongs to and the resource counterpart of this instance. To make sure the object can be created (without a default constructor) we have to tell RTTI what constructor to use. You explicitly tell RTTI that there is no default constructor and for each custom constructor, we have to add one seperately:

~~~~~~~~~~~~~~~{.cpp}
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PerspCameraComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS
~~~~~~~~~~~~~~~

When loadFile() is called, the constructor mentioned above will be invoked and our scene is instantiated correctly. The [update()](@ref nap::ComponentInstance::update) function can be overridden to add per-frame functionality. In the case of this camera component, the roles of the various components could look something like this:

Entity
- TransformComponent: initial camera location
- PerspCameraComponent: initial field of view

EntityInstance
- TransformComponent: The transform is modified at runtime by some other component, for example a component that handles mouse input
- PerspCameraComponentInstance: Calculates and stores the camera transform for the renderer