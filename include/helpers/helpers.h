#pragma once
#include "../global.h"
#include <variant>




namespace ecspp {    

class HelperFunctions {
public:
    static inline std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter = " ", const int max_elements = 0) {
        std::vector<std::string> tokens;
        std::string::size_type start_index = 0;
        while (true) {
            std::string::size_type next_index = str.find(delimiter, start_index);
            if (next_index == std::string::npos) {
                if(str.substr(start_index) != ""){
                tokens.push_back(str.substr(start_index));
                }
                break;
            } else {
                if(str.substr(start_index, next_index - start_index) != ""){
                    tokens.push_back(str.substr(start_index, next_index - start_index));
                }
                start_index = next_index + delimiter.length();
            }
            if (max_elements > 0 && tokens.size() == max_elements - 1) {
                tokens.push_back(str.substr(start_index));
                break;
            }
        }

        return tokens;
    }

    static bool StringReplace(std::string& word,const std::string& oldVal,std::string newVal) {

        std::string finalWord;
        int index = 0;
        for (auto& character : word) {
            if (word.substr(index, oldVal.size()) == oldVal) {
                finalWord += newVal;
            }
            else {
                finalWord += character;
            }
            index++;
        }


        return true;
    }
    
    static size_t HashPtr(void* ptr) {
        return std::hash<void*>()(ptr);
    }


    static std::string GenerateStringHash(void* ptr) {
        return std::to_string(HashPtr(ptr));
    }

    static bool EraseWordFromString(std::string& mainWord, const std::string& wordToLookFor) {
        auto iter = mainWord.find(wordToLookFor);

        bool foundAny = false;
        if (iter != std::string::npos) {
            foundAny = true;
        }
        while (iter != std::string::npos) {

            mainWord.erase(iter, wordToLookFor.length());

            iter = mainWord.find(wordToLookFor, iter);
        }
        return foundAny;
    }

    template<typename T>
    static std::string GetClassName() {
        std::string name = std::string(entt::type_id<T>().name());
        HelperFunctions::EraseWordFromString(name, "class ");
        HelperFunctions::EraseWordFromString(name, "struct ");
        if (auto loc = name.find_last_of(':'); loc != std::string::npos) {
            name = std::string(name.begin() + loc + 1, name.end());
        }
        return name;
    }

   

    

    template<typename T>
    static entt::id_type HashClassName() {
        return entt::hashed_string(GetClassName<T>().c_str());
    }

    template<typename... Args>
    static entt::meta_any CallMetaFunction(std::string handle, std::string funcName,Args&&... args) {
        auto resolved = entt::resolve(entt::hashed_string(handle.c_str()));

        if (resolved) {
            if (auto func = resolved.func(entt::hashed_string(funcName.c_str())); func) {
                return func.invoke({}, std::forward<Args>(args)...);
            }
            return {};
        }
        return {};
    }

    static bool IsMetaClass(std::string className) {
        return entt::resolve(entt::hashed_string(className.c_str())).operator bool();
    }
    static bool IsMetaFunction(const std::string& className, std::string funcName) {
        if (!IsMetaClass(className)) {
            return false;
        }
        return entt::resolve(entt::hashed_string(className.c_str())).func(entt::hashed_string(funcName.c_str())).operator bool();
    }
    


    




private:

    

    template<typename T,auto Fun,typename... Args>
    static constexpr auto CallFunctionForObject(entt::entity e, Args&&... args) {

        T obj(e);

        return std::invoke(Fun,&obj, std::forward<decltype(args)>(args)...);

    }

    


    template<typename Base,typename T, auto Func, typename... Args>
    static constexpr auto CallFunctionForObjectWithVirtualBase(entt::entity e, Args&&... args) {

        T obj(e);

        [&obj](auto&&... ar) {
            (((Base*)(&std::forward<T>(obj)))->*Func)(std::forward<decltype(ar)>(ar)...);
        }(std::forward<decltype(args)>(args)...);


        return true;

    };

    template<typename>
    friend class Meta;
    
    
};

template<typename T>
class Meta {
public:

