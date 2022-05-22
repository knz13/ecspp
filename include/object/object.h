#pragma once
#include "registry.h"
#include "object_properties.h"
#include "object_property_register.h"
#include "object_base.h"


namespace ecspp {

class Component;

class Object : public ObjectBase {
public:
    Object(entt::entity ent);
    ~Object();


    template<typename T>
    bool HasComponent() {

        return ObjectPropertyRegister::HasComponent<T>(m_EntityHandle);
    }

    bool HasComponent(std::string type);

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
            Properties().SetComponentsNames(ObjectPropertyRegister::GetObjectComponents(m_EntityHandle));
            return true;
        }
        return false;
    };

    bool HasSameObjectTypeAs(Object other);

    template<typename T>
    bool IsOfType() {
        return HelperFunctions::GetClassName<T>() == GetType();
    };
    
    void SetParent(Object object);
    void RemoveChildren(Object object);
    void AddChildren(Object object);
    bool IsInChildren(Object object);
    ObjectHandle GetParent();
    const std::vector<ObjectHandle>& GetChildren() const;
    const std::vector< std::string>& GetComponentsNames() const;
    void ForEachComponent(std::function<void(NamedComponentHandle<Component>&)> func);
    void ForEachChild(std::function<void(Object&)> func);
    

    std::string GetType();
   
    std::string GetName();

protected:
   



    
    
private:
    ObjectProperties& Properties() {
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


};