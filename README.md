# ECSpp
Wrapper for the [entt](https://github.com/skypjack/entt) Entity Component System library.

# Motivation

I created this library in order to lower the entry requirements in the ecs usage with c++ since the incredible entt library made by [skypjack](https://github.com/skypjack) was a very hard to get into and understand.


# Build

I tried to make the build steps as simple as possible.

You'll need to have:
1. CMake in your machine which can be downloaded with this [link](https://cmake.org/download/)

That's it!

Just add it to your project like this...
```
  include(FetchContent)
  
  FetchContent_Declare(ECSPP
  GIT_REPOSITORY https://github.com/knz13/ecspp
  GIT_TAG origin/main
  SOURCE_DIR "OptionalDirectoryToDownloadInto"
  )
  
  FetchContent_MakeAvailable(ECSPP)
  
  target_link_libraries(YourLib ecspp)
```

And you're done!

# Usage

The library provides you with some useful classes that I'm going to describe here

## The templated TaggedObject class

Let's introduce an example. Suppose you need to make a GameObject class for your game. You want it to have a highlight color and a bool flag to define if it is active.

Alright, first we'll create the GameObject class. (I'm using the [glm](https://github.com/g-truc/glm) library for the vectors) 

```
#pragma once
#include "ecspp.h" //including the library

using namespace ecspp;

class GameObject : public TaggedObject<GameObject,GameComponent<>,GameObjectProperties> {
public:
  GameObject(entt::entity e);


  bool IsActive();
  void SetHightlightColor(glm::vec3 color);


  //Don't place any variables in this class, we will see why in a little bit
};
```
Ok, so far we can see that the TaggedObject class takes three template parameters and an entt::entity in the constructor. The first template parameter will always be the class you're creating (template magic purposes) and the constructor must always take only and entt::entity! 

But you may be questioning yourself, what are those other two template parameters?

For that I'll need to introduce the second most important class...

## The templated ComponentSpecifier class

This class specifies a new component type that will be used for the TaggedObject child that we are creating

It must **always** be a template class.

Let's take a look at it now...

```
template<typename DerivedComponent>
class GameComponent : public ComponentSpecifier<DerivedComponent,GameObject> {
  
// You can place the variables common for all components here
};
```

Ok, we can see that it takes a DerivedComponent template and the related object (in our case the GameObject).

Any methods you need to have on all components can be added to this class.

Now let's take a look at the last template parameter for the GameObject class...

## The storage part of the TaggedObject

That last class we introduced (GameObjectProperties) does not have to be derived from anything and is just a storage for our object variables, so we'll just add the flag and the color to it.

```
class GameObjectProperties {
public:
  bool m_Active = true; //let's make it active by default
  glm::vec3 m_CurrentColor = glm::vec3(1,1,1); //white color for now

};

```

We can get access to it in any functions of the GameObject class via the Storage() protected method of the TaggedObject class.

Now let's take a look at the implementation file for these classes...

```
#include "game_object.h"

GameObject::GameObject(entt::entity e) : TaggedObject(e){
  //that's all we need in the constructor
};

bool GameObject::IsActive() {
  //We can use the storage method to read from the GameObjectProperties class...
  return Storage().m_Active;
};

void GameObject::SetHightlightColor(glm::vec3 color){
  //And write to it!
  Storage().m_HightlightColor = color;
};


```

That's it! We've created our GameObject! Now lets take a look at how we could use it.

Lets introduce a Transform class which will have the position, rotation and scale of our object.

(In transform.h...)
```
#pragma once
#include "ecspp.h" //including the library

using namespace ecspp;

class Transform : public GameComponent<Transform> {
public:
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

};

```

Ok, but we want to make sure that every GameObject has this class.

For that we have two useful classes that we can subclass.
1. AddToEveryObject<>
2. AddOnlyTo<>

Both of these classes work the same as the ones we've seen already, just pass the classes of the objects you want them to apply to.

```
#pragma once
#include "ecspp.h" //including the library
#include "game_object.h" // adding our GameObject file

using namespace ecspp;

class Transform : public GameComponent<Transform>,public AddOnlyTo<GameObject> {
public:
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

};

```

We will use the AddOnlyTo<> since we may create different objects later.

That's it! our GameObject will always have a Transform component when created.

## Practical usage

In order to enjoy what we've done so far, we can test it on our main() function


```
#include "transform.h"
#include "game_object.h"


int main() {
  GameObject myObject = GameObject::CreateNew("Hi! this is my Name!");
};


```

This is how we create a new object, it'll work like that for any TaggedObject derived classes

We can access our Transform...
```
  .
  .
  .
  
  Transform& myTransform = myObject.GetComponent<Transform>();

```
And change it however you like!

Our object also has some very useful methods such as...

### Getting, adding, erasing and copying by name!
```
  .
  . 
  .
  
  //checking and getting the component
  if(myObject.HasComponent("Transform")){
    Transform& myTransform = myObject.GetComponentByName("Transform").GetAs<Transform>();
    
    //do something with myTransform...
  }
  
  //adding
  
  Camera& camera = myObject.AddComponentByName("Camera").GetAs<Camera>();
  // can also use myObject.AddComponent<Camera>();
  
  //copying
  GameObject other;
  
  other.CopyComponentByName("Camera",myObject);
  //can also use Object::CopyComponent<Camera>(myObject,other);

```

### Parenting system!
```
  GameObject myObject = GameObject::CreateNew("I'm a parent!");
  GameObject myChild = GameObject::CreateNew("I'm a child!");
  
  //adding child
  myObject.AddChildren(myChild);
  
  GameObject otherObject = GameObject::CreateNew("I'm a second child!");
  
  //setting the parent (also adds as child to myObject
  other.SetParent(myObject);
  
  //cycling through children
  for(auto& childHandle : myObject.GetChildren()){
    if(childHandle){ //checking if valid
      std::cout << "my name is " << childHandle.GetAsObject().GetName() << std::endl;
    }
  }
  
  //using a function for each child
  myObject.ForEachChild([](Object& object){ // objects are guaranteed to be valid
    //do something with it...
  });

```

### Cycling through components!
```
  GameObject myObject = GameObject::CreateNew("I'm a parent!");
  
  myObject.AddComponent<Camera>(); // you have to create Camera and RigidBody first
  myObject.AddComponent<RigidBody>();
  
  myObject.ForEachComponent([](std::string name){
    if(myObject.GetComponentByName(name)){ //if the component is found and valid...
      std::cout << "My component name is: " << myObject.GetComponentByName(name).Get().GetName() << " and it is: " << myObject.GetComponentByName(name).Get().IsActive() << std::endl;
    
      //do something else with it
    }
  });
  

```


