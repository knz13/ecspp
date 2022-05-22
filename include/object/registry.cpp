#include "registry.h"
#include "object.h"
#include "object_properties.h"
#include <ctime>


namespace ecspp {

entt::registry Registry::m_Registry;
std::mt19937 Registry::m_RandomGenerator(time(nullptr));
entt::registry& Registry::Get() {
    return Registry::m_Registry;
}



std::string Registry::GetComponentDisplayName(std::string componentClassName)
{
    auto resolved = entt::resolve(entt::hashed_string(componentClassName.c_str()));

    if (resolved) {
        if (auto func = resolved.func(entt::hashed_string("Get Display Name")); func) {

            if (auto result = func.invoke({}); result) {
                return result.cast<std::string>();
            }
            else {
                return "";
            }
        }
        else {
            return "";
        }
    }
    else {
        return "";
    }
}





size_t Registry::GenerateRandomNumber() {
    return m_RandomGenerator();
}















ObjectHandle Registry::FindObjectByName(std::string name) {
    
    
    for(auto [handle,comp] : Registry::Get().storage<ObjectProperties>().each()){
        if(comp.GetName() == name){
            return ObjectHandle(handle);
        }
    }
    return ObjectHandle();
}



};