#pragma once 
#include "../../vendor/entt/single_include/entt/entt.hpp"
#include "../object/registry.h"


namespace ecspp {


class Component {

public:
    bool Valid() {
        return Registry().valid(m_MasterHandle);
    }


    
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
    

    ~Component(){}

    

    friend class Object;


private:

    void SetMaster(entt::entity entity) {
        m_MasterHandle = entity;
    };
    
    
    entt::entity m_MasterHandle = entt::null;
    
    
    
    friend class Window;
    
    template<typename,typename>
    friend class ComponentSpecifier;
    
    
    friend class ObjectPropertyRegister;

};

};
