cmake_minimum_required(VERSION 3.4)
project(libcsv VERSION 3.0.3)

set(STATIC OFF)

if(STATIC)
	add_library(csv STATIC libcsv.c)
else()
	add_library(csv SHARED libcsv.c)
	target_compile_definitions(csv PRIVATE CSV_EXPORT)
	set_target_properties(csv
		PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
	if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
		set_target_properties(csv PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
		set_target_properties(csv PROPERTIES INSTALL_RPATH "\$ORIGIN")
	elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
	  	set_target_properties(csv PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
		set_target_properties(csv PROPERTIES INSTALL_RPATH "@loader_path")
	elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
#		set_target_properties(csv PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
#		set_target_properties(csv PROPERTIES INSTALL_RPATH "./")
	endif()

	set_target_properties(csv PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${OUTPUT_DIR}>)
	set_target_properties(csv PROPERTIES LIBRARY_OUTPUT_DIRECTORY $<1:${OUTPUT_DIR}/lib>)

endif()

target_include_directories(csv INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/libcsv)
