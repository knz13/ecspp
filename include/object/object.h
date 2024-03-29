﻿#pragma once
#include "registry.h"
#include "object_properties.h"
#include "object_property_register.h"
#include "object_base.h"
#include "object_handle.h"
#include <type_traits>

namespace ecspp {




class Component;

class Object : public ObjectBase {
public:
    Object(entt::entity ent) {
        if (!Registry().valid(ent)) {
            ECSPP_DEBUG_ERROR("Passing an invalid entity!!!");
        }
        m_EntityHandle = ent;
    }
    ~Object() {};


    template<typename T>
    bool HasComponent() {

        return ObjectPropertyRegister::HasComponent<T>(m_EntityHandle);
    }

    bool HasComponent(std::string type) {

        auto resolved = entt::resolve(entt::hashed_string(type.c_str()));

        if (resolved) {
            if (auto func = resolved.func(entt::hashed_string("Has Component")); func) {
                return *((bool*)func.invoke({}, this->ID()).data());
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }

    }

private:
    template<auto Func>
    struct VirtualFuncSpecializer {};

    template<typename Obj,typename ReturnType,typename... Args,ReturnType(Obj::*funcPointer)(Args...)>
    struct VirtualFuncSpecializer<funcPointer> {
        auto Call(Object object,Args&&... args) {
            entt::meta_any any = HelperFunctions::CallMetaFunction(object.GetType(), "CallVirtualFunc", object.ID(), std::function([&](Object* obj) -> entt::meta_any {           
                if constexpr (std::is_same<ReturnType, void>::value) {
                    (((Obj*)obj)->*funcPointer)(std::forward<Args>(args)...);
                    return {};
                }
                else {
                    return entt::meta_any((((Obj*)obj)->*funcPointer)(std::forward<Args>(args)...));
                }
            }));


            if constexpr (std::is_same<ReturnType, void>::value) {
                return;
            }
            else {
                if (any.operator bool()) {
                    return *static_cast<ReturnType*>(( * static_cast<entt::meta_any*>(any.data())).data());
                }
                else {
                    throw std::runtime_error("Could not call method with virtual func!");
                }
            }
        };

    };

public:

    template<auto Func,typename... Args>
    constexpr auto CallVirtualFunction(Args&&... args) {
        return VirtualFuncSpecializer<Func>().Call(*this,std::forward<Args...>(args)...);
    };

    void Update(float deltaTime) {
        for (auto& name : GetComponentsNames()) {
            HelperFunctions::CallMetaFunction(name,"Update Component",this->ID(),deltaTime);
        }
    };
    

    template<typename T>
    T& GetComponent() {
        return *ObjectPropertyRegister::GetComponent<T>(m_EntityHandle);
    }


    void SetName(std::string name){
        Properties().SetName(name);
    }

    

    bool CopyComponentByName(std::string stringToHash,Object from){
        auto resolved = entt::resolve(entt::hashed_string(stringToHash.c_str()));
        
        if(resolved){
            entt::meta_any component = resolved.construct(*this);
            if(auto func = resolved.func(entt::hashed_string("Copy Component")) ; func){
                return func.invoke({},from.ID(), this->ID()).operator bool();
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    };

    

    template<typename T,typename ...Args>
    T& AddComponent(Args&&... args){
        return *ObjectPropertyRegister::GetComponent<T, Args...>(m_EntityHandle, std::forward<Args>(args)...);
    }

  
    ComponentHandle AddComponentByName(std::string stringToHash) {
        return { ObjectPropertyRegister::AddComponentByName(this->ID(), stringToHash) };
    };

    ComponentHandle GetComponentByName(std::string stringToHash) {
        return { ObjectPropertyRegister::GetComponentByName(this->ID(), stringToHash) };
	};

	


    template<typename T>
    bool EraseComponent(){
        bool val = ObjectPropertyRegister::EraseComponent<T>(m_EntityHandle);
        Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(m_EntityHandle));
      
        return val;
    };

    bool EraseComponentByName(std::string componentName){
        auto resolved = entt::resolve(entt::hashed_string(componentName.c_str()));
        
        if(resolved){
            if(auto func = resolved.func(entt::hashed_string("Erase Component")) ; func){
                if (func.invoke({}, m_EntityHandle)) {
                    Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(m_EntityHandle));
                    return true;
                }
                else {
                    return false;
                }
            }
            else{
                return false;
            }
        }
        else{
            return false;
        }
    }

    bool Valid(){
        return Registry().valid(m_EntityHandle);
    }

    
    

    const entt::entity& ID(){
        return m_EntityHandle;
    }

    
    template<typename T>
    static bool CopyComponent(Object from,Object to){
        if (ObjectPropertyRegister::CopyComponent<T>(from.ID(), to.ID())) {
            to.Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(to.m_EntityHandle));
            return true;
        }
        return false;
    };

    bool HasSameObjectTypeAs(Object other) {
        return Properties().m_MasterType == other.Properties().m_MasterType;
    }

    template<typename T>
    bool IsOfType() {
        return HelperFunctions::GetClassName<T>() == GetType();
    };

    bool IsOfType(entt::id_type type) {
        return type == Properties().m_MasterType;
    };

    entt::id_type GetTypeID() {
        return Properties().m_MasterType;
    };
    
