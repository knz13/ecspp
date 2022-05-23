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