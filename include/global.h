#pragma once



#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
#include <memory>
#include "entt/entt.hpp"


#ifdef NDEBUG
#define DEBUG_LOG(x)
#define DEBUG_WARN(x)
#define DEBUG_ERROR(x)
#else
#define DEBUG_LOG(x) cout << "LOG: " << x << endl <<   " At line: "<< __LINE__ << endl << "In file: " << __FILE__ << endl
#define DEBUG_WARN(x) cout << "WARNING: " << x << endl <<  "At line: "<< __LINE__ << endl << "In file: " << __FILE__ << endl
#define DEBUG_ERROR(x) cout << "ERROR! -> " << x  << endl <<  "At line: "<< __LINE__ << endl << "In file: " << __FILE__ << endl
#endif

using namespace std;