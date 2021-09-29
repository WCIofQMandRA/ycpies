cmake_minimum_required(VERSION 3.1)
project(xlnt_all)

# This indicates to CMakeLists in subdirectories that they are part of a larger project
set(COMBINED_PROJECT TRUE)

set(STATIC OFF)
# Library type
#option(STATIC "Set to ON to build xlnt as a static library instead of a shared library" ON)

# c++ language standard to use
set(XLNT_VALID_LANGS 11 14 17)
set(XLNT_CXX_LANG "14" CACHE STRING "c++ language features to compile with")
# enumerate allowed values for cmake gui
set_property(CACHE XLNT_CXX_LANG PROPERTY STRINGS ${XLNT_VALID_LANGS})
# validate value is in XLNT_VALID_LANGS
list(FIND XLNT_VALID_LANGS ${XLNT_CXX_LANG} index)
if(index EQUAL -1)
    message(FATAL_ERROR "XLNT_CXX_LANG must be one of ${XLNT_VALID_LANGS}")
endif()

add_subdirectory(source)
