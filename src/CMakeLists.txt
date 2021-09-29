#src/CMakeLists.txt
#Copyright (C) 2021 张子辰 <zichen350@gmail.com>

# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC_YCPIES)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(ycpies ${SRC_YCPIES})
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
	set_target_properties(ycpies PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
	set_target_properties(ycpies PROPERTIES INSTALL_RPATH "\$ORIGIN/lib")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
  	set_target_properties(ycpies PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
	set_target_properties(ycpies PROPERTIES INSTALL_RPATH "@loader_path/lib")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
#Stupid operating system doesn't support rpath.
#	set_target_properties(ycpies PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
#	set_target_properties(ycpies PROPERTIES INSTALL_RPATH "./lib")
endif()
set_property(TARGET ycpies PROPERTY RUNTIME_OUTPUT_DIRECTORY $<1:${OUTPUT_DIR}>)

set_property(SOURCE
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/shared_variable.cpp
	${CMAKE_CURRENT_LIST_DIR}/program_t.cpp
	APPEND PROPERTY COMPILE_DEFINITIONS
	YCPIES_VERSION_MAJOR=${ycpies_VERSION_MAJOR}
	YCPIES_VERSION_MINOR=${ycpies_VERSION_MINOR}
	YCPIES_VERSION_PATCH=${ycpies_VERSION_PATCH}
	YCPIES_VERSION_TWEAK=${ycpies_VERSION_TWEAK}
	YCPIES_VERSION=\"${ycpies_VERSION_MAJOR}.${ycpies_VERSION_MINOR}.${ycpies_VERSION_PATCH}.${ycpies_VERSION_TWEAK}\")

if(DISPLAY_WIDTH_WX)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/text_width.cpp
	PROPERTIES
	COMPILE_DEFINITIONS YCPIES_DISPLAY_WIDTH_WX)
endif()

target_include_directories(ycpies PRIVATE ${EXTERNAL_DIR}/xlnt-1.5.0/third-party/utfcpp)
target_link_libraries(ycpies xlnt csv ${wxWidgets_LIBRARIES})
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_link_libraries(ycpies -defaultlib:shell32)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_link_libraries(ycpies -lboost_program_options -lboost_nowide)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	target_link_libraries(ycpies -lboost_program_options -lboost_nowide)
endif()
if(WIN32)
	if(MSVC)
		target_compile_definitions(ycpies PRIVATE _CRT_SECURE_NO_WARNINGS)
		target_link_libraries(ycpies -subsystem:windows)
	else()
		target_link_libraries(ycpies -mwindows)
	endif()
endif()
