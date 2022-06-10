#pragma once 
#include "../../vendor/entt/single_include/entt/entt.hpp"
#include "../object/registry.h"


namespace ecspp {


class Component {

public:
    bool Valid() {
        return Registry().valid(m_MasterHandle);
    }

    virtual std::string GetTypeName() {
        return HelperFunctions::GetClassName<Component>();
    }

    entt::entity GetMasterHandle() {
        return m_MasterHandle;
    }

    virtual ~Component(){}
    
protected:
    
    Component& operator=(const Component& comp){
        return *this;
    }
    

    /**
     * Use instead of constructor.
     */
    virtual void Init() {};
    /**
     * Use instead of destructor.
     */
    virtual void Destroy() {};

    virtual void Update(float delta) {};
    

    void SetType(entt::id_type type) { m_MyType == type; };
    

    friend class Object;


private:

    void SetMaster(entt::entity entity) {
        m_MasterHandle = entity;
    };
    
    entt::id_type m_MyType = entt::null;
    entt::entity m_MasterHandle = entt::null;
    
    
    
    friend class Window;
    
    template<typename,typename>
    friend class ComponentSpecifier;
    
    
    friend class ObjectPropertyRegister;

};

};
