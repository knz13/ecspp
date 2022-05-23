

#this CMakeLists was created with EasyCmake - V2 
#the repository can be found at https://github.com/knz13/EasyCmake_Cpp


cmake_minimum_required(VERSION 3.20)



#adding useful functions

function(DIR_EXISTS variable dir_path)

file(GLOB ${variable}_check ${dir_path}/*)

list(LENGTH ${variable}_check ${variable}_len)

if(${${variable}_len} EQUAL 0)

set(${variable} FALSE PARENT_SCOPE)

else()

set(${variable} TRUE PARENT_SCOPE)

endif()

endfunction()

#adding extra cmake libs
include(GNUInstallDirs)
include(ExternalProject)
include(FetchContent)


#project name
project("ecspp_project")

#adding subdirectories...

add_subdirectory(test)


# --------------------- Repository declarations ------------------------


#repository download and settings for alias entt...


	FetchContent_Declare(entt_project

		GIT_REPOSITORY https://github.com/skypjack/entt

		GIT_TAG origin/master

		SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/entt

	)

	list(APPEND ${PROJECT_NAME}_DEPENDENCIES entt_project)


	FetchContent_MakeAvailable(${${PROJECT_NAME}_DEPENDENCIES})

	foreach(X IN LISTS ${PROJECT_NAME}_DEPENDENCIES)

		FetchContent_GetProperties(${X}
			SOURCE_DIR ${X}_SOURCE_DIR
			BINARY_DIR ${X}_BINARY_DIR
		)

	endforeach() 

#creating library
add_library(ecspp STATIC

	${PROJECT_SOURCE_DIR}/include/helpers/helpers.cpp

	${PROJECT_SOURCE_DIR}/include/object/object.cpp

	${PROJECT_SOURCE_DIR}/include/object/object_properties.cpp

	${PROJECT_SOURCE_DIR}/include/object/object_property_register.cpp

	${PROJECT_SOURCE_DIR}/include/object/registry.cpp

)

set_property(TARGET ecspp PROPERTY CXX_STANDARD 17)



#adding includes...


target_include_directories(ecspp PUBLIC ${PROJECT_SOURCE_DIR}/vendor/entt/single_include)
