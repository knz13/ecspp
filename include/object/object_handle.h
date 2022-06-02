#pragma once
#include "../global.h"
#include "registry.h"


namespace ecspp {
    class Object;
    class ObjectHandle {
    public:
        ObjectHandle(entt::entity ent) {
            m_Handle = ent;
        }

        ObjectHandle(Object obj);

        ObjectHandle() {};

        Object GetAsObject();

        template<typename T>
        T GetAs() const {
            return T(m_Handle);
        }

        operator bool() const {
            if (isNull) {
                return false;
            }
            return Registry::Get().valid(m_Handle);
        };

        entt::entity ID() const {
            return m_Handle;
        }

        std::string ToString() const {
            return std::to_string((uint32_t)m_Handle);
        }

        bool operator==(const ObjectHandle& other) const {
            return this->m_Handle == other.m_Handle;
        }


    private:
        entt::entity m_Handle = entt::null;
        bool isNull = false;

    };


};