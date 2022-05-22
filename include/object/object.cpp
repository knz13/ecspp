#include "object.h"

Object::~Object() {
  
}

bool Object::HasComponent(std::string type)
{
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


Object::Object(entt::entity ent) {
    if(!Registry::Get().valid(ent)){
        DEBUG_ERROR("Passing an invalid entity!!!");
    }
    m_EntityHandle = ent;
}

bool Object::HasSameObjectTypeAs(Object other)
{
    return Properties().m_MasterType == other.Properties().m_MasterType;
}

std::string Object::GetType()
{
    return ObjectPropertyRegister::GetClassNameByID(Properties().m_MasterType);
}

std::string Object::GetName()
{
    return Properties().m_Name;
}




void Object::AddChildren(Object object) {
    return Properties().AddChildren(object);
}

void Object::ForEachComponent(std::function<void(Component&)> func) {
    for(auto& comp : GetComponentsNames()){
        Component& comp = ObjectPropertyRegister::GetComponentByName<Component>(this->ID(), stringToHash);
        if(comp.Valid()){
            func(comp);
        }
    }
}

const std::vector<ObjectHandle>& Object::GetChildren() const {
    return Properties().GetChildren();
}

const std::vector< std::string>& Object::GetComponentsNames() const {
    return Properties().GetComponentsNames();
}

ObjectHandle Object::GetParent() {
    return Properties().GetParent();
}

bool Object::IsInChildren(Object object) {
    return Properties().IsInChildren();
}

void Object::ForEachChild(std::function<void(Object&)> func) {
    if(GetChildren().size() == 0){
        return;
    }  
    for(auto& child : GetChildren()){
        if(child){
            func(child.GetAsObject());
        }
    }
}