#pragma once
#include "../helpers/helpers.h"
#include "../../vendor/entt/single_include/entt/entt.hpp"
#include <ctime>
#include <random>
#include "../global.h"

namespace ecspp {

    static entt::registry& Registry() {
        static entt::registry m_Registry;

        return m_Registry;
    };

    namespace ComponentHelpers {
        class Null {

        private:
            int dummy = 0;

        };
    }


    
};