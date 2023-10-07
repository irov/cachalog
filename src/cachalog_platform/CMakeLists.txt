PROJECT(cachalog_platform C)

ADD_FILTER(
src
	cachalog_platform_windows.h
)

ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

if(MSVC)
    add_custom_target(${PROJECT_NAME}.headers SOURCES ${CACHALOG_SOURCE_FILES})
endif()