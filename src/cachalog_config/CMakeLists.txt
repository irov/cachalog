PROJECT(cachalog_config C)

ADD_FILTER(
src
    cachalog_config.h
)

ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

if(MSVC)
    add_custom_target(${PROJECT_NAME}.headers SOURCES ${CACHALOG_SOURCE_FILES})
endif()