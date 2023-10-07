CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(cachalog_unix LANGUAGES C)

INCLUDE(../base_template.cmake)
INCLUDE(../unix_template.cmake)
INCLUDE(../project_template.cmake)
INCLUDE(../macro_template.cmake)

ADD_SUBDIRECTORY(${CACHALOG_SOURCE_DIRECTORY} ${CMAKE_BINARY_DIR}/src)