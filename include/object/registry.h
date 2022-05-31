#pragma once
#include "../helpers/helpers.h"
#include "../../vendor/entt/single_include/entt/entt.hpp"
#include <ctime>
#include <random>
#include "../global.h"

namespace ecspp {

    namespace ComponentHelpers {
        class Null {

        private:
            int dummy = 0;

        };
    }


    class Object;
    class ObjectHandle;
    class Registry {
    public:





        static entt::registry& Get() {
            return m_Registry;
        }
        static size_t GenerateRandomNumber() {
            return m_RandomGenerator();
        };




    private:




        static inline std::mt19937 m_RandomGenerator = std::mt19937(time(nullptr));

        static inline entt::registry m_Registry;

    };

};