CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(cachalot_win32 LANGUAGES C)

INCLUDE(../base_template.cmake)
INCLUDE(../win32_template.cmake)
INCLUDE(../project_template.cmake)
INCLUDE(../macro_template.cmake)

if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX /wd4200 /wd4324")
  
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif()

ADD_SUBDIRECTORY(${CACHALOT_SOURCE_DIRECTORY} ${CMAKE_BINARY_DIR}/src)