    template<auto Func>
    bool RegisterFunc(std::string funcMetaName) {
        entt::meta<T>().type(hash).template func<Func>(entt::hashed_string(funcMetaName.c_str()));
        return true;
    };

    template<typename Base,auto Func> 
    bool RegisterVirtualMemberFunc(std::string funcMetaName) {
        entt::meta<T>().type(hash).template func<&HelperFunctions::CallFunctionForObjectWithVirtualBase<Base,T, Func>>(entt::hashed_string(funcMetaName.c_str()));
        return true;
    }

    template<auto Func>
    bool RegisterMemberFunc(std::string funcMetaName) {
        entt::meta<T>().type(hash).template func<&HelperFunctions::CallFunctionForObject<T, Func>>(entt::hashed_string(funcMetaName.c_str()));
        return true;
    }
private:
    entt::id_type hash = HelperFunctions::HashClassName<T>();

};

class HelperClasses {
public:
    class Null {
    private:
        int dummy = 0;
    };


    
    template<typename Derived>
    class PointerHolder {
    public:
        operator bool() const {

            if (std::holds_alternative<std::monostate>(m_Pointer)) {
                return false;
            }
            if (IsHolding()) {
                return std::get<std::shared_ptr<Derived>>(m_Pointer).operator bool();
            }

            if (IsObserver()) {
                return std::get<std::weak_ptr<Derived>>(m_Pointer).lock().operator bool();
            }

            return false;
            
        };

        template<typename T>
        
        T& HoldType() {
            auto deleter = [](Derived* ptr) {
                delete ((T*)ptr);
            };

            static_assert(std::is_base_of<Derived, T>::value);
            m_CurrentType = HelperFunctions::GetClassName<T>();
            m_Pointer = std::shared_ptr<Derived>(new T(), deleter);
            return *((T*)std::get< std::shared_ptr<Derived>>(m_Pointer).get());
        }


        void WatchPointer(const PointerHolder<Derived>& pointer) {
            if (!std::holds_alternative<std::shared_ptr<Derived>>(pointer.m_Pointer)) {
                ECSPP_DEBUG_LOG("Trying to watch a pointer that is not valid!");
                return;
            }
            m_CurrentType = pointer.m_CurrentType;
            m_Pointer = std::weak_ptr<Derived>(std::get<std::shared_ptr<Derived>>(pointer.m_Pointer));
        };

        void ClearCurrentType() {
            m_Pointer = std::monostate();
        }

        template<typename T>
        bool IsHoldingType() const {
            return m_CurrentType == HelperFunctions::GetClassName<T>();
        }

        template<typename T>
        T* GetAs() const {
            if (std::holds_alternative< std::shared_ptr<Derived>>(m_Pointer)) {
                return (T*)std::get<std::shared_ptr<Derived>>(m_Pointer).get();
            }
            if (std::holds_alternative<std::weak_ptr<Derived>>(m_Pointer)) {
                return (T*)std::get<std::weak_ptr<Derived>>(m_Pointer).lock().get();
            }

            return nullptr;
        }

        

        Derived* Get() const {
            Derived* ptr = nullptr;

            if (IsHolding()) {
                ptr = (Derived*)std::get<std::shared_ptr<Derived>>(m_Pointer).get();
            }
            if (IsObserver()) {
                ptr = (Derived*)std::get<std::weak_ptr<Derived>>(m_Pointer).lock().get();
            }

            return ptr;
        }

        bool operator==(const PointerHolder<Derived>& other) const{
            if (!other || !this) {
                return false;
            };

            return (this->m_CurrentType == other.m_CurrentType) && (this->Get() == other.Get());
        };

        std::string GetType() {
            return m_CurrentType;
        }

        bool IsHolding() const {
            return std::holds_alternative<std::shared_ptr<Derived>>(m_Pointer);
        }

        bool IsObserver() const{
            return std::holds_alternative<std::weak_ptr<Derived>>(m_Pointer);
        }

    private:
        std::string m_CurrentType = "";
        std::variant<std::monostate, std::shared_ptr<Derived>, std::weak_ptr<Derived>> m_Pointer;

    };





};

};