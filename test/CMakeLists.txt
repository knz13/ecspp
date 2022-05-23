

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
project("ecspp_test_project")

# --------------------- Repository declarations ------------------------


#repository download and settings for alias gtest...


	FetchContent_Declare(gtest_project

		GIT_REPOSITORY https://github.com/google/googletest

		GIT_TAG release-1.11.0

		SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/gtest

	)

	list(APPEND ${PROJECT_NAME}_DEPENDENCIES gtest_project)


	FetchContent_MakeAvailable(${${PROJECT_NAME}_DEPENDENCIES})

	foreach(X IN LISTS ${PROJECT_NAME}_DEPENDENCIES)

		FetchContent_GetProperties(${X}
			SOURCE_DIR ${X}_SOURCE_DIR
			BINARY_DIR ${X}_BINARY_DIR
		)

	endforeach() 

#creating executable
add_executable(ecspp_test


	${PROJECT_SOURCE_DIR}/test.cpp

)

set_property(TARGET ecspp_test PROPERTY CXX_STANDARD 20)


#adding libraries...

target_link_libraries(ecspp_test PUBLIC ecspp)

target_link_libraries(ecspp_test PUBLIC gtest)


#adding includes...


target_include_directories(ecspp_test PUBLIC ${PROJECT_SOURCE_DIR}/../include)

target_include_directories(ecspp_test PUBLIC ${PROJECT_SOURCE_DIR}/vendor/gtest/googletest/include)
