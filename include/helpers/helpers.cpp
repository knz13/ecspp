#include "helpers.h"

namespace ecspp {

bool HelperFunctions::StringReplace(std::string& word, const std::string& oldVal, std::string newVal)
{
   
    std::string finalWord;
    int index = 0;
    for (auto& character : word) {
        if (word.substr(index,oldVal.size()) == oldVal) {
            finalWord += newVal;
        }
        else {
            finalWord += character;
        }
        index++;
    }


    return true;
}

size_t HelperFunctions::HashPtr(void* ptr)
{
    return std::hash<void*>()(ptr);
}



std::string HelperFunctions::GenerateStringHash(void* ptr)
{
    return std::to_string(HashPtr(ptr));
}

bool HelperFunctions::EraseWordFromString(std::string& mainWord, const std::string& wordToLookFor) {
    auto iter = mainWord.find(wordToLookFor);
    
    bool foundAny = false;
    if(iter != std::string::npos){
        foundAny = true;
    }
    while (iter != std::string::npos) {
        
        mainWord.erase(iter, wordToLookFor.length());
        
        iter = mainWord.find(wordToLookFor, iter);
    }
    return foundAny;
}

bool HelperFunctions::IsMetaClass(std::string className) {
    return entt::resolve(entt::hashed_string(className.c_str())).operator bool();
}

bool HelperFunctions::IsMetaFunction(const std::string& className, std::string funcName)
{
    if (!IsMetaClass(className)) {
        return false;
    }
    return entt::resolve(entt::hashed_string(className.c_str())).func(entt::hashed_string(funcName.c_str())).operator bool();
}





};