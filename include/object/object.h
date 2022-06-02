﻿#pragma once
#include "registry.h"
#include "object_properties.h"
#include "object_property_register.h"
#include "object_base.h"
#include "object_handle.h"

namespace ecspp {




class Component;

class Object : public ObjectBase {
public:
    Object(entt::entity ent) {
        if (!Registry::Get().valid(ent)) {
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

    template<typename T>
    T& GetComponent() {
        NamedComponentHandle<T> handle(nullptr);
        try {
            handle = ObjectPropertyRegister::GetComponent<T>(m_EntityHandle);
        }
        catch (std::runtime_error& err) {
            throw err;
        }
        if (handle) {
            Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(m_EntityHandle));
        }
        return *handle.GetComponentPointer();
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
        NamedComponentHandle<T> handle(nullptr);
        try {
            handle = ObjectPropertyRegister::GetComponent<T, Args...>(m_EntityHandle, std::forward<Args>(args)...);
        }
        catch (std::runtime_error& err) {
            throw err;
        }
        Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(m_EntityHandle));
        return *handle.GetComponentPointer();
    }

  
    NamedComponentHandle<Component> AddComponentByName(std::string stringToHash) {
		return {ObjectPropertyRegister::AddComponentByName<Component>(this->ID(), stringToHash)};
	};

	NamedComponentHandle<Component> GetComponentByName(std::string stringToHash) {
		return {ObjectPropertyRegister::GetComponentByName<Component>(this->ID(), stringToHash)};
	};



    template<typename T>
    void EraseComponent(){
        ObjectPropertyRegister::EraseComponent<T>(m_EntityHandle);
        Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(m_EntityHandle));
      
        
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
        return Registry::Get().valid(m_EntityHandle);
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

    void ForEachComponent(std::function<void(NamedComponentHandle<Component>&)> func) {
        for (auto& componentName : GetComponentsNames()) {
            NamedComponentHandle<Component> comp = ObjectPropertyRegister::GetComponentByName<Component>(this->ID(), componentName);
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
        Registry::Get().each([&](const entt::entity e) {

            func(Object(e));

        });

    }

    static bool DeleteObject(Object obj) {
		return ObjectPropertyRegister::DeleteObject({obj});
	};

	static ObjectHandle CopyObject(Object objectToCopy){
		return {ObjectPropertyRegister::CopyObject(objectToCopy)};
	}

    static void ClearDeletingQueue() {
        ObjectPropertyRegister::ClearDeletingQueue();
    };

    
    

protected:
   
    



    
    
private:
    ObjectProperties& Properties() const{
        return Registry::Get().get<ObjectProperties>(m_EntityHandle);
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

};