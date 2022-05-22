#pragma once
#include "../global.h"






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

    static bool StringReplace(std::string& word,const std::string& oldVal,std::string newVal);
    
    static size_t HashPtr(void* ptr);

    static std::string GenerateStringHash(void* ptr);

    static bool EraseWordFromString(std::string& mainWord, const std::string& wordToLookFor);

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
    static std::string GetClassDisplayName() {
        std::string name = std::string(entt::type_id<T>().name());
        HelperFunctions::EraseWordFromString(name, "Component");
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

    static bool IsMetaClass(std::string className);
    static bool IsMetaFunction(const std::string& className, std::string funcName);
    


    




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
    friend class HelperClasses::Meta;
    
    
};

class HelperClasses {
public:
    class Null {
    private:
        int dummy = 0;
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



    struct EventReceiver;
    struct EventReceiverAttachmentProperties {
        std::function<void(EventReceiver*)> m_CopyFunc;
        std::function<void(void*, size_t)> m_DeleteFunc;
    };


    template<typename T>
    struct EventLauncher;
    template<typename T>
    struct FunctionSink;

    struct EventReceiver {

        ~EventReceiver() {
            if (m_SubscribedEvents.size() == 0) {
                return;
            }
            auto it = m_SubscribedEvents.begin();
            while (it != m_SubscribedEvents.end()) {
                it->second.m_DeleteFunc(it->first, std::hash<void*>()((void*)this));
                it = m_SubscribedEvents.begin();
            }
        }

        EventReceiver& operator=(const EventReceiver& other) {
            for (auto& [ptr, prop] : other.m_SubscribedEvents) {
                prop.m_CopyFunc(this);
            }
            return *this;
        }

    private:
        std::map<void*, EventReceiverAttachmentProperties> m_SubscribedEvents;

        template<typename T>
        friend struct FunctionSink;
        template<typename T>
        friend struct EventLauncher;
    };


    template<typename R, typename... Args>
    struct EventLauncher<R(Args...)> {

        EventLauncher() {
        };

        ~EventLauncher() {
        }


        bool DisconnectReceiver(size_t hash) {
            if (m_Receivers.find(hash) != m_Receivers.end()) {
                m_Receivers.erase(hash);
                return true;
            }
            return false;
        };


        void EmitEvent(Args... args) {

            for (auto& [handle, func] : m_Receivers) {
                if (func) {
                    (*func.get())(args...);
                }
            }

        };

        EventLauncher<R(Args...)>& operator=(const EventLauncher<R(Args...)>& other) {
            return *this;
        }

    private:
        std::unordered_map<size_t, std::shared_ptr<std::function<R(Args...)>>> m_Receivers;
        size_t m_MyHash = std::hash<void*>()((void*)this);

        template<typename T>
        friend struct FunctionSink;

    };







    template<typename R, typename... Args>
    struct FunctionSink<R(Args...)> {
        FunctionSink(EventLauncher<R(Args...)>& sink) : m_Master(&sink) {};

        size_t Connect(std::function<R(Args...)> windowFunc) {
            static int count = 1;
            size_t hash = std::hash<int>()(count);
            count++;

            std::function<R(Args...)>* func = new std::function<R(Args...)>(windowFunc);
            m_Master->m_Receivers[hash] = std::make_shared<std::function<R(Args...)>>(*func);
            return hash;

        }

        void Connect(EventReceiver* key, std::function<R(EventReceiver*, Args...)> windowFunc) {

            size_t hash = std::hash<void*>()((void*)key);
            auto deleter = [=](std::function<R(Args...)>* func) {
                key->m_SubscribedEvents.erase((void*)m_Master);
                delete func;
            };

            auto func = new std::function<R(Args...)>([=](auto... args) {
                windowFunc(key, args...);
            });
            m_Master->m_Receivers[hash] = std::shared_ptr<std::function<R(Args...)>>(func, deleter);

            EventReceiverAttachmentProperties prop;
            prop.m_CopyFunc = [=](EventReceiver* receiver) {
                FunctionSink<R(Args...)>(*m_Master).Connect(receiver, windowFunc);
            };

            prop.m_DeleteFunc = [](void* ptr, size_t hash) {
                ((EventLauncher<R(Args...)>*)(ptr))->DisconnectReceiver(hash);
            };

            key->m_SubscribedEvents[(void*)m_Master] = std::move(prop);

        };

        bool Disconnect(size_t hashKey) {
            return m_Master->DisconnectReceiver(hashKey);
        }

        bool Disconnect(EventReceiver* key) {
            if (key == nullptr) {
                return false;
            }
            size_t hash = std::hash<void*>()((void*)key);
            if (m_Master->m_Receivers.find(hash) != m_Master->m_Receivers.end()) {
                m_Master->DisconnectReceiver(hash);
            }
            else {
                return false;
            }
            return true;
        }

    private:



        EventLauncher<R(Args...)>* m_Master;


    };

}

};