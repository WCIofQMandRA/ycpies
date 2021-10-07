#CMakeLists.txt
#Copyright (C) 2021 张子辰 <zichen350@gmail.com>

# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.
cmake_minimum_required(VERSION 3.4)
project(ycpies VERSION 0.2.1.0)#Yaskawa Controller Program Import and Export System
set(CMAKE_CXX_STANDARD 17)

option(DISPLAY_WIDTH_WX "使用wxWidgets获得字符串的渲染宽度." ON)

set(EXTERNAL_DIR ${CMAKE_CURRENT_LIST_DIR}/external)
set(OUTPUT_DIR $<IF:$<CONFIG:Debug>,${CMAKE_BINARY_DIR}/output/debug,${CMAKE_BINARY_DIR}/output/ycpies>)

if(MSVC)
  option(STATIC_CRT "Link with the static version of MSVCRT (/MD[d])" OFF)
  add_compile_options(-utf-8)
endif()

add_subdirectory(${EXTERNAL_DIR}/xlnt-1.5.0)
add_subdirectory(${EXTERNAL_DIR}/libcsv-3.0.3)
add_subdirectory(src)
