#pragma once



#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include "../vendor/entt/single_include/entt/entt.hpp"


#ifdef NDEBUG
#define DEBUG_LOG(x)
#define DEBUG_WARN(x)
#define DEBUG_ERROR(x)
#else
#define DEBUG_LOG(x) std::cout << "LOG: " << x << std::endl <<   " At line: "<< __LINE__ << std::endl << "In file: " << __FILE__ << std::endl
#define DEBUG_WARN(x) std::cout << "WARNING: " << x << std::endl <<  "At line: "<< __LINE__ << std::endl << "In file: " << __FILE__ << std::endl
#define DEBUG_ERROR(x) std::cout << "ERROR! -> " << x  << std::endl <<  "At line: "<< __LINE__ << std::endl << "In file: " << __FILE__ << std::endl
#endif