    bool Empty() {
        bool found = false;
        ForEachComponent([&](auto handle) {
            found = true;

            });
        return !found;
    }

    void ClearParent() {
        if (Properties().m_Parent) {
            Properties().m_Parent.GetAs<Object>().Properties().RemoveChildren(this->Properties());
            Properties().m_Parent = ObjectHandle();
        }
    }

    void SetParent(Object object) {
        Properties().SetParent(object.Properties());

    }

    void RemoveChildren(Object object) {
        Properties().RemoveChildren(object.Properties());
    }

    void AddChildren(Object object) {
        return Properties().AddChildren(object.Properties());
    }

    bool IsInChildren(Object object) const {
        ObjectHandle handle(object.ID());
        if (Properties().m_Children.size() == 0) {
            return false;
        }
        auto it = std::find(Properties().m_Children.begin(), Properties().m_Children.end(), handle);
        if (it != Properties().m_Children.end()) {
            return true;
        }
        for (auto& id : Properties().m_Children) {
            if (id.GetAs<Object>().IsInChildren(object)) {
                return true;
            }
        }
        return false;
    }

    ObjectHandle GetParent() const {
        return Properties().GetParent();
    }

    const std::vector<ObjectHandle>& GetChildren() const {
        return Properties().GetChildren();
    }

    const std::vector< std::string>& GetComponentsNames() const {
        return Properties().GetComponentsNames();
    }

    void ForEachComponent(std::function<void(ComponentHandle&)> func) {
        for (auto& componentName : GetComponentsNames()) {
            ComponentHandle comp = ObjectPropertyRegister::GetComponentByName(this->ID(), componentName);
            if (comp) {
                func(comp);
            }
        }
    }

    void ForEachChild(std::function<void(Object)> func) {
        if (GetChildren().size() == 0) {
            return;
        }
        for (auto& child : GetChildren()) {
            if (child) {
                func(child.GetAs<Object>());
            }
        }
    }
    

    std::string GetType() {
        return ObjectPropertyRegister::GetClassNameByID(Properties().m_MasterType);
    }
   
    std::string GetName() {
        return Properties().m_Name;
    }

    static void ForEach(std::function<void(Object)> func) {
        Registry().each([&](const entt::entity e) {

            func(Object(e));

        });

    }

    
	void ForSelfAndEachChild(std::function<void(Object)> func) {
		func(*this);
		if (GetChildren().size() == 0) {
			return;
		}
		for (auto id : GetChildren()) {
			if(id){
				id.GetAsObject().ForSelfAndEachChild(func);
			}
		}
	};

    

    
    

protected:
   
    



    
    
private:
    ObjectProperties& Properties() const{
        return Registry().get<ObjectProperties>(m_EntityHandle);
    };

    

    entt::entity m_EntityHandle;
    
    friend class ObjectPropertyRegister;

    friend class Registry;

    template<typename,typename>
    friend class ObjectPropertyStorage;

    

    template<typename,typename>
    friend class ComponentSpecifier;

    friend class Component;
};


inline ObjectHandle::ObjectHandle(Object obj) {
    m_Handle = obj.ID();
};

inline Object ObjectHandle::GetAsObject() {
    return Object(m_Handle);
}

inline bool ObjectHandle::IsType(entt::id_type id){
    return GetAsObject().IsOfType(id);
};

template<template<class Something> class T>
inline T<Object> ObjectHandle::GetAs() const{
    static_assert(std::is_base_of<Object, T<Object>>::value, "Class is not derived from object!");
    return { m_Handle };
}

inline void ObjectPropertyRegister::RegisterComponentsNames(entt::entity e) {
    if (!ObjectHandle(e)) {
        return;
    }

    Object(e).Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(e));
}

template<typename T>
inline auto ObjectPropertyRegister::CallVirtualFunc(entt::entity e, std::function<entt::meta_any(Object*)> func) {
    T obj(e);

    return func(&obj);
}

template<typename T>
inline void ObjectPropertyRegister::UpdateComponent(entt::entity e, float deltaTime) {
    if (!ObjectHandle(e)) {
        return;
    }
    dynamic_cast<Component*>(&Object(e).GetComponent<T>())->Update(deltaTime);

}

template<typename T>
inline Component* ObjectPropertyRegister::CastComponentToCommonBase(entt::entity e) {
    if (!ObjectHandle(e)) {
        return nullptr;
    }

    return (Component*) & Object(e).GetComponent<T>();
}

template<typename MainComponentType, typename FinalType>
inline FinalType* ObjectPropertyRegister::CastComponentTo(entt::entity e) {
    if (!ObjectHandle(e)) {
        return nullptr;
    }

    return (FinalType*) & Object(e).GetComponent<MainComponentType>();

}   

template<typename Type>
inline Type* ComponentHandle::GetAs() {

    if (HelperFunctions::HashClassName<Type>() == m_ComponentType) {
        return &Object(m_MasterID).GetComponent<Type>();
    }

    if (auto result = HelperFunctions::CallMetaFunction(m_ComponentType, "Cast To " + HelperFunctions::GetClassName<Type>(), m_MasterID); result) {
        Type* address = *(Type**)result.data();
        return address;
    }
    return nullptr;
};

template<typename T>
inline bool ObjectHandle::IsType() {
    return GetAsObject().IsOfType<T>();
};